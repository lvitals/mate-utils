/* Copyright (C) 2001-2006 Jonathan Blandford <jrb@alum.mit.edu>
 * Copyright (C) 2008 Cosimo Cecchi <cosimoc@gnome.org>
 * Copyright (C) 2012-2021 MATE Developers
 *
 * This file is part of MATE Utils.
 *
 * MATE Utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * MATE Utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MATE Utils.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "screenshot-utils.h"

#ifdef GDK_WINDOWING_X11
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#endif
#ifdef MATE_SCREENSHOT_ENABLE_WAYLAND
#include <gdk/gdkwayland.h>
#include <wayland-client.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
#include <X11/extensions/shape.h>
#endif

static GtkWidget *selection_window;

#define SELECTION_NAME "_MATE_PANEL_SCREENSHOT"

static gboolean
is_wayland (void)
{
#ifdef MATE_SCREENSHOT_ENABLE_WAYLAND
  GdkDisplay *display = gdk_display_get_default ();
  return GDK_IS_WAYLAND_DISPLAY (display);
#else
  return FALSE;
#endif
}

gboolean
screenshot_is_wayland (void)
{
  return is_wayland ();
}

/* To make sure there is only one screenshot taken at a time,
 * (Imagine key repeat for the print screen key) we hold a selection
 * until we are done taking the screenshot
 */
gboolean
screenshot_grab_lock (void)
{
  GdkAtom selection_atom;
  gboolean result = FALSE;
  GdkDisplay *display;

  if (is_wayland ())
    return TRUE;

#ifdef GDK_WINDOWING_X11
  selection_atom = gdk_atom_intern (SELECTION_NAME, FALSE);
  gdk_x11_grab_server ();

  if (gdk_selection_owner_get (selection_atom) != NULL)
    goto out;

  selection_window = gtk_invisible_new ();
  gtk_widget_show (selection_window);

  if (!gtk_selection_owner_set (selection_window,
				gdk_atom_intern (SELECTION_NAME, FALSE),
				GDK_CURRENT_TIME))
    {
      gtk_widget_destroy (selection_window);
      selection_window = NULL;
      goto out;
    }

  result = TRUE;

 out:
  gdk_x11_ungrab_server ();

  display = gdk_display_get_default ();
  gdk_display_flush (display);
#endif

  return result;
}

void
screenshot_release_lock (void)
{
  GdkDisplay *display;

  if (is_wayland ())
    return;

#ifdef GDK_WINDOWING_X11
  if (selection_window)
    {
      gtk_widget_destroy (selection_window);
      selection_window = NULL;
    }

  display = gdk_display_get_default ();
  gdk_display_flush (display);
#endif
}

static GdkWindow *
screen_get_active_window (GdkScreen *screen)
{
  GdkWindow *ret = NULL;

  if (is_wayland ())
    return NULL;

#ifdef GDK_WINDOWING_X11
  Atom type_return;
  gint format_return;
  gulong nitems_return;
  gulong bytes_after_return;
  guchar *data = NULL;

  if (!gdk_x11_screen_supports_net_wm_hint (screen,
                                            gdk_atom_intern_static_string ("_NET_ACTIVE_WINDOW")))
    return NULL;

  if (XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_screen_get_display (screen)),
                          RootWindow (GDK_DISPLAY_XDISPLAY (gdk_screen_get_display (screen)),
                                      GDK_SCREEN_XNUMBER (screen)),
                          gdk_x11_get_xatom_by_name_for_display (gdk_screen_get_display (screen),
                                                                 "_NET_ACTIVE_WINDOW"),
                          0, 1, False, XA_WINDOW, &type_return,
                          &format_return, &nitems_return,
                          &bytes_after_return, &data)
      == Success)
    {
      if ((type_return == XA_WINDOW) && (format_return == 32) && (data))
        {
          Window window = *(Window *) data;

          if (window != None)
            {
              ret = gdk_x11_window_foreign_new_for_display (gdk_screen_get_display (screen),
                                                            window);
            }
        }
    }

  if (data)
    XFree (data);
#endif

  return ret;
}

static GdkWindow *
screenshot_find_active_window (void)
{
  GdkWindow *window;
  GdkScreen *default_screen;

  if (is_wayland ())
    return NULL;

  default_screen = gdk_screen_get_default ();
  window = screen_get_active_window (default_screen);

  return window;
}

static gboolean
screenshot_window_is_desktop (GdkWindow *window)
{
  GdkWindow *root_window = gdk_get_default_root_window ();
  GdkWindowTypeHint window_type_hint;

  if (is_wayland ())
    return FALSE;

  if (window == root_window)
    return TRUE;

  window_type_hint = gdk_window_get_type_hint (window);
  if (window_type_hint == GDK_WINDOW_TYPE_HINT_DESKTOP)
    return TRUE;

  return FALSE;

}

GdkWindow *
screenshot_find_current_window ()
{
  GdkWindow *current_window;
  GdkDisplay *display;
  GdkSeat *seat;
  GdkDevice *device;

  if (is_wayland ())
    return NULL;

  current_window = screenshot_find_active_window ();
  display = gdk_display_get_default ();
  seat = gdk_display_get_default_seat (display);
  device = gdk_seat_get_pointer (seat);

  /* If there's no active window, we fall back to returning the
   * window that the cursor is in.
   */
  if (!current_window)
    current_window = gdk_device_get_window_at_position (device, NULL, NULL);

  if (current_window)
    {
      if (screenshot_window_is_desktop (current_window))
	/* if the current window is the desktop (e.g. caja), we
	 * return NULL, as getting the whole screen makes more sense.
	 */
        return NULL;

      /* Once we have a window, we take the toplevel ancestor. */
      current_window = gdk_window_get_toplevel (current_window);
    }

  return current_window;
}

typedef struct {
  GdkRectangle rect;
  gboolean button_pressed;
  GtkWidget *window;
} select_area_filter_data;

static gboolean
select_area_button_press (GtkWidget               *window,
                          GdkEventButton          *event,
			  select_area_filter_data *data)
{
  if (data->button_pressed)
    return TRUE;

  data->button_pressed = TRUE;
  data->rect.x = event->x_root;
  data->rect.y = event->y_root;

  return TRUE;
}

