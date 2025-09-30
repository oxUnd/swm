/*
 * System Tray Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "swm.h"
#include "config.h"

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2
#define XEMBED_EMBEDDED_NOTIFY      0
#define XEMBED_MAPPED              (1 << 0)

static Atom net_system_tray_s;
static Atom net_system_tray_opcode;
static Atom xembed;
static Atom xembed_info;

SystemTray* create_tray(void) {
    SystemTray *t;
    XSetWindowAttributes wa;
    char tray_atom_name[32];
    
    /* Check if another tray is running */
    snprintf(tray_atom_name, sizeof(tray_atom_name), "_NET_SYSTEM_TRAY_S%d", screen);
    net_system_tray_s = XInternAtom(dpy, tray_atom_name, False);
    
    if (XGetSelectionOwner(dpy, net_system_tray_s) != None) {
        fprintf(stderr, "swm: another system tray is running\n");
        return NULL;
    }
    
    t = calloc(1, sizeof(SystemTray));
    t->x = screen_width - TRAY_WIDTH;
    t->y = screen_height - TRAY_HEIGHT;
    t->w = TRAY_WIDTH;
    t->h = TRAY_HEIGHT;
    t->clients = NULL;
    
    /* Create tray window */
    wa.override_redirect = True;
    wa.event_mask = ButtonPressMask | ExposureMask;
    wa.background_pixel = get_color("#000000");
    
    t->win = XCreateWindow(dpy, root, t->x, t->y, t->w, t->h, 0,
                          DefaultDepth(dpy, screen),
                          CopyFromParent,
                          DefaultVisual(dpy, screen),
                          CWOverrideRedirect | CWBackPixel | CWEventMask,
                          &wa);
    
    /* Set up atoms */
    net_system_tray_opcode = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
    xembed = XInternAtom(dpy, "_XEMBED", False);
    xembed_info = XInternAtom(dpy, "_XEMBED_INFO", False);
    
    /* Claim selection */
    XSetSelectionOwner(dpy, net_system_tray_s, t->win, CurrentTime);
    
    if (XGetSelectionOwner(dpy, net_system_tray_s) != t->win) {
        fprintf(stderr, "swm: could not acquire system tray selection\n");
        XDestroyWindow(dpy, t->win);
        free(t);
        return NULL;
    }
    
    /* Send manager message */
    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = root;
    ev.message_type = XInternAtom(dpy, "MANAGER", False);
    ev.format = 32;
    ev.data.l[0] = CurrentTime;
    ev.data.l[1] = net_system_tray_s;
    ev.data.l[2] = t->win;
    ev.data.l[3] = 0;
    ev.data.l[4] = 0;
    XSendEvent(dpy, root, False, StructureNotifyMask, (XEvent *)&ev);
    
    /* Set orientation */
    Atom orientation = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
    unsigned long orient_horz = 0;
    XChangeProperty(dpy, t->win,
                   XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False),
                   XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&orient_horz, 1);
    
    XMapRaised(dpy, t->win);
    XSync(dpy, False);
    
    return t;
}

void destroy_tray(SystemTray *t) {
    if (!t) {
        return;
    }
    
    /* Remove all tray clients */
    while (t->clients) {
        TrayClient *next = t->clients->next;
        XUnmapWindow(dpy, t->clients->win);
        XReparentWindow(dpy, t->clients->win, root, 0, 0);
        free(t->clients);
        t->clients = next;
    }
    
    /* Release selection */
    XSetSelectionOwner(dpy, net_system_tray_s, None, CurrentTime);
    
    XDestroyWindow(dpy, t->win);
    free(t);
}

void add_tray_client(Window w) {
    TrayClient *tc;
    XWindowAttributes wa;
    
    if (!tray) {
        return;
    }
    
    /* Check if already in tray */
    for (tc = tray->clients; tc && tc->win != w; tc = tc->next);
    if (tc) {
        return;
    }
    
    if (!XGetWindowAttributes(dpy, w, &wa)) {
        return;
    }
    
    /* Create tray client */
    tc = calloc(1, sizeof(TrayClient));
    tc->win = w;
    tc->w = TRAY_HEIGHT;
    tc->h = TRAY_HEIGHT;
    tc->next = tray->clients;
    tray->clients = tc;
    
    /* Embed the icon */
    XSelectInput(dpy, w, StructureNotifyMask | PropertyChangeMask);
    XReparentWindow(dpy, w, tray->win, 0, 0);
    
    /* Send XEMBED message */
    XClientMessageEvent ev;
    ev.type = ClientMessage;
    ev.window = w;
    ev.message_type = xembed;
    ev.format = 32;
    ev.data.l[0] = CurrentTime;
    ev.data.l[1] = XEMBED_EMBEDDED_NOTIFY;
    ev.data.l[2] = 0;
    ev.data.l[3] = tray->win;
    ev.data.l[4] = 0;
    XSendEvent(dpy, w, False, NoEventMask, (XEvent *)&ev);
    
    XMapRaised(dpy, w);
    update_tray_layout();
}

void remove_tray_client(Window w) {
    TrayClient *tc, **ptc;
    
    if (!tray) {
        return;
    }
    
    for (ptc = &tray->clients; *ptc && (*ptc)->win != w; ptc = &(*ptc)->next);
    
    if (!*ptc) {
        return;
    }
    
    tc = *ptc;
    *ptc = tc->next;
    
    XUnmapWindow(dpy, tc->win);
    XReparentWindow(dpy, tc->win, root, 0, 0);
    free(tc);
    
    update_tray_layout();
}

void update_tray_layout(void) {
    TrayClient *tc;
    int x = 0;
    
    if (!tray) {
        return;
    }
    
    /* Layout tray icons horizontally */
    for (tc = tray->clients; tc; tc = tc->next) {
        tc->x = x;
        tc->y = 0;
        XMoveResizeWindow(dpy, tc->win, tc->x, tc->y, tc->w, tc->h);
        x += tc->w + 2;
    }
    
    XSync(dpy, False);
}
