#define _POSIX_C_SOURCE 200809L
#include <cairo/cairo.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"

#include "menu.h"
#include "pango.h"
#include "pool-buffer.h"
#include "wayland.h"

// Calculate text widths.
void
calc_widths (struct menu *menu)
{
  struct wl_context *context = menu->context;
  int scale = context_get_scale (context);
  cairo_surface_set_device_scale (menu->test_surface, scale, scale);
  cairo_set_antialias (menu->test_cairo, CAIRO_ANTIALIAS_BEST);
  cairo_font_options_t *fo = cairo_font_options_create ();
  cairo_set_font_options (menu->test_cairo, fo);
  cairo_font_options_destroy (fo);
  cairo_t *cairo = menu->test_cairo;

  // Calculate prompt width
  if (menu->prompt)
    {
      menu->promptw = text_width (cairo, menu->font, menu->prompt)
                      + menu->padding + menu->padding / 2;
    }
  else
    {
      menu->promptw = 0;
    }

  // Calculate scroll indicator widths
  menu->left_arrow = text_width (cairo, menu->font, "<") + 2 * menu->padding;
  menu->right_arrow = text_width (cairo, menu->font, ">") + 2 * menu->padding;

  // Calculate item widths and input area width
  for (size_t i = 0; i < menu->item_count; i++)
    {
      struct item *item = &menu->items[i];
      item->width = text_width (cairo, menu->font, item->text);
      if (item->width > menu->inputw)
        {
          menu->inputw = item->width;
        }
    }
}

static void
cairo_set_source_u32 (cairo_t *cairo, uint32_t color)
{
  cairo_set_source_rgba (cairo, (color >> (3 * 8) & 0xFF) / 255.0,
                         (color >> (2 * 8) & 0xFF) / 255.0,
                         (color >> (1 * 8) & 0xFF) / 255.0,
                         (color >> (0 * 8) & 0xFF) / 255.0);
}

// Renders text to cairo.
static int
render_text (struct menu *menu, cairo_t *cairo, const char *str, int x, int y,
             int width, uint32_t bg_color, uint32_t fg_color, int left_padding,
             int right_padding)
{

  int text_width, text_height;
  get_text_size (cairo, menu->font, &text_width, &text_height, NULL, 1, str);
  int text_y = (menu->line_height / 2.0) - (text_height / 2.0);

  if (width == 0)
    {
      width = text_width + left_padding + right_padding;
    }
  if (bg_color)
    {
      cairo_set_source_u32 (cairo, bg_color);
      cairo_rectangle (cairo, x, y, width, menu->line_height);
      cairo_fill (cairo);
    }
  cairo_move_to (cairo, x + left_padding, y + text_y);
  cairo_set_source_u32 (cairo, fg_color);
  pango_printf (cairo, menu->font, 1, str);

  return width;
}

// Renders the prompt message.
static void
render_prompt (struct menu *menu, cairo_t *cairo)
{
  if (!menu->prompt)
    {
      return;
    }
  render_text (menu, cairo, menu->prompt, 0, 0, 0,
               menu->position == POSITION_CENTER ? 0x00000000 : menu->promptbg,
               menu->promptfg, menu->padding, menu->padding / 2);
}

// Renders the input text.
static void
render_input (struct menu *menu, cairo_t *cairo)
{
  char *censort = NULL;

  if (menu->passwd)
    {
      censort = calloc (1, sizeof (menu->input));
      if (!censort)
        {
          return;
        }
      memset (censort, '*', strlen (menu->input));
    }

  render_text (menu, cairo, menu->passwd ? censort : menu->input,
               menu->promptw, 0, 0, 0, menu->normalfg, menu->padding,
               menu->padding);

  if (censort)
    {
      free (censort);
    }
}

// Renders a cursor for the input field.
static void
render_cursor (struct menu *menu, cairo_t *cairo)
{
  const int cursor_width = 2;
  const int cursor_margin = 2;
  int cursor_pos = menu->promptw + menu->padding
                   + text_width (cairo, menu->font, menu->input)
                   - text_width (cairo, menu->font, &menu->input[menu->cursor])
                   - cursor_width / 2;
  cairo_rectangle (cairo, cursor_pos, cursor_margin, cursor_width,
                   menu->line_height - 2 * cursor_margin);
  cairo_fill (cairo);
}

// Renders a single menu item horizontally.
static int
render_horizontal_item (struct menu *menu, cairo_t *cairo, struct item *item,
                        int x)
{
  uint32_t bg_color = menu->sel == item ? menu->selectionbg
                                        : (menu->position == POSITION_CENTER
                                               ? 0x00000000
                                               : menu->normalbg);
  uint32_t fg_color = menu->sel == item ? menu->selectionfg : menu->normalfg;

  return render_text (menu, cairo, item->text, x, 0, 0, bg_color, fg_color,
                      menu->padding, menu->padding);
}