static gboolean
select_area_motion_notify (GtkWidget               *window,
                           GdkEventMotion          *event,
                           select_area_filter_data *data)
{
  GdkRectangle draw_rect;

  if (!data->button_pressed)
    return TRUE;

  draw_rect.width = ABS (data->rect.x - event->x_root);
  draw_rect.height = ABS (data->rect.y - event->y_root);
  draw_rect.x = MIN (data->rect.x, event->x_root);
  draw_rect.y = MIN (data->rect.y, event->y_root);

  if (draw_rect.width <= 0 || draw_rect.height <= 0)
    {
      gtk_window_move (GTK_WINDOW (window), -100, -100);
      gtk_window_resize (GTK_WINDOW (window), 10, 10);
      return TRUE;
    }

  gtk_window_move (GTK_WINDOW (window), draw_rect.x, draw_rect.y);
  gtk_window_resize (GTK_WINDOW (window), draw_rect.width, draw_rect.height);

  /* We (ab)use app-paintable to indicate if we have an RGBA window */
  if (!gtk_widget_get_app_paintable (window))
    {
      GdkWindow *gdkwindow = gtk_widget_get_window (window);

      /* Shape the window to make only the outline visible */
      if (draw_rect.width > 2 && draw_rect.height > 2)
        {
          cairo_region_t *region, *region2;
          cairo_rectangle_int_t region_rect = {
	    0, 0,
            draw_rect.width - 2, draw_rect.height - 2
          };

          region = cairo_region_create_rectangle (&region_rect);
          region_rect.x++;
          region_rect.y++;
          region_rect.width -= 2;
          region_rect.height -= 2;
          region2 = cairo_region_create_rectangle (&region_rect);
          cairo_region_subtract (region, region2);

          gdk_window_shape_combine_region (gdkwindow, region, 0, 0);

          cairo_region_destroy (region);
          cairo_region_destroy (region2);
        }
      else
        gdk_window_shape_combine_region (gdkwindow, NULL, 0, 0);
    }

  return TRUE;
}

static gboolean
select_area_button_release (GtkWidget *window,
                            GdkEventButton *event,
                            select_area_filter_data *data)
{
  if (!data->button_pressed)
    return TRUE;

  data->rect.width = ABS (data->rect.x - event->x_root);
  data->rect.height = ABS (data->rect.y - event->y_root);
  data->rect.x = MIN (data->rect.x, event->x_root);
  data->rect.y = MIN (data->rect.y, event->y_root);

  gtk_main_quit ();

  return TRUE;
}

static gboolean
select_area_key_press (GtkWidget *window,
                       GdkEventKey *event,
                       select_area_filter_data *data)
{
  if (event->keyval == GDK_KEY_Escape)
    {
      data->rect.x = 0;
      data->rect.y = 0;
      data->rect.width  = 0;
      data->rect.height = 0;
      gtk_main_quit ();
    }

  return TRUE;
}

static gboolean
draw (GtkWidget *window, cairo_t *cr, gpointer unused)
{
  GtkStyleContext *style;

  style = gtk_widget_get_style_context (window);

  if (gtk_widget_get_app_paintable (window))
    {
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_set_source_rgba (cr, 0, 0, 0, 0);
      cairo_paint (cr);

      gtk_style_context_save (style);
      gtk_style_context_add_class (style, GTK_STYLE_CLASS_RUBBERBAND);

      gtk_render_background (style, cr,
                             0, 0,
                             gtk_widget_get_allocated_width (window),
                             gtk_widget_get_allocated_height (window));
      gtk_render_frame (style, cr,
                        0, 0,
                        gtk_widget_get_allocated_width (window),
                        gtk_widget_get_allocated_height (window));

      gtk_style_context_restore (style);
    }

  return TRUE;
}

static GtkWidget *
create_select_window (void)
{
  GdkScreen *screen = gdk_screen_get_default ();
  GtkWidget *window = gtk_window_new (GTK_WINDOW_POPUP);

  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);
  if (gdk_screen_is_composited (screen) && visual)
    {
      gtk_widget_set_visual (window, visual);
      gtk_widget_set_app_paintable (window, TRUE);
    }

  g_signal_connect (window, "draw", G_CALLBACK (draw), NULL);

  gtk_window_move (GTK_WINDOW (window), -100, -100);
  gtk_window_resize (GTK_WINDOW (window), 10, 10);
  gtk_widget_show (window);
  return window;
}

typedef struct {
  GdkRectangle rectangle;
  SelectAreaCallback callback;
} CallbackData;

#ifdef MATE_SCREENSHOT_ENABLE_WAYLAND
static gboolean
screenshot_select_area_wayland (GdkRectangle *rectangle)
{
  GSubprocess *process;
  GBytes *stdout_bytes = NULL;
  GBytes *stderr_bytes = NULL;
  GError *error = NULL;
  const gchar *stdout_data;
  gchar *geometry;
  gsize stdout_size = 0;
  gint matched;

  process = g_subprocess_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                              G_SUBPROCESS_FLAGS_STDERR_PIPE,
                              &error,
                              "slurp",
                              NULL);
  if (!process)
    {
      g_warning ("Error launching slurp area selector: %s", error->message);
      g_error_free (error);
      return FALSE;
    }

  if (!g_subprocess_communicate (process, NULL, NULL,
                                 &stdout_bytes, &stderr_bytes, &error))
    {
      g_warning ("Error reading slurp area selector output: %s", error->message);
      g_error_free (error);
      g_object_unref (process);
      return FALSE;
    }

  if (!g_subprocess_get_successful (process))
    {
      if (stderr_bytes)
        g_bytes_unref (stderr_bytes);
      if (stdout_bytes)
        g_bytes_unref (stdout_bytes);
      g_object_unref (process);
      return FALSE;
    }

  stdout_data = g_bytes_get_data (stdout_bytes, &stdout_size);
  geometry = g_strndup (stdout_data, stdout_size);
  matched = sscanf (geometry, "%d,%d %dx%d",
                    &rectangle->x,
                    &rectangle->y,
                    &rectangle->width,
                    &rectangle->height);
  g_free (geometry);

  if (stderr_bytes)
    g_bytes_unref (stderr_bytes);
  g_bytes_unref (stdout_bytes);
  g_object_unref (process);

  return matched == 4 &&
         rectangle->width > 0 &&
         rectangle->height > 0;
}
#endif

