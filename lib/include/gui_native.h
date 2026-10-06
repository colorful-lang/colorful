/* Colorful gui module - native back end (C ABI).
 *
 * The Colorful side (`gui/gui.cf`) is written in Colorful; this file is the
 * "cpp half" of the module: it owns the window, the Direct2D device, the Roboto
 * / Material Symbols text formats and the Material 3 paint helpers.
 *
 * Rules this back end follows (see docs/gui.md):
 *   - it is a *renderer*, never a layout/interpreter engine: every widget call
 *     comes from compiled Colorful code and is executed immediately
 *   - all state that must survive a frame (widget hover/press/ripple, the
 *     application's own key-value store) is plain native memory, not a script
 *     interpreter state
 *   - rendering can target a window (interactive) or a WIC bitmap (headless,
 *     `cf_gui_render_png`) so the very same drawing code can be tested in CI
 */
#ifndef COLORFUL_GUI_NATIVE_H
#define COLORFUL_GUI_NATIVE_H

#include "runtime.h" /* cf_str, cf_closure */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CF_EXPORT
#if defined(_WIN32)
#define CF_EXPORT __declspec(dllexport)
#else
#define CF_EXPORT __attribute__((visibility("default")))
#endif
#endif

/* -------------------------------------------------------------- lifecycle -- */
CF_EXPORT void cf_gui_init(void);
CF_EXPORT void cf_gui_shutdown(void);
/* 1 when the native back end (Direct2D + DirectWrite) is usable. */
CF_EXPORT int64_t cf_gui_available(void);
/* 1 when Roboto and Material Symbols were loaded from the packaged fonts. */
CF_EXPORT int64_t cf_gui_fonts_ready(void);
CF_EXPORT cf_str* cf_gui_font_report(void);

/* ---------------------------------------------------------------- windows -- */
/* Creates a window; returns a handle (0 on failure). Size is in logical pixels. */
CF_EXPORT int64_t cf_gui_window(cf_str* title, int64_t w, int64_t h);
CF_EXPORT void cf_gui_set_title(int64_t win, cf_str* title);
CF_EXPORT void cf_gui_close(int64_t win);
/* Runs the message loop, calling `frame(frameNumber)` before every repaint. */
CF_EXPORT int64_t cf_gui_run(int64_t win, cf_closure frame);
/* Renders one frame into a PNG file; returns the number of draw operations. */
CF_EXPORT int64_t cf_gui_render_png(cf_str* path, int64_t w, int64_t h, cf_closure frame);

/* ------------------------------------------------------------------ theme -- */
/* Material 3 colour roles, so widgets can paint themselves correctly. */
CF_EXPORT void cf_gui_theme(int64_t primary, int64_t on_primary, int64_t primary_container,
                            int64_t on_primary_container, int64_t secondary_container,
                            int64_t on_secondary_container, int64_t tertiary_container,
                            int64_t on_tertiary_container, int64_t surface, int64_t surface_variant,
                            int64_t surface_container, int64_t on_surface,
                            int64_t on_surface_variant, int64_t outline, int64_t outline_variant,
                            int64_t error, int64_t on_error);
CF_EXPORT double cf_gui_dpi(void);
/* Colour roles by name: "primary", "surface_container_low", "outline", ... */
CF_EXPORT int64_t cf_gui_set_color(cf_str* role, int64_t color);
CF_EXPORT int64_t cf_gui_color(cf_str* role);

/* --------------------------------------------------------------- painting -- */
CF_EXPORT void cf_gui_clear(int64_t color);
CF_EXPORT void cf_gui_rect(double x, double y, double w, double h, int64_t color, double radius);
CF_EXPORT void cf_gui_stroke(double x, double y, double w, double h, int64_t color, double radius,
                             double width);
CF_EXPORT void cf_gui_circle(double cx, double cy, double r, int64_t color);
CF_EXPORT void cf_gui_line(double x1, double y1, double x2, double y2, int64_t color, double width);
/* M3 tonal elevation: layered translucent rounded rectangles. */
CF_EXPORT void cf_gui_shadow(double x, double y, double w, double h, double radius,
                             int64_t elevation);
