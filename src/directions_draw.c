#include "directions_draw.h"


#define SQUARE_CELL_HEIGHT_SUMMARY 64
#define SQUARE_CELL_HEIGHT_STEP 122

// Work out the height of each cell
int16_t directions_get_cell_height(int16_t window_height, int16_t cell_index) {
  // Round watches paginate!
  #ifdef PBL_ROUND
    return window_height;
  #endif
  // Square watches scroll!
  switch (cell_index) {
    case 0:
      return SQUARE_CELL_HEIGHT_SUMMARY;
      break;
    default:
      return SQUARE_CELL_HEIGHT_STEP;
  }
}

// Draw the title card
void directions_draw_summary(GContext *ctx, GRect bounds, GColor color, int distance, int time) {
  // Get the strings
  char string_distance[10];
  char string_time[10];
  snprintf(string_distance, sizeof(string_distance), "%i m", distance);
  snprintf(string_time, sizeof(string_time), "%i min", time);
  // Get the colors
  #ifdef PBL_COLOR
    GColor color_secondary = GColorDarkGray;
  #else
    GColor color_secondary = GColorWhite;
    color = GColorBlack;
  #endif
  // Draw the background
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  #ifdef PBL_ROUND
    // ************
    // * Round UI *
    // ************
    // Outside ring
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 30, 0, TRIG_MAX_ANGLE);
    // Time
    GRect time_box = GRect(4, (bounds.size.h - 46) / 2, bounds.size.w - 8, 28);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, string_time, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), time_box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Distance
    GRect distance_box = GRect(4, (bounds.size.h - 46) / 2 + time_box.size.h, bounds.size.w - 8, 18);
    graphics_context_set_text_color(ctx, color_secondary);
    graphics_draw_text(ctx, string_distance, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), distance_box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  #else
    // *************
    // * Square UI *
    // *************
    // Time
    GRect time_box = GRect(7, 5, bounds.size.w - 14, 28);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, string_time, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), time_box, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    // Distance
    GRect distance_box = GRect(7, time_box.size.h + 5, bounds.size.w - 14, 18);
    graphics_context_set_text_color(ctx, color_secondary);
    graphics_draw_text(ctx, string_distance, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), distance_box, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  #endif
}

// Draw a step
void directions_draw_step(GContext *ctx, GRect bounds, GColor color, char *text, int16_t data_index, int16_t data_count) {
  // Get the colors
  #ifdef PBL_COLOR
    GColor color_text = GColorDarkGray;
  #else
    GColor color_text = GColorBlack;
    color = GColorBlack;
  #endif
  // Draw the background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  #ifdef PBL_ROUND
    // ************
    // * Round UI *
    // ************
    // Draw text
    GRect text_box = GRect(5, (bounds.size.w - 96) / 2, bounds.size.w - 10, 96);
    GTextAttributes *text_attributes = graphics_text_attributes_create();
    graphics_text_attributes_enable_screen_text_flow(text_attributes, 5);
    graphics_context_set_text_color(ctx, color_text);
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), text_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, text_attributes);
    graphics_text_attributes_destroy(text_attributes);
    // Draw more indicators
    graphics_context_set_fill_color(ctx, color);
    if (data_index < data_count - 1) {
      // Bottom
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h), 8);
    }
    if (data_index > 0) {
      // Top
      graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, 0), 8);
    }
  #else
    // *************
    // * Square UI *
    // *************
    // Draw text
    GRect text_box = GRect(7, 3, bounds.size.w - 14, bounds.size.h - 6);
    graphics_context_set_text_color(ctx, color_text);
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), text_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    // Draw block-end indicator (if not last block)
    if (data_index < data_count - 1) {
      GRect line_box = GRect(7, text_box.size.h + 1, 40, 4);
      graphics_context_set_fill_color(ctx, color);
      graphics_fill_rect(ctx, line_box, 2, GCornersAll);
    }
  #endif
}