static gboolean
emit_select_callback_in_idle (gpointer user_data)
{
  CallbackData *data = user_data;

  data->callback (&data->rectangle);

  g_slice_free (CallbackData, data);

  return FALSE;
}

void
screenshot_select_area_async (SelectAreaCallback callback)
{
  GdkDisplay *display;
  GdkCursor               *cursor;
  GdkSeat *seat;
  GdkGrabStatus res;
  select_area_filter_data  data;
  CallbackData *cb_data;

  if (is_wayland ())
    {
      GdkRectangle rect = { 0, 0, 0, 0 };
      screenshot_select_area_wayland (&rect);
      callback (&rect);
      return;
    }

  data.rect.x = 0;
  data.rect.y = 0;
  data.rect.width  = 0;
  data.rect.height = 0;
  data.button_pressed = FALSE;
  data.window = create_select_window();

  cb_data = g_slice_new0 (CallbackData);
  cb_data->callback = callback;

  g_signal_connect (data.window, "key-press-event", G_CALLBACK (select_area_key_press), &data);
  g_signal_connect (data.window, "button-press-event", G_CALLBACK (select_area_button_press), &data);
  g_signal_connect (data.window, "button-release-event", G_CALLBACK (select_area_button_release), &data);
  g_signal_connect (data.window, "motion-notify-event", G_CALLBACK (select_area_motion_notify), &data);

  display = gdk_display_get_default ();
  cursor = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);

  seat = gdk_display_get_default_seat (display);

  res = gdk_seat_grab (seat,
                       gtk_widget_get_window (data.window),
                       GDK_SEAT_CAPABILITY_ALL,
                       FALSE,
                       cursor,
                       NULL,
                       NULL,
                       NULL);

  if (res != GDK_GRAB_SUCCESS)
    {
      g_object_unref (cursor);
      goto out;
    }

  gtk_main ();

  gdk_seat_ungrab (seat);

  gtk_widget_destroy (data.window);
  g_object_unref (cursor);
  gdk_display_flush (display);

out:
  cb_data->rectangle = data.rect;

  /* FIXME: we should actually be emitting the callback When
   * the compositor has finished re-drawing, but there seems to be no easy
   * way to know that.
   */
  g_timeout_add (200, emit_select_callback_in_idle, cb_data);
}

#ifdef GDK_WINDOWING_X11
static Window
find_wm_window (Window xid)
{
  Window root, parent, *children;
  unsigned int nchildren;

  do
    {
      if (XQueryTree (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), xid, &root,
		      &parent, &children, &nchildren) == 0)
	{
	  g_warning ("Couldn't find window manager window");
	  return None;
	}

      if (root == parent)
	return xid;

      xid = parent;
    }
  while (TRUE);
}
#endif

static cairo_region_t *
make_region_with_monitors (GdkScreen *screen)
{
  GdkDisplay     *display;
  cairo_region_t *region;
  int num_monitors;
  int i;

  display = gdk_screen_get_display (screen);
  num_monitors = gdk_display_get_n_monitors (display);

  region = cairo_region_create ();

  for (i = 0; i < num_monitors; i++)
    {
      GdkRectangle rect;

      gdk_monitor_get_geometry (gdk_display_get_monitor (display, i), &rect);
      cairo_region_union_rectangle (region, &rect);
    }

  return region;
}

static void
blank_rectangle_in_pixbuf (GdkPixbuf *pixbuf, GdkRectangle *rect)
{
  int x, y;
  int x2, y2;
  guchar *pixels;
  int rowstride;
  int n_channels;
  guchar *row;
  gboolean has_alpha;

  g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);

  x2 = rect->x + rect->width;
  y2 = rect->y + rect->height;

  pixels = gdk_pixbuf_get_pixels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  for (y = rect->y; y < y2; y++)
    {
      guchar *p;

      row = pixels + y * rowstride;
      p = row + rect->x * n_channels;

      for (x = rect->x; x < x2; x++)
	{
	  *p++ = 0;
	  *p++ = 0;
	  *p++ = 0;

	  if (has_alpha)
	    *p++ = 255; /* opaque black */
	}
    }
}

static void
blank_region_in_pixbuf (GdkPixbuf *pixbuf, cairo_region_t *region)
{
  int n_rects;
  int i;
  int width, height;
  cairo_rectangle_int_t pixbuf_rect;

  n_rects = cairo_region_num_rectangles (region);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  pixbuf_rect.x	     = 0;
  pixbuf_rect.y	     = 0;
  pixbuf_rect.width  = width;
  pixbuf_rect.height = height;

  for (i = 0; i < n_rects; i++)
    {
      cairo_rectangle_int_t rect, dest;

      cairo_region_get_rectangle (region, i, &rect);
      if (gdk_rectangle_intersect (&rect, &pixbuf_rect, &dest))
	blank_rectangle_in_pixbuf (pixbuf, &dest);

    }
}

/* When there are multiple monitors with different resolutions, the visible area
 * within the root window may not be rectangular (it may have an L-shape, for
 * example).  In that case, mask out the areas of the root window which would
 * not be visible in the monitors, so that screenshot do not end up with content
 * that the user won't ever see.
 */
static void
mask_monitors (GdkPixbuf *pixbuf, GdkWindow *root_window)
{
#ifdef GDK_WINDOWING_X11
  GdkScreen *screen;
  cairo_region_t *region_with_monitors;
  cairo_region_t *invisible_region;
  cairo_rectangle_int_t rect;
  gint scale;

  screen = gdk_window_get_screen (root_window);
  scale = gdk_window_get_scale_factor (root_window);

  region_with_monitors = make_region_with_monitors (screen);

  rect.x = 0;
  rect.y = 0;
  rect.width = WidthOfScreen (gdk_x11_screen_get_xscreen (screen)) / scale;
  rect.height = HeightOfScreen (gdk_x11_screen_get_xscreen (screen)) / scale;

  invisible_region = cairo_region_create_rectangle (&rect);
  cairo_region_subtract (invisible_region, region_with_monitors);

  blank_region_in_pixbuf (pixbuf, invisible_region);

  cairo_region_destroy (region_with_monitors);
  cairo_region_destroy (invisible_region);
#endif
}

