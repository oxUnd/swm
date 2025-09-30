/*
 * Simple Window Manager (SWM)
 * Main implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "swm.h"
#include "config.h"

/* Global variables */
Display *dpy = NULL;
Window root;
Monitor *mon = NULL;
Config config;
SystemTray *tray = NULL;
int screen;
int screen_width, screen_height;
bool running = true;
static Atom wm_protocols;
static Atom wm_delete_window;
static Atom net_system_tray_opcode;
static Atom net_system_tray_orientation;
static Atom net_system_tray_selection;
static Atom xembed_info;

/* Event handler function pointer array */
static void (*event_handlers[LASTEvent])(XEvent *) = {
    [ConfigureRequest] = on_configure_request,
    [MapRequest] = on_map_request,
    [UnmapNotify] = on_unmap_notify,
    [DestroyNotify] = on_destroy_notify,
    [EnterNotify] = on_enter_notify,
    [KeyPress] = on_key_press,
    [ButtonPress] = on_button_press,
    [ClientMessage] = on_client_message,
};

void die(const char *errstr) {
    fprintf(stderr, "swm: %s\n", errstr);
    exit(EXIT_FAILURE);
}

unsigned long get_color(const char *color) {
    Colormap cmap = DefaultColormap(dpy, screen);
    XColor xcolor;
    
    if (!XAllocNamedColor(dpy, cmap, color, &xcolor, &xcolor)) {
        die("Cannot allocate color");
    }
    return xcolor.pixel;
}

void setup(void) {
    XSetWindowAttributes wa;
    
    /* Open display */
    if (!(dpy = XOpenDisplay(NULL))) {
        die("Cannot open display");
    }
    
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    screen_width = DisplayWidth(dpy, screen);
    screen_height = DisplayHeight(dpy, screen);
    
    /* Initialize atoms */
    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    net_system_tray_opcode = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
    net_system_tray_orientation = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
    char tray_atom_name[32];
    snprintf(tray_atom_name, sizeof(tray_atom_name), "_NET_SYSTEM_TRAY_S%d", screen);
    net_system_tray_selection = XInternAtom(dpy, tray_atom_name, False);
    xembed_info = XInternAtom(dpy, "_XEMBED_INFO", False);
    
    /* Check if another WM is running */
    XSetErrorHandler(NULL);
    XSelectInput(dpy, root, SubstructureRedirectMask);
    XSync(dpy, False);
    
    /* Set up event mask for root window */
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                    ButtonPressMask | PointerMotionMask | EnterWindowMask |
                    LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask, &wa);
    XSelectInput(dpy, root, wa.event_mask);
    
    /* Initialize configuration */
    config.border_width = BORDER_WIDTH;
    config.border_normal = get_color(BORDER_NORMAL);
    config.border_focus = get_color(BORDER_FOCUS);
    config.master_factor = MASTER_FACTOR;
    config.num_master = NUM_MASTER;
    config.keys = keys;
    config.num_keys = sizeof(keys) / sizeof(keys[0]);
    config.layouts = layouts;
    config.num_layouts = sizeof(layouts) / sizeof(layouts[0]);
    
    /* Initialize monitor */
    mon = calloc(1, sizeof(Monitor));
    mon->x = 0;
    mon->y = 0;
    mon->w = screen_width;
    mon->h = screen_height - TRAY_HEIGHT;
    mon->master_factor = config.master_factor;
    mon->num_master = config.num_master;
    mon->layout = &config.layouts[0];
    
    /* Grab keys */
    grab_keys();
    
    /* Initialize system tray */
    tray = create_tray();
    
    /* Scan for existing windows */
    scan();
    
    printf("SWM initialized successfully\n");
}

void cleanup(void) {
    /* Clean up clients */
    while (mon->clients) {
        remove_client(mon->clients);
    }
    
    /* Clean up tray */
    if (tray) {
        destroy_tray(tray);
    }
    
    /* Clean up monitor */
    free(mon);
    
    /* Close display */
    XCloseDisplay(dpy);
}

void scan(void) {
    unsigned int num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;
    
    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (unsigned int i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa) ||
                wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1)) {
                continue;
            }
            if (wa.map_state == IsViewable || wa.map_state == IsUnmapped) {
                Client *c = create_client(wins[i]);
                if (c) {
                    attach_client(c);
                }
            }
        }
        if (wins) {
            XFree(wins);
        }
    }
    apply_layout();
}

void run(void) {
    XEvent ev;
    
    /* Main event loop */
    while (running && !XNextEvent(dpy, &ev)) {
        if (event_handlers[ev.type]) {
            event_handlers[ev.type](&ev);
        }
    }
}

/* Event handlers */
void on_configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;
    
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    XSync(dpy, False);
}

void on_map_request(XEvent *e) {
    XMapRequestEvent *ev = &e->xmaprequest;
    XWindowAttributes wa;
    
    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) {
        return;
    }
    
    /* Check if it's a tray icon */
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    
    if (XGetWindowProperty(dpy, ev->window, xembed_info, 0, 2, False,
                          xembed_info, &actual_type, &actual_format,
                          &nitems, &bytes_after, &data) == Success && data) {
        XFree(data);
        add_tray_client(ev->window);
        return;
    }
    
    /* Create and manage the client */
    Client *c = create_client(ev->window);
    if (c) {
        attach_client(c);
        XMapWindow(dpy, ev->window);
        focus_client(c);
        apply_layout();
    }
}

void on_unmap_notify(XEvent *e) {
    XUnmapEvent *ev = &e->xunmap;
    Client *c;
    
    /* Find client */
    for (c = mon->clients; c && c->win != ev->window; c = c->next);
    
    if (c) {
        remove_client(c);
        apply_layout();
    } else {
        /* Check if it's a tray client */
        remove_tray_client(ev->window);
    }
}

void on_destroy_notify(XEvent *e) {
    XDestroyWindowEvent *ev = &e->xdestroywindow;
    Client *c;
    
    /* Find client */
    for (c = mon->clients; c && c->win != ev->window; c = c->next);
    
    if (c) {
        remove_client(c);
        apply_layout();
    } else {
        /* Check if it's a tray client */
        remove_tray_client(ev->window);
    }
}

void on_enter_notify(XEvent *e) {
    XCrossingEvent *ev = &e->xcrossing;
    Client *c;
    
    if (ev->mode != NotifyNormal || ev->detail == NotifyInferior) {
        return;
    }
    
    /* Find client and focus it */
    for (c = mon->clients; c && c->win != ev->window; c = c->next);
    if (c) {
        focus_client(c);
    }
}

void on_key_press(XEvent *e) {
    XKeyEvent *ev = &e->xkey;
    KeySym keysym = XLookupKeysym(ev, 0);
    
    for (int i = 0; i < config.num_keys; i++) {
        if (keysym == config.keys[i].keysym &&
            ev->state == config.keys[i].mod &&
            config.keys[i].func) {
            config.keys[i].func(config.keys[i].arg);
            return;
        }
    }
}

void on_button_press(XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    Client *c;
    
    /* Find client */
    for (c = mon->clients; c && c->win != ev->window; c = c->next);
    
    if (c) {
        focus_client(c);
    }
}

void on_client_message(XEvent *e) {
    XClientMessageEvent *ev = &e->xclient;
    
    /* Handle system tray messages */
    if (ev->message_type == net_system_tray_opcode) {
        if (ev->data.l[1] == 0) { /* SYSTEM_TRAY_REQUEST_DOCK */
            add_tray_client((Window)ev->data.l[2]);
        }
    }
}

int main(void) {
    setup();
    run();
    cleanup();
    return EXIT_SUCCESS;
}
