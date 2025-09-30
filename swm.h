/*
 * Simple Window Manager (SWM)
 * A tiling window manager for X11
 */

#ifndef SWM_H
#define SWM_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Client Client;
typedef struct Monitor Monitor;
typedef struct KeyBinding KeyBinding;
typedef struct TilingLayout TilingLayout;
typedef struct Config Config;

/* Client (Window) structure */
struct Client {
    Window win;
    int x, y, w, h;
    int old_x, old_y, old_w, old_h;
    bool is_floating;
    bool is_fullscreen;
    Client *next;
    Client *prev;
};

/* Tiling layout function pointer */
typedef void (*LayoutFunc)(Monitor *m);

/* Tiling layout structure */
struct TilingLayout {
    const char *name;
    LayoutFunc apply;
};

/* Monitor (workspace) structure */
struct Monitor {
    int x, y, w, h;
    Client *clients;
    Client *selected;
    TilingLayout *layout;
    float master_factor;
    int num_master;
};

/* Key binding structure */
struct KeyBinding {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const char *arg);
    const char *arg;
};

/* System tray client structure */
typedef struct TrayClient {
    Window win;
    int x, y, w, h;
    struct TrayClient *next;
} TrayClient;

/* System tray structure */
typedef struct {
    Window win;
    int x, y, w, h;
    TrayClient *clients;
} SystemTray;

/* Configuration structure */
struct Config {
    const char *font;
    unsigned int border_width;
    unsigned long border_normal;
    unsigned long border_focus;
    float master_factor;
    int num_master;
    KeyBinding *keys;
    int num_keys;
    TilingLayout *layouts;
    int num_layouts;
};

/* Global variables */
extern Display *dpy;
extern Window root;
extern Monitor *mon;
extern Config config;
extern SystemTray *tray;
extern int screen;
extern int screen_width, screen_height;
extern bool running;

/* Core functions */
void setup(void);
void cleanup(void);
void run(void);
void scan(void);

/* Event handlers */
void on_configure_request(XEvent *e);
void on_map_request(XEvent *e);
void on_unmap_notify(XEvent *e);
void on_destroy_notify(XEvent *e);
void on_enter_notify(XEvent *e);
void on_key_press(XEvent *e);
void on_button_press(XEvent *e);
void on_client_message(XEvent *e);

/* Client management */
Client* create_client(Window w);
void attach_client(Client *c);
void detach_client(Client *c);
void focus_client(Client *c);
void remove_client(Client *c);
void configure_client(Client *c);
void resize_client(Client *c, int x, int y, int w, int h);
void show_client(Client *c);
void hide_client(Client *c);

/* Layout functions */
void tile_layout(Monitor *m);
void monocle_layout(Monitor *m);
void floating_layout(Monitor *m);
void grid_layout(Monitor *m);
void apply_layout(void);
void set_layout(const char *arg);

/* Key bindings */
void grab_keys(void);
void spawn(const char *arg);
void quit_wm(const char *arg);
void kill_client(const char *arg);
void toggle_floating(const char *arg);
void toggle_fullscreen(const char *arg);
void focus_next(const char *arg);
void focus_prev(const char *arg);
void set_master_factor(const char *arg);
void inc_num_master(const char *arg);
void dec_num_master(const char *arg);

/* System tray */
SystemTray* create_tray(void);
void destroy_tray(SystemTray *t);
void add_tray_client(Window w);
void remove_tray_client(Window w);
void update_tray_layout(void);

/* Utility functions */
unsigned long get_color(const char *color);
void die(const char *errstr);

#endif /* SWM_H */