#ifdef MATE_SCREENSHOT_ENABLE_WAYLAND
typedef struct {
  struct wl_proxy *proxy;
  gchar *identifier;
  gchar *title;
  gchar *app_id;
  gboolean closed;
} WaylandToplevel;

typedef struct {
  struct wl_proxy *toplevel_list;
  GPtrArray *toplevels;
} WaylandToplevelContext;

static const struct wl_message ext_foreign_toplevel_handle_v1_requests[] = {
  { "destroy", "", NULL },
};

static const struct wl_message ext_foreign_toplevel_handle_v1_events[] = {
  { "closed", "", NULL },
  { "done", "", NULL },
  { "title", "s", NULL },
  { "app_id", "s", NULL },
  { "identifier", "s", NULL },
};

static const struct wl_interface ext_foreign_toplevel_handle_v1_interface = {
  "ext_foreign_toplevel_handle_v1",
  1,
  G_N_ELEMENTS (ext_foreign_toplevel_handle_v1_requests),
  ext_foreign_toplevel_handle_v1_requests,
  G_N_ELEMENTS (ext_foreign_toplevel_handle_v1_events),
  ext_foreign_toplevel_handle_v1_events
};

static const struct wl_interface *ext_foreign_toplevel_list_v1_types[] = {
  &ext_foreign_toplevel_handle_v1_interface,
};

static const struct wl_message ext_foreign_toplevel_list_v1_requests[] = {
  { "stop", "", NULL },
  { "destroy", "", NULL },
};

static const struct wl_message ext_foreign_toplevel_list_v1_events[] = {
  { "toplevel", "n", ext_foreign_toplevel_list_v1_types },
  { "finished", "", NULL },
};

static const struct wl_interface ext_foreign_toplevel_list_v1_interface = {
  "ext_foreign_toplevel_list_v1",
  1,
  G_N_ELEMENTS (ext_foreign_toplevel_list_v1_requests),
  ext_foreign_toplevel_list_v1_requests,
  G_N_ELEMENTS (ext_foreign_toplevel_list_v1_events),
  ext_foreign_toplevel_list_v1_events
};

static void
wayland_toplevel_free (WaylandToplevel *toplevel)
{
  if (!toplevel)
    return;

  if (toplevel->proxy)
    wl_proxy_marshal_flags (toplevel->proxy,
                            0,
                            NULL,
                            wl_proxy_get_version (toplevel->proxy),
                            WL_MARSHAL_FLAG_DESTROY);

  g_free (toplevel->identifier);
  g_free (toplevel->title);
  g_free (toplevel->app_id);
  g_free (toplevel);
}

static void
wayland_toplevel_closed (void *data,
                         void *handle)
{
  WaylandToplevel *toplevel = data;

  toplevel->closed = TRUE;
}

static void
wayland_toplevel_done (void *data,
                       void *handle)
{
}

static void
wayland_toplevel_title (void       *data,
                        void       *handle,
                        const char *title)
{
  WaylandToplevel *toplevel = data;

  g_free (toplevel->title);
  toplevel->title = g_strdup (title);
}

static void
wayland_toplevel_app_id (void       *data,
                         void       *handle,
                         const char *app_id)
{
  WaylandToplevel *toplevel = data;

  g_free (toplevel->app_id);
  toplevel->app_id = g_strdup (app_id);
}

static void
wayland_toplevel_identifier (void       *data,
                             void       *handle,
                             const char *identifier)
{
  WaylandToplevel *toplevel = data;

  g_free (toplevel->identifier);
  toplevel->identifier = g_strdup (identifier);
}

static const struct {
  void (*closed)     (void *data, void *handle);
  void (*done)       (void *data, void *handle);
  void (*title)      (void *data, void *handle, const char *title);
  void (*app_id)     (void *data, void *handle, const char *app_id);
  void (*identifier) (void *data, void *handle, const char *identifier);
} wayland_toplevel_listener = {
  wayland_toplevel_closed,
  wayland_toplevel_done,
  wayland_toplevel_title,
  wayland_toplevel_app_id,
  wayland_toplevel_identifier
};

static void
wayland_toplevel_list_toplevel (void            *data,
                                void            *list,
                                struct wl_proxy *handle)
{
  WaylandToplevelContext *context = data;
  WaylandToplevel *toplevel;

  toplevel = g_new0 (WaylandToplevel, 1);
  toplevel->proxy = handle;

  wl_proxy_add_listener (handle,
                         (void (**)(void)) &wayland_toplevel_listener,
                         toplevel);

  g_ptr_array_add (context->toplevels, toplevel);
}

static void
wayland_toplevel_list_finished (void *data,
                                void *list)
{
}

static const struct {
  void (*toplevel) (void *data, void *list, struct wl_proxy *handle);
  void (*finished) (void *data, void *list);
} wayland_toplevel_list_listener = {
  wayland_toplevel_list_toplevel,
  wayland_toplevel_list_finished
};

static void
wayland_registry_global (void               *data,
                         struct wl_registry *registry,
                         uint32_t            name,
                         const char         *interface,
                         uint32_t            version)
{
  WaylandToplevelContext *context = data;

  if (g_strcmp0 (interface, "ext_foreign_toplevel_list_v1") == 0 &&
      context->toplevel_list == NULL)
    {
      context->toplevel_list = wl_registry_bind (registry,
                                                 name,
                                                 &ext_foreign_toplevel_list_v1_interface,
                                                 1);
      wl_proxy_add_listener (context->toplevel_list,
                             (void (**)(void)) &wayland_toplevel_list_listener,
                             context);
    }
}

static void
wayland_registry_global_remove (void               *data,
                                struct wl_registry *registry,
                                uint32_t            name)
{
}

static const struct wl_registry_listener wayland_registry_listener = {
  wayland_registry_global,
  wayland_registry_global_remove
};

static gboolean
wayland_toplevel_is_candidate (WaylandToplevel *toplevel)
{
  if (toplevel->closed || !toplevel->identifier || *toplevel->identifier == '\0')
    return FALSE;

  if (g_strcmp0 (toplevel->app_id, "mate-screenshot") == 0)
    return FALSE;

  return TRUE;
}

