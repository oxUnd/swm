/*
 * Client (Window) Management
 */

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "swm.h"

Client* create_client(Window w) {
    Client *c;
    XWindowAttributes wa;
    
    if (!XGetWindowAttributes(dpy, w, &wa)) {
        return NULL;
    }
    
    c = calloc(1, sizeof(Client));
    c->win = w;
    c->x = wa.x;
    c->y = wa.y;
    c->w = wa.width;
    c->h = wa.height;
    c->old_x = c->x;
    c->old_y = c->y;
    c->old_w = c->w;
    c->old_h = c->h;
    c->is_floating = false;
    c->is_fullscreen = false;
    
    /* Set border */
    XSetWindowBorderWidth(dpy, w, config.border_width);
    XSetWindowBorder(dpy, w, config.border_normal);
    
    /* Set event mask */
    XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
    
    return c;
}

void attach_client(Client *c) {
    c->next = mon->clients;
    if (mon->clients) {
        mon->clients->prev = c;
    }
    mon->clients = c;
    c->prev = NULL;
}

void detach_client(Client *c) {
    if (c->prev) {
        c->prev->next = c->next;
    } else {
        mon->clients = c->next;
    }
    if (c->next) {
        c->next->prev = c->prev;
    }
}

void focus_client(Client *c) {
    if (!c) {
        return;
    }
    
    /* Unfocus previously selected client */
    if (mon->selected && mon->selected != c) {
        XSetWindowBorder(dpy, mon->selected->win, config.border_normal);
    }
    
    /* Focus new client */
    mon->selected = c;
    XSetWindowBorder(dpy, c->win, config.border_focus);
    XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
    XRaiseWindow(dpy, c->win);
}

void remove_client(Client *c) {
    if (!c) {
        return;
    }
    
    detach_client(c);
    
    if (mon->selected == c) {
        mon->selected = mon->clients;
        if (mon->selected) {
            focus_client(mon->selected);
        }
    }
    
    free(c);
}

void configure_client(Client *c) {
    XConfigureEvent ce;
    
    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = config.border_width;
    ce.above = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void resize_client(Client *c, int x, int y, int w, int h) {
    if (!c) {
        return;
    }
    
    c->x = x;
    c->y = y;
    c->w = w;
    c->h = h;
    
    XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
    configure_client(c);
}

void show_client(Client *c) {
    if (!c) {
        return;
    }
    XMapWindow(dpy, c->win);
}

void hide_client(Client *c) {
    if (!c) {
        return;
    }
    XUnmapWindow(dpy, c->win);
}
