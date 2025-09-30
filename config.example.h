/*
 * SWM Configuration Example
 * Copy this file to config.h and customize to your needs
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "swm.h"

/* ============================================
 * APPEARANCE
 * ============================================ */

/* Border width in pixels */
#define BORDER_WIDTH        3

/* Border colors (X11 color names or hex codes) */
#define BORDER_NORMAL       "#333333"
#define BORDER_FOCUS        "#0088cc"

/* Master area factor (0.1 - 0.9) */
#define MASTER_FACTOR       0.55

/* Number of windows in master area */
#define NUM_MASTER          1

/* ============================================
 * MODKEY
 * ============================================ */

/* Modkey: Mod1Mask = Alt, Mod4Mask = Super/Windows key */
#define MODKEY Mod4Mask

/* ============================================
 * SYSTEM TRAY
 * ============================================ */

#define TRAY_HEIGHT         24
#define TRAY_WIDTH          300

/* ============================================
 * KEY BINDINGS
 * ============================================
 * Format: { modifier, key, function, argument }
 * 
 * Available modifiers:
 * - MODKEY (defined above)
 * - ShiftMask
 * - ControlMask
 * - Mod1Mask (Alt)
 * - Mod4Mask (Super)
 * 
 * Combine with: MODKEY|ShiftMask
 * 
 * Available functions:
 * - spawn(cmd)              : Run a command
 * - kill_client()           : Close focused window
 * - quit_wm()               : Exit window manager
 * - focus_next()            : Focus next window
 * - focus_prev()            : Focus previous window
 * - set_master_factor(val)  : Adjust master area size (+0.05 or -0.05)
 * - inc_num_master()        : Increase master window count
 * - dec_num_master()        : Decrease master window count
 * - toggle_floating()       : Toggle floating mode
 * - toggle_fullscreen()     : Toggle fullscreen mode
 * - set_layout(name)        : Change layout (tile, monocle, floating, grid)
 */

static KeyBinding keys[] = {
    /* modifier              key                function              argument */
    
    /* Application launchers */
    { MODKEY,                XK_Return,         spawn,                "xterm" },
    { MODKEY,                XK_d,              spawn,                "dmenu_run" },
    { MODKEY,                XK_w,              spawn,                "firefox" },
    { MODKEY,                XK_e,              spawn,                "thunar" },
    
    /* Window management */
    { MODKEY,                XK_q,              kill_client,          NULL },
    { MODKEY,                XK_j,              focus_next,           NULL },
    { MODKEY,                XK_k,              focus_prev,           NULL },
    { MODKEY,                XK_f,              toggle_fullscreen,    NULL },
    { MODKEY,                XK_space,          toggle_floating,      NULL },
    
    /* Layout control */
    { MODKEY,                XK_h,              set_master_factor,    "-0.05" },
    { MODKEY,                XK_l,              set_master_factor,    "+0.05" },
    { MODKEY,                XK_i,              inc_num_master,       NULL },
    { MODKEY,                XK_o,              dec_num_master,       NULL },
    
    /* Layout selection */
    { MODKEY,                XK_t,              set_layout,           "tile" },
    { MODKEY,                XK_m,              set_layout,           "monocle" },
    { MODKEY,                XK_g,              set_layout,           "grid" },
    { MODKEY,                XK_s,              set_layout,           "floating" },
    
    /* System */
    { MODKEY|ShiftMask,      XK_q,              quit_wm,              NULL },
    { MODKEY|ShiftMask,      XK_r,              spawn,                "swm" }, /* Restart WM */
};

/* ============================================
 * TILING LAYOUTS
 * ============================================
 * Format: { name, layout_function }
 * 
 * Available layouts:
 * - tile_layout      : Master/stack tiling
 * - monocle_layout   : One window fullscreen
 * - floating_layout  : No tiling
 * - grid_layout      : Grid arrangement
 * 
 * The first layout in the array is the default.
 */

static TilingLayout layouts[] = {
    { "tile",     tile_layout },
    { "monocle",  monocle_layout },
    { "floating", floating_layout },
    { "grid",     grid_layout },
};

#endif /* CONFIG_H */