static gchar *
wayland_select_toplevel_identifier (GPtrArray *toplevels)
{
  GtkWidget *dialog;
  GtkWidget *content;
  GtkWidget *scrolled;
  GtkWidget *tree;
  GtkListStore *store;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gchar *identifier = NULL;
  guint candidates = 0;
  guint i;

  for (i = 0; i < toplevels->len; i++)
    {
      WaylandToplevel *toplevel = g_ptr_array_index (toplevels, i);

      if (!wayland_toplevel_is_candidate (toplevel))
        continue;

      candidates++;
      identifier = toplevel->identifier;
    }

  if (candidates == 0)
    return NULL;

  if (candidates == 1)
    return g_strdup (identifier);

  dialog = gtk_dialog_new_with_buttons (_("Select Window"),
                                        NULL,
                                        GTK_DIALOG_MODAL,
                                        _("_Cancel"),
                                        GTK_RESPONSE_CANCEL,
                                        _("_Capture"),
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  gtk_window_set_default_size (GTK_WINDOW (dialog), 480, 320);

  content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (content), scrolled, TRUE, TRUE, 0);

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

  for (i = 0; i < toplevels->len; i++)
    {
      WaylandToplevel *toplevel = g_ptr_array_index (toplevels, i);
      gchar *label;

      if (!wayland_toplevel_is_candidate (toplevel))
        continue;

      if (toplevel->title && toplevel->app_id)
        label = g_strdup_printf ("%s - %s", toplevel->title, toplevel->app_id);
      else
        label = g_strdup (toplevel->title ? toplevel->title : toplevel->app_id);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
                          0, label ? label : _("Untitled Window"),
                          1, toplevel->identifier,
                          -1);
      g_free (label);
    }

  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Window"),
                                                     renderer,
                                                     "text", 0,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
  gtk_container_add (GTK_CONTAINER (scrolled), tree);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
    gtk_tree_selection_select_iter (selection, &iter);

  gtk_widget_show_all (dialog);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT &&
      gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                          1, &identifier,
                          -1);
    }
  else
    {
      identifier = NULL;
    }

  gtk_widget_destroy (dialog);

  return identifier;
}

static gchar *
wayland_get_toplevel_identifier (void)
{
  GdkDisplay *gdk_display;
  struct wl_display *display;
  struct wl_registry *registry;
  WaylandToplevelContext context;
  gchar *identifier = NULL;

  gdk_display = gdk_display_get_default ();
  if (!GDK_IS_WAYLAND_DISPLAY (gdk_display))
    return NULL;

  display = gdk_wayland_display_get_wl_display (gdk_display);
  if (!display)
    return NULL;

  context.toplevel_list = NULL;
  context.toplevels = g_ptr_array_new_with_free_func ((GDestroyNotify) wayland_toplevel_free);

  registry = wl_display_get_registry (display);
  wl_registry_add_listener (registry, &wayland_registry_listener, &context);

  wl_display_roundtrip (display);
  if (context.toplevel_list)
    wl_display_roundtrip (display);

  if (context.toplevel_list)
    identifier = wayland_select_toplevel_identifier (context.toplevels);

  if (context.toplevel_list)
    wl_proxy_marshal_flags (context.toplevel_list,
                            1,
                            NULL,
                            wl_proxy_get_version (context.toplevel_list),
                            WL_MARSHAL_FLAG_DESTROY);

  wl_registry_destroy (registry);
  g_ptr_array_unref (context.toplevels);

  return identifier;
}

typedef struct {
  GMainLoop *loop;
  gchar *uri;
  gboolean timeout;
} PortalContext;

static GdkPixbuf *
screenshot_get_pixbuf_grim (GdkRectangle *rectangle)
{
  GSubprocess *process;
  GBytes *stdout_bytes = NULL;
  GBytes *stderr_bytes = NULL;
  GInputStream *stream;
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;
  gchar *geometry = NULL;
  const gchar *argv_full[] = { "grim", "-", NULL };
  const gchar *argv_area[] = { "grim", "-g", NULL, "-", NULL };
  const gchar **argv;

  if (rectangle)
    {
      geometry = g_strdup_printf ("%d,%d %dx%d",
                                  rectangle->x,
                                  rectangle->y,
                                  rectangle->width,
                                  rectangle->height);
      argv_area[2] = geometry;
      argv = argv_area;
    }
  else
    {
      argv = argv_full;
    }

  process = g_subprocess_newv (argv,
                               G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                               G_SUBPROCESS_FLAGS_STDERR_PIPE,
                               &error);
  if (!process)
    {
      g_warning ("Error launching grim fallback: %s", error->message);
      g_error_free (error);
      g_free (geometry);
      return NULL;
    }

  if (!g_subprocess_communicate (process, NULL, NULL,
                                 &stdout_bytes, &stderr_bytes, &error))
    {
      g_warning ("Error reading grim fallback output: %s", error->message);
      g_error_free (error);
      g_object_unref (process);
      g_free (geometry);
      return NULL;
    }

  if (!g_subprocess_get_successful (process))
    {
      const gchar *stderr_data = NULL;
      gsize stderr_size = 0;

      if (stderr_bytes)
        stderr_data = g_bytes_get_data (stderr_bytes, &stderr_size);

      if (stderr_data && stderr_size > 0)
        g_warning ("grim fallback failed: %.*s", (gint) stderr_size, stderr_data);
      else
        g_warning ("grim fallback failed");

      if (stdout_bytes)
        g_bytes_unref (stdout_bytes);
      if (stderr_bytes)
        g_bytes_unref (stderr_bytes);
      g_object_unref (process);
      g_free (geometry);
      return NULL;
    }

  stream = g_memory_input_stream_new_from_bytes (stdout_bytes);
  pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
  if (!pixbuf)
    {
      g_warning ("Error loading screenshot from grim fallback: %s", error->message);
      g_error_free (error);
    }

  g_object_unref (stream);
  g_bytes_unref (stdout_bytes);
  if (stderr_bytes)
    g_bytes_unref (stderr_bytes);
  g_object_unref (process);
  g_free (geometry);

  return pixbuf;
}