// Renders a single menu item vertically.
static int
render_vertical_item (struct menu *menu, cairo_t *cairo, struct item *item,
                      int x, int y)
{
  uint32_t fg_color = menu->sel == item ? menu->selectionfg : menu->normalfg;

  // Solid highlight block spanning the whole box width (pinentry style).
  if (menu->sel == item)
    {
      cairo_set_source_u32 (cairo, menu->selectionbg);
      cairo_rectangle (cairo, 0, y, menu->width,
                       (item->thumb_path ? 160 : menu->line_height));
      cairo_fill (cairo);
    }

  int thumb_size = 0;
  if (item->thumb_path)
    {
      thumb_size = 128;
      cairo_surface_t *img
          = cairo_image_surface_create_from_png (item->thumb_path);
      if (cairo_surface_status (img) == CAIRO_STATUS_SUCCESS)
        {
          int iw = cairo_image_surface_get_width (img);
          int ih = cairo_image_surface_get_height (img);
          double scale = (double)thumb_size / (iw > ih ? iw : ih);
          /* center on the ACTUAL scaled height: for wide (16:9) thumbs the
           * drawn height is ~72px, not thumb_size (128), so a constant
           * offset would leave them visibly high in the 160px row */
          int draw_h = (int)(ih * scale + 0.5);
          cairo_save (cairo);
          cairo_translate (cairo, x + menu->padding,
                           y + (160 - draw_h) / 2);
          cairo_scale (cairo, scale, scale);
          cairo_set_source_surface (cairo, img, 0, 0);
          cairo_paint (cairo);
          cairo_restore (cairo);
        }
      cairo_surface_destroy (img);
    }

  int text_offset = (item->thumb_path ? (160 - menu->line_height) / 2 : 0);
  render_text (menu, cairo, item->text, x + thumb_size + menu->padding,
               y + text_offset,
               menu->width - x - thumb_size - menu->padding, 0, fg_color,
               menu->padding, 0);
  return item->thumb_path ? 160 : menu->line_height;
}

// Renders a page of menu items horizontally.
static void
render_horizontal_page (struct menu *menu, cairo_t *cairo, struct page *page)
{
  int x = menu->promptw + menu->inputw + menu->left_arrow;
  for (struct item *item = page->first; item != page->last->next_match;
       item = item->next_match)
    {
      x += render_horizontal_item (menu, cairo, item, x);
    }

  // Draw left and right scroll indicators if necessary
  if (page->prev)
    {
      cairo_move_to (cairo, menu->promptw + menu->inputw + menu->padding, 0);
      pango_printf (cairo, menu->font, 1, "<");
    }
  if (page->next)
    {
      cairo_move_to (cairo, menu->width - menu->right_arrow + menu->padding,
                     0);
      pango_printf (cairo, menu->font, 1, ">");
    }
}

// Renders a page of menu items vertically.
static void
render_vertical_page (struct menu *menu, cairo_t *cairo, struct page *page)
{
  int x = menu->promptw;
  int y = menu->line_height;
  for (struct item *item = page->first; item != page->last->next_match;
       item = item->next_match)
    {
      y += render_vertical_item (menu, cairo, item, x, y);
    }
}

// Renders the menu to cairo.
static void
render_to_cairo (struct menu *menu, cairo_t *cairo)
{
  // Clear the reused ARGB32 buffer first: repainting OVER the previous
  // frame without clearing compounds the panel alpha toward opaque on
  // every keypress.
  cairo_set_operator (cairo, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cairo);

  // Opaque-or-alpha solid panel; alpha comes from -N (RRGGBBAA).
  cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba (cairo, (menu->normalbg >> 24 & 0xFF) / 255.0,
                         (menu->normalbg >> 16 & 0xFF) / 255.0,
                         (menu->normalbg >> 8 & 0xFF) / 255.0,
                         (menu->normalbg & 0xFF) / 255.0);
  cairo_paint (cairo);

  // Render prompt and input
  render_prompt (menu, cairo);
  render_input (menu, cairo);
  render_cursor (menu, cairo);

  // Render selected page
  if (menu->sel)
    {
      if (menu->lines > 0)
        {
          render_vertical_page (menu, cairo, menu->sel->page);
        }
      else
        {
          render_horizontal_page (menu, cairo, menu->sel->page);
        }
    }

  // 2px border frame around the whole box, drawn on top.
  if (menu->position == POSITION_CENTER)
    {
      cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);
      double bw = 2.0;
      cairo_set_line_width (cairo, bw);
      cairo_set_source_u32 (cairo, menu->border);
      cairo_rectangle (cairo, bw / 2, bw / 2, menu->width - bw,
                       menu->height - bw);
      cairo_stroke (cairo);
    }
}

// Renders a single frame of the menu.
void
render_menu (struct menu *menu)
{
  struct wl_context *context = menu->context;

  int scale = context_get_scale (context);
  struct pool_buffer *buffer = context_get_next_buffer (context, scale);
  if (!buffer)
    {
      return;
    }

  cairo_t *shm = buffer->cairo;
  cairo_set_antialias (shm, CAIRO_ANTIALIAS_BEST);
  cairo_font_options_t *fo = cairo_font_options_create ();
  cairo_set_font_options (shm, fo);
  cairo_font_options_destroy (fo);
  render_to_cairo (menu, shm);

  struct wl_surface *surface = context_get_surface (context);
  wl_surface_set_buffer_scale (surface, scale);
  wl_surface_attach (surface, buffer->buffer, 0, 0);
  wl_surface_damage (surface, 0, 0, menu->width, menu->height);
  wl_surface_commit (surface);

  menu->rendered = true;
}