/* Text with the M3 type scale (style 0..14, see gui.cf). */
CF_EXPORT void cf_gui_text(cf_str* text, double x, double y, int64_t style, int64_t color);
CF_EXPORT double cf_gui_text_width(cf_str* text, int64_t style);
CF_EXPORT double cf_gui_text_height(int64_t style);
/* Material Symbols icon by ligature name ("home", "settings", "star", ...). */
CF_EXPORT void cf_gui_icon(cf_str* name, double x, double y, double size, int64_t color);
CF_EXPORT double cf_gui_icon_width(cf_str* name, double size);

/* --------------------------------------------------------------- widgets -- */
/* kind: 0 filled, 1 tonal, 2 outlined, 3 text, 4 elevated.
 * `icon` is an optional Material Symbols name ("" for none). */
CF_EXPORT int64_t cf_gui_button(cf_str* id, double x, double y, double w, double h, cf_str* label,
                                cf_str* icon, int64_t kind);
CF_EXPORT int64_t cf_gui_icon_button(cf_str* id, double x, double y, double size, cf_str* icon,
                                     int64_t kind);
CF_EXPORT int64_t cf_gui_checkbox(cf_str* id, double x, double y, cf_str* label, int64_t checked);
CF_EXPORT int64_t cf_gui_switch(cf_str* id, double x, double y, int64_t on);
CF_EXPORT double cf_gui_slider(cf_str* id, double x, double y, double w, double value);
CF_EXPORT int64_t cf_gui_chip(cf_str* id, double x, double y, cf_str* label, int64_t selected);
CF_EXPORT int64_t cf_gui_tab(cf_str* id, double x, double y, double w, cf_str* label,
                             int64_t selected);
/* Invisible interactive area: lets Colorful draw a custom widget and still get
 * hover / press / click handling from the module. */
CF_EXPORT int64_t cf_gui_hotspot(cf_str* id, double x, double y, double w, double h);
/* Draws a list item / card row and reports whether it was activated. */
CF_EXPORT int64_t cf_gui_list_item(cf_str* id, double x, double y, double w, double h,
                                   cf_str* headline, cf_str* supporting, cf_str* leading_icon,
                                   cf_str* trailing_icon);
/* Snackbar style message; `seconds` <= 0 draws it without a timer. */
CF_EXPORT void cf_gui_snackbar(cf_str* text, double x, double y, double w, int64_t color,
                               int64_t text_color);
CF_EXPORT void cf_gui_divider(double x, double y, double w);

/* ------------------------------------------------------- state and input --- */
/* The application store: the Colorful side has no globals, so per-frame state
 * lives here, owned by the module and shared by every callback frame. */
CF_EXPORT int64_t cf_gui_state_get(cf_str* key);
CF_EXPORT void cf_gui_state_set(cf_str* key, int64_t value);
CF_EXPORT double cf_gui_state_getf(cf_str* key);
CF_EXPORT void cf_gui_state_setf(cf_str* key, double value);
CF_EXPORT int64_t cf_gui_state_has(cf_str* key);
CF_EXPORT void cf_gui_state_clear(void);

CF_EXPORT double cf_gui_mouse_x(void);
CF_EXPORT double cf_gui_mouse_y(void);
CF_EXPORT int64_t cf_gui_mouse_down(void);
/* Interaction state of a widget by id (works for custom, hand drawn widgets). */
CF_EXPORT int64_t cf_gui_hovered(cf_str* id);
CF_EXPORT int64_t cf_gui_pressed(cf_str* id);

/* Synthetic pointer: the automated hit tests (colortest) drive the very same
 * widget code the window uses, without a window. `test_click` performs a full
 * press + release; the widget fires on the following frame. */
CF_EXPORT void cf_gui_test_mouse(double x, double y, int64_t down);
CF_EXPORT void cf_gui_test_click(double x, double y);
CF_EXPORT void cf_gui_test_press(double x, double y);
CF_EXPORT void cf_gui_test_release(double x, double y);
CF_EXPORT void cf_gui_test_clear(void);
CF_EXPORT int64_t cf_gui_frame(void);
CF_EXPORT double cf_gui_time(void);
CF_EXPORT double cf_gui_dt(void);
CF_EXPORT int64_t cf_gui_width(void);
CF_EXPORT int64_t cf_gui_height(void);

/* Native message box, used by the module's `gui.error` helper. */
CF_EXPORT void cf_gui_message_box(cf_str* title, cf_str* body);

#ifdef __cplusplus
}
#endif
#endif /* COLORFUL_GUI_NATIVE_H */