static GdkPixbuf *
screenshot_get_pixbuf_grim_toplevel (const gchar *identifier)
{
  GSubprocess *process;
  GBytes *stdout_bytes = NULL;
  GBytes *stderr_bytes = NULL;
  GInputStream *stream;
  GdkPixbuf *pixbuf = NULL;
  GError *error = NULL;
  const gchar *argv[] = { "grim", "-T", NULL, "-", NULL };

  if (!identifier || *identifier == '\0')
    return NULL;

  argv[2] = identifier;

  process = g_subprocess_newv (argv,
                               G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                               G_SUBPROCESS_FLAGS_STDERR_PIPE,
                               &error);
  if (!process)
    {
      g_warning ("Error launching grim toplevel fallback: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  if (!g_subprocess_communicate (process, NULL, NULL,
                                 &stdout_bytes, &stderr_bytes, &error))
    {
      g_warning ("Error reading grim toplevel fallback output: %s", error->message);
      g_error_free (error);
      g_object_unref (process);
      return NULL;
    }

  if (!g_subprocess_get_successful (process))
    {
      const gchar *stderr_data = NULL;
      gsize stderr_size = 0;

      if (stderr_bytes)
        stderr_data = g_bytes_get_data (stderr_bytes, &stderr_size);

      if (stderr_data && stderr_size > 0)
        g_warning ("grim toplevel fallback failed: %.*s", (gint) stderr_size, stderr_data);
      else
        g_warning ("grim toplevel fallback failed");

      if (stdout_bytes)
        g_bytes_unref (stdout_bytes);
      if (stderr_bytes)
        g_bytes_unref (stderr_bytes);
      g_object_unref (process);
      return NULL;
    }

  stream = g_memory_input_stream_new_from_bytes (stdout_bytes);
  pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
  if (!pixbuf)
    {
      g_warning ("Error loading screenshot from grim toplevel fallback: %s", error->message);
      g_error_free (error);
    }

  g_object_unref (stream);
  g_bytes_unref (stdout_bytes);
  if (stderr_bytes)
    g_bytes_unref (stderr_bytes);
  g_object_unref (process);

  return pixbuf;
}

static GdkPixbuf *
screenshot_get_pixbuf_wayland_toplevel (void)
{
  GdkPixbuf *pixbuf;
  gchar *identifier;

  identifier = wayland_get_toplevel_identifier ();
  if (!identifier)
    return NULL;

  pixbuf = screenshot_get_pixbuf_grim_toplevel (identifier);
  g_free (identifier);

  return pixbuf;
}

static void
on_portal_response (GDBusConnection *conn,
                    const gchar     *sender,
                    const gchar     *obj_path,
                    const gchar     *iface,
                    const gchar     *signal,
                    GVariant        *params,
                    gpointer         data)
{
  guint32 response;
  GVariant *results;
  const gchar *uri_val = NULL;
  PortalContext *context = data;

  g_variant_get (params, "(u@a{sv})", &response, &results);
  if (response == 0)
    {
      if (g_variant_lookup (results, "uri", "&s", &uri_val))
        {
          context->uri = g_strdup (uri_val);
        }
    }
  g_variant_unref (results);
  g_main_loop_quit (context->loop);
}

static gboolean
on_portal_timeout (gpointer data)
{
  PortalContext *context = data;
  context->timeout = TRUE;
  g_main_loop_quit (context->loop);
  return G_SOURCE_REMOVE;
}

