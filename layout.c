/*
 * Tiling Layout Algorithms
 * Customizable layouts for window management
 */

#include <math.h>
#include <string.h>
#include "swm.h"

/* Count visible clients */
static int count_clients(Monitor *m) {
    int n = 0;
    Client *c;
    for (c = m->clients; c; c = c->next) {
        if (!c->is_floating && !c->is_fullscreen) {
            n++;
        }
    }
    return n;
}

/* Tile layout: Master on left, stack on right */
void tile_layout(Monitor *m) {
    int n = count_clients(m);
    if (n == 0) {
        return;
    }
    
    int master_width = (n > m->num_master) ? m->w * m->master_factor : m->w;
    int stack_width = m->w - master_width;
    int master_height = (m->num_master > 0) ? m->h / m->num_master : m->h;
    int stack_count = n - m->num_master;
    int stack_height = (stack_count > 0) ? m->h / stack_count : m->h;
    
    int i = 0;
    Client *c;
    for (c = m->clients; c; c = c->next) {
        if (c->is_floating || c->is_fullscreen) {
            continue;
        }
        
        if (i < m->num_master) {
            /* Master area */
            resize_client(c, 
                m->x,
                m->y + i * master_height,
                master_width - 2 * config.border_width,
                master_height - 2 * config.border_width);
        } else {
            /* Stack area */
            int stack_idx = i - m->num_master;
            resize_client(c,
                m->x + master_width,
                m->y + stack_idx * stack_height,
                stack_width - 2 * config.border_width,
                stack_height - 2 * config.border_width);
        }
        show_client(c);
        i++;
    }
}

/* Monocle layout: One window at a time, fullscreen */
void monocle_layout(Monitor *m) {
    Client *c;
    
    for (c = m->clients; c; c = c->next) {
        if (c->is_floating || c->is_fullscreen) {
            continue;
        }
        
        if (c == m->selected) {
            resize_client(c,
                m->x,
                m->y,
                m->w - 2 * config.border_width,
                m->h - 2 * config.border_width);
            show_client(c);
        } else {
            hide_client(c);
        }
    }
}

/* Floating layout: No tiling */
void floating_layout(Monitor *m) {
    Client *c;
    
    for (c = m->clients; c; c = c->next) {
        if (c->is_fullscreen) {
            continue;
        }
        show_client(c);
    }
}

/* Grid layout: Arrange windows in a grid */
void grid_layout(Monitor *m) {
    int n = count_clients(m);
    if (n == 0) {
        return;
    }
    
    /* Calculate grid dimensions */
    int cols = (int)ceil(sqrt(n));
    int rows = (int)ceil((double)n / cols);
    int cell_w = m->w / cols;
    int cell_h = m->h / rows;
    
    int i = 0;
    Client *c;
    for (c = m->clients; c; c = c->next) {
        if (c->is_floating || c->is_fullscreen) {
            continue;
        }
        
        int col = i % cols;
        int row = i / cols;
        
        resize_client(c,
            m->x + col * cell_w,
            m->y + row * cell_h,
            cell_w - 2 * config.border_width,
            cell_h - 2 * config.border_width);
        show_client(c);
        i++;
    }
}

void apply_layout(void) {
    if (!mon || !mon->layout) {
        return;
    }
    
    /* Handle fullscreen clients */
    Client *c;
    for (c = mon->clients; c; c = c->next) {
        if (c->is_fullscreen) {
            resize_client(c, 0, 0, screen_width, screen_height);
            XRaiseWindow(dpy, c->win);
        }
    }
    
    /* Apply layout to non-fullscreen clients */
    mon->layout->apply(mon);
    
    XSync(dpy, False);
}

void set_layout(const char *arg) {
    if (!arg) {
        return;
    }
    
    for (int i = 0; i < config.num_layouts; i++) {
        if (strcmp(config.layouts[i].name, arg) == 0) {
            mon->layout = &config.layouts[i];
            apply_layout();
            return;
        }
    }
}
