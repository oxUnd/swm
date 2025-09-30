/*
 * Keyboard Binding and Actions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "swm.h"

void grab_keys(void) {
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    
    for (int i = 0; i < config.num_keys; i++) {
        KeyCode code = XKeysymToKeycode(dpy, config.keys[i].keysym);
        if (code) {
            XGrabKey(dpy, code, config.keys[i].mod, root,
                    True, GrabModeAsync, GrabModeAsync);
            /* Also grab with numlock */
            XGrabKey(dpy, code, config.keys[i].mod | Mod2Mask, root,
                    True, GrabModeAsync, GrabModeAsync);
            /* Also grab with capslock */
            XGrabKey(dpy, code, config.keys[i].mod | LockMask, root,
                    True, GrabModeAsync, GrabModeAsync);
            /* Also grab with both */
            XGrabKey(dpy, code, config.keys[i].mod | Mod2Mask | LockMask, root,
                    True, GrabModeAsync, GrabModeAsync);
        }
    }
}

void spawn(const char *arg) {
    if (!arg) {
        return;
    }
    
    if (fork() == 0) {
        if (dpy) {
            close(ConnectionNumber(dpy));
        }
        setsid();
        execl("/bin/sh", "sh", "-c", arg, (char *)NULL);
        fprintf(stderr, "swm: execl '%s' failed\n", arg);
        exit(EXIT_SUCCESS);
    }
}

void quit_wm(const char *arg) {
    (void)arg;
    running = false;
}

void kill_client(const char *arg) {
    (void)arg;
    
    if (!mon->selected) {
        return;
    }
    
    XEvent ev;
    Atom wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
    Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    Atom *protocols;
    int count;
    int supports_delete = 0;
    
    if (XGetWMProtocols(dpy, mon->selected->win, &protocols, &count)) {
        for (int i = 0; i < count; i++) {
            if (protocols[i] == wm_delete) {
                supports_delete = 1;
                break;
            }
        }
        XFree(protocols);
    }
    
    if (supports_delete) {
        ev.type = ClientMessage;
        ev.xclient.window = mon->selected->win;
        ev.xclient.message_type = wm_protocols;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = wm_delete;
        ev.xclient.data.l[1] = CurrentTime;
        XSendEvent(dpy, mon->selected->win, False, NoEventMask, &ev);
    } else {
        XKillClient(dpy, mon->selected->win);
    }
}

void toggle_floating(const char *arg) {
    (void)arg;
    
    if (!mon->selected) {
        return;
    }
    
    mon->selected->is_floating = !mon->selected->is_floating;
    
    if (mon->selected->is_floating) {
        /* Restore old geometry */
        resize_client(mon->selected,
            mon->selected->old_x,
            mon->selected->old_y,
            mon->selected->old_w,
            mon->selected->old_h);
    } else {
        /* Save current geometry */
        mon->selected->old_x = mon->selected->x;
        mon->selected->old_y = mon->selected->y;
        mon->selected->old_w = mon->selected->w;
        mon->selected->old_h = mon->selected->h;
    }
    
    apply_layout();
}

void toggle_fullscreen(const char *arg) {
    (void)arg;
    
    if (!mon->selected) {
        return;
    }
    
    mon->selected->is_fullscreen = !mon->selected->is_fullscreen;
    
    if (mon->selected->is_fullscreen) {
        /* Save old geometry */
        mon->selected->old_x = mon->selected->x;
        mon->selected->old_y = mon->selected->y;
        mon->selected->old_w = mon->selected->w;
        mon->selected->old_h = mon->selected->h;
        resize_client(mon->selected, 0, 0, screen_width, screen_height);
        XRaiseWindow(dpy, mon->selected->win);
    } else {
        /* Restore old geometry */
        resize_client(mon->selected,
            mon->selected->old_x,
            mon->selected->old_y,
            mon->selected->old_w,
            mon->selected->old_h);
        apply_layout();
    }
}

void focus_next(const char *arg) {
    (void)arg;
    
    if (!mon->selected || !mon->selected->next) {
        return;
    }
    
    focus_client(mon->selected->next);
}

void focus_prev(const char *arg) {
    (void)arg;
    
    if (!mon->selected || !mon->selected->prev) {
        return;
    }
    
    focus_client(mon->selected->prev);
}

void set_master_factor(const char *arg) {
    if (!arg) {
        return;
    }
    
    float delta = atof(arg);
    float new_factor = mon->master_factor + delta;
    
    if (new_factor >= 0.1 && new_factor <= 0.9) {
        mon->master_factor = new_factor;
        apply_layout();
    }
}

void inc_num_master(const char *arg) {
    (void)arg;
    
    mon->num_master++;
    apply_layout();
}

void dec_num_master(const char *arg) {
    (void)arg;
    
    if (mon->num_master > 0) {
        mon->num_master--;
        apply_layout();
    }
}