static GdkPixbuf *
screenshot_get_pixbuf_portal (gboolean interactive)
{
  GDBusConnection *connection;
  GError *error = NULL;
  GVariant *ret;
  const gchar *handle;
  GdkPixbuf *pixbuf = NULL;
  PortalContext context;
  gchar *handle_token;
  gchar *sender;
  gchar *request_path;
  guint sub_id;
  guint timeout_id;
  GVariantBuilder options_builder;
  gint i;

  connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  if (!connection)
    {
      if (interactive)
        g_warning ("Error connecting to D-Bus: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  sender = g_strdup (g_dbus_connection_get_unique_name (connection));
  for (i = 0; sender[i]; i++)
    if (sender[i] == '.')
      sender[i] = '_';
  if (sender[0] == ':') sender[0] = '_';

  handle_token = g_strdup_printf ("mate_screenshot_%u", g_random_int ());
  request_path = g_strdup_printf ("/org/freedesktop/portal/desktop/request/%s/%s", sender, handle_token);

  context.loop = g_main_loop_new (NULL, FALSE);
  context.uri = NULL;
  context.timeout = FALSE;

  sub_id = g_dbus_connection_signal_subscribe (connection,
                                               "org.freedesktop.portal.Desktop",
                                               "org.freedesktop.portal.Request",
                                               "Response",
                                               request_path,
                                               NULL,
                                               G_DBUS_SIGNAL_FLAGS_NONE,
                                               on_portal_response,
                                               &context,
                                               NULL);

  g_variant_builder_init (&options_builder, G_VARIANT_TYPE_VARDICT);
  g_variant_builder_add (&options_builder, "{sv}", "handle_token", g_variant_new_string (handle_token));
  g_variant_builder_add (&options_builder, "{sv}", "interactive", g_variant_new_boolean (interactive));

  ret = g_dbus_connection_call_sync (connection,
                                     "org.freedesktop.portal.Desktop",
                                     "/org/freedesktop/portal/desktop",
                                     "org.freedesktop.portal.Screenshot",
                                     "Screenshot",
                                     g_variant_new ("(s@a{sv})", "", g_variant_builder_end (&options_builder)),
                                     G_VARIANT_TYPE ("(o)"),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     -1,
                                     NULL,
                                     &error);

  if (!ret)
    {
      if (interactive)
        g_warning ("Error calling Screenshot portal: %s", error->message);
      g_clear_error (&error);
      g_dbus_connection_signal_unsubscribe (connection, sub_id);
      g_main_loop_unref (context.loop);
      g_object_unref (connection);
      g_free (handle_token);
      g_free (sender);
      g_free (request_path);
      return NULL;
    }

  g_variant_get (ret, "(&o)", &handle);
  if (g_strcmp0 (handle, request_path) != 0)
    {
      g_warning ("Portal returned unexpected handle: %s (expected %s)", handle, request_path);
    }

  timeout_id = g_timeout_add_seconds (60, on_portal_timeout, &context);
  g_main_loop_run (context.loop);

  if (!context.timeout)
    g_source_remove (timeout_id);

  g_dbus_connection_signal_unsubscribe (connection, sub_id);

  if (context.timeout)
    {
      g_warning ("Portal interaction timed out");
    }
  else if (context.uri)
    {
      GFile *file = g_file_new_for_uri (context.uri);
      g_clear_error (&error);
      GInputStream *stream = G_INPUT_STREAM (g_file_read (file, NULL, &error));
      if (stream)
        {
          pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
          if (error)
            {
              g_warning ("Error loading screenshot from portal: %s", error->message);
              g_clear_error (&error);
            }
          g_object_unref (stream);
        }
      else
        {
          g_warning ("Error opening screenshot file: %s", error->message);
          g_clear_error (&error);
        }
      g_object_unref (file);
      g_free (context.uri);
    }

  g_main_loop_unref (context.loop);
  g_variant_unref (ret);
  g_object_unref (connection);
  g_free (handle_token);
  g_free (sender);
  g_free (request_path);

  return pixbuf;
}
#endif

GdkPixbuf *
screenshot_get_pixbuf (GdkWindow    *window,
                       GdkRectangle *rectangle,
                       gboolean      include_pointer,
                       gboolean      include_border,
                       gboolean      include_mask)
{
  GdkWindow *root;
  GdkPixbuf *screenshot;
  gint x_real_orig, y_real_orig, x_orig, y_orig;
  gint width, real_width, height, real_height;
  gint screen_width, screen_height, scale;
  gint invis_x = 0, invis_y = 0;

#ifdef MATE_SCREENSHOT_ENABLE_WAYLAND
  if (is_wayland ())
    {
      gboolean interactive = !include_mask;
      GdkPixbuf *pixbuf;

      if (rectangle)
        {
          pixbuf = screenshot_get_pixbuf_grim (rectangle);
          if (pixbuf)
            return pixbuf;
        }

      /* Window capture on Wayland needs compositor or portal support.  A raw
       * slurp selection without predefined window boxes is area capture, not
       * window capture, so keep the window mode on the portal path. */
      if (interactive)
        {
          pixbuf = screenshot_get_pixbuf_wayland_toplevel ();
          if (pixbuf)
            return pixbuf;

          return screenshot_get_pixbuf_portal (TRUE);
        }

      if (!interactive)
        {
          pixbuf = screenshot_get_pixbuf_grim (NULL);
          if (pixbuf)
            return pixbuf;
        }

      return screenshot_get_pixbuf_portal (interactive);
    }
#endif

#ifdef GDK_WINDOWING_X11
  /* If the screenshot should include the border, we look for the WM window. */

  Window client_xid = None;

  if (include_border)
    {
      Window xid, wm;

      xid = GDK_WINDOW_XID (window);
      client_xid = xid;
      wm = find_wm_window (xid);

      if (wm != None)
        window = gdk_x11_window_foreign_new_for_display (gdk_display_get_default (), wm);

      /* fallback to no border if we can't find the WM window. */
    }

  root = gdk_get_default_root_window ();
  scale = gdk_window_get_scale_factor (root);

  real_width = gdk_window_get_width (window);
  real_height = gdk_window_get_height (window);

  screen_width = WidthOfScreen (gdk_x11_screen_get_xscreen (gdk_screen_get_default ())) / scale;
  screen_height = HeightOfScreen (gdk_x11_screen_get_xscreen (gdk_screen_get_default ())) / scale;

  gdk_window_get_origin (window, &x_real_orig, &y_real_orig);

  x_orig = x_real_orig;
  y_orig = y_real_orig;
  width  = real_width;
  height = real_height;

  if (include_border && client_xid != None && width > 0 && height > 0)
    {
      Display *xdpy = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
      Atom frame_extents = XInternAtom (xdpy, "_NET_FRAME_EXTENTS", False);
      Atom type;
      int fmt;
      unsigned long nitems, bytes;
      unsigned char *data = NULL;

      if (XGetWindowProperty (xdpy, client_xid, frame_extents, 0, 4, False,
                              XA_CARDINAL, &type, &fmt, &nitems, &bytes,
                              &data) == Success && data && nitems >= 4)
        {
          unsigned long *ext = (unsigned long *) data;
          XWindowAttributes wa;

          if (XGetWindowAttributes (xdpy, client_xid, &wa))
            {
              int vis_w = (int)(wa.width  + ext[0] + ext[1]) / scale;
              int vis_h = (int)(wa.height + ext[2] + ext[3]) / scale;
              int border_x = (wa.x - (int) ext[0]) / scale;
              int border_y = (wa.y - (int) ext[2]) / scale;

              if (border_x > 0 && vis_w < real_width)
                {
                  invis_x = (wa.x - (int) ext[0]);
                  x_orig += border_x;
                  width  = vis_w;
                }
              if (border_y > 0 && vis_h < real_height)
                {
                  invis_y = (wa.y - (int) ext[2]);
                  y_orig += border_y;
                  height = vis_h;
                }
            }
        }
      if (data)
        XFree (data);
    }

  if (x_orig < 0)
    {
      width = width + x_orig;
      x_orig = 0;
    }

  if (y_orig < 0)
    {
      height = height + y_orig;
      y_orig = 0;
    }

  if (x_orig + width > screen_width)
    width = screen_width - x_orig;

  if (y_orig + height > screen_height)
    height = screen_height - y_orig;

  if (rectangle)
    {
      x_orig = rectangle->x - x_orig;
      y_orig = rectangle->y - y_orig;
      width  = rectangle->width;
      height = rectangle->height;
    }

  screenshot = gdk_pixbuf_get_from_window (root,
                                           x_orig, y_orig,
                                           width, height);

  /*
   * Masking currently only works properly with full-screen shots
   */
  if (include_mask)
      mask_monitors (screenshot, root);

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
  if (include_border)
    {
      XRectangle *rectangles;
      GdkPixbuf *tmp;
      int rectangle_count, rectangle_order, i;

      /* we must use XShape to avoid showing what's under the rounder corners
       * of the WM decoration.
       */

      rectangles = XShapeGetRectangles (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                                        GDK_WINDOW_XID (window),
                                        ShapeBounding,
                                        &rectangle_count,
                                        &rectangle_order);
      if (rectangles && rectangle_count > 0 && window != root)
        {
          gboolean has_alpha = gdk_pixbuf_get_has_alpha (screenshot);

          if (scale)
            {
              width *= scale;
              height *= scale;
            }

          tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
          gdk_pixbuf_fill (tmp, 0);

          for (i = 0; i < rectangle_count; i++)
            {
              gint rec_x, rec_y;
              gint rec_width, rec_height;
              gint y;

              rec_x = rectangles[i].x - invis_x;
              rec_y = rectangles[i].y - invis_y;
              rec_width = rectangles[i].width / scale;
              rec_height = rectangles[i].height / scale;

              if (rec_x < 0)
                {
                  rec_width += rec_x;
                  rec_x = 0;
                }
              if (rec_y < 0)
                {
                  rec_height += rec_y;
                  rec_y = 0;
                }
              if (rec_x + rec_width > width)
                rec_width = width - rec_x;
              if (rec_y + rec_height > height)
                rec_height = height - rec_y;
              if (rec_width <= 0 || rec_height <= 0)
                continue;

              if (x_real_orig < 0)
                {
                  rec_x += x_real_orig;
                  rec_x = MAX(rec_x, 0);
                  rec_width += x_real_orig;
                }

              if (y_real_orig < 0)
                {
                  rec_y += y_real_orig;
                  rec_y = MAX(rec_y, 0);
                  rec_height += y_real_orig;
                }

              if (x_orig + rec_x + rec_width > screen_width)
                rec_width = screen_width - x_orig - rec_x;

              if (y_orig + rec_y + rec_height > screen_height)
                rec_height = screen_height - y_orig - rec_y;

              if (scale)
                {
                  rec_width *= scale;
                  rec_height *= scale;
                }

              for (y = rec_y; y < rec_y + rec_height; y++)
                {
                  guchar *src_pixels, *dest_pixels;
                  gint x;

                  src_pixels = gdk_pixbuf_get_pixels (screenshot)
                             + y * gdk_pixbuf_get_rowstride (screenshot)
                             + rec_x * (has_alpha ? 4 : 3);
                  dest_pixels = gdk_pixbuf_get_pixels (tmp)
                              + y * gdk_pixbuf_get_rowstride (tmp)
                              + rec_x * 4;

                  for (x = 0; x < rec_width; x++)
                    {
                      *dest_pixels++ = *src_pixels++;
                      *dest_pixels++ = *src_pixels++;
                      *dest_pixels++ = *src_pixels++;

                      if (has_alpha)
                        *dest_pixels++ = *src_pixels++;
                      else
                        *dest_pixels++ = 255;
                    }
                }
            }

          g_object_unref (screenshot);
          screenshot = tmp;
        }
    }
#endif /* HAVE_X11_EXTENSIONS_SHAPE_H */

  /* if we have a selected area, there were by definition no cursor in the
   * screenshot */
  if (include_pointer && !rectangle)
    {
      GdkCursor *cursor;
      GdkPixbuf *cursor_pixbuf;

      cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_LEFT_PTR);
      cursor_pixbuf = gdk_cursor_get_image (cursor);

      if (cursor_pixbuf != NULL)
        {
          GdkDisplay *display;
          GdkSeat *seat;
          GdkDevice *device;
          GdkRectangle r1, r2;
          gint cx, cy, xhot, yhot;

          display = gdk_window_get_display (window);
          seat = gdk_display_get_default_seat (display);
          device = gdk_seat_get_pointer (seat);

          gdk_window_get_device_position (window, device, &cx, &cy, NULL);
          sscanf (gdk_pixbuf_get_option (cursor_pixbuf, "x_hot"), "%d", &xhot);
          sscanf (gdk_pixbuf_get_option (cursor_pixbuf, "y_hot"), "%d", &yhot);

          if (scale)
            {
              cx *= scale;
              cy *= scale;
            }

          /* in r1 we have the window coordinates */
          r1.x = x_real_orig;
          r1.y = y_real_orig;
          r1.width = real_width * scale;
          r1.height = real_height * scale;

          /* in r2 we have the cursor window coordinates */
          r2.x = cx + x_real_orig;
          r2.y = cy + y_real_orig;
          r2.width = gdk_pixbuf_get_width (cursor_pixbuf);
          r2.height = gdk_pixbuf_get_height (cursor_pixbuf);

          /* see if the pointer is inside the window */
          if (gdk_rectangle_intersect (&r1, &r2, &r2))
            {
              gdk_pixbuf_composite (cursor_pixbuf, screenshot,
                                    cx - xhot, cy - yhot,
                                    r2.width, r2.height,
                                    cx - xhot, cy - yhot,
                                    1.0, 1.0,
                                    GDK_INTERP_BILINEAR,
                                    255);
            }

          g_object_unref (cursor_pixbuf);
          g_object_unref (cursor);
        }
    }

  return screenshot;
#else
  return NULL;
#endif
}

void
screenshot_show_error_dialog (GtkWindow   *parent,
                              const gchar *message,
                              const gchar *detail)
{
  GtkWidget *dialog;

  g_return_if_fail ((parent == NULL) || (GTK_IS_WINDOW (parent)));
  g_return_if_fail (message != NULL);

  dialog = gtk_message_dialog_new (parent,
  				   GTK_DIALOG_DESTROY_WITH_PARENT,
  				   GTK_MESSAGE_ERROR,
  				   GTK_BUTTONS_OK,
  				   "%s", message);
  gtk_window_set_title (GTK_WINDOW (dialog), "");

  if (detail)
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
  					      "%s", detail);

  if (parent && gtk_window_get_group (parent))
    gtk_window_group_add_window (gtk_window_get_group (parent), GTK_WINDOW (dialog));

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
}

void
screenshot_show_gerror_dialog (GtkWindow   *parent,
                               const gchar *message,
                               GError      *error)
{
  g_return_if_fail (parent == NULL || GTK_IS_WINDOW (parent));
  g_return_if_fail (message != NULL);
  g_return_if_fail (error != NULL);

  screenshot_show_error_dialog (parent, message, error->message);
}
