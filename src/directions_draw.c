#include "directions_draw.h"


// Draw the more indicator and return the new bounds (excluding the space occupied by the more indicator)
GRect directions_draw_more_indicator(GContext *ctx, GRect bounds) {
  // Set the colors
  GColor color_bg;
  GColor color_arrow = GColorBlack;
  #ifdef PBL_COLOR
    color_bg = GColorLightGray;
  #else
    color_bg = GColorWhite;
  #endif

  // Draw the background
  graphics_context_set_fill_color(ctx, color_bg);
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - 17, bounds.size.w, 25), 0, GCornerNone);
  // Draw the arrow (draws a path)
  struct GPathInfo path_arrow_info = {
    .num_points = 3,
    .points = (GPoint []) { {0,0}, {6,5}, {12,0} },
  };
  GPath *path_arrow = gpath_create(&path_arrow_info);
  gpath_move_to(path_arrow, (GPoint) {(bounds.size.w - 12) / 2, bounds.size.h - 10});
  graphics_context_set_fill_color(ctx, color_arrow);
  gpath_draw_filled(ctx, path_arrow);
  gpath_destroy(path_arrow);

  // Return the new bounds
  bounds.size.h -= 15;
  return bounds;
}

// Work out the height of each cell
int16_t directions_get_cell_height(int16_t window_height, int16_t cell_index) {
  // Each cell occupies an entire screen!
  return window_height;
}

// Draw the title card
void directions_draw_summary(GContext *ctx, GRect bounds, GColor color, int distance, int time, char *string_address) {
  // Get the strings
  char string_distance[16];
  char string_time[16];
  // Disance string
  switch (health_service_get_measurement_system_for_display(HealthMetricWalkedDistanceMeters)) {
    // Imperial (miles)
    case MeasurementSystemImperial:
      snprintf(string_distance, sizeof(string_distance), "%i.%i mi", distance / 1609, ((distance % 1609) * 10) / 1609);
      break;
    // Metric (meters & kilometers)
    default:
      if (distance < 1000) {
        // Use meters
        snprintf(string_distance, sizeof(string_distance), "%i m", distance);
      } else {
        // Convert to kilometeres with one place after the point
        snprintf(string_distance, sizeof(string_distance), "%i.%i km", distance / 1000, (distance % 1000) / 100);
      }
  }
  // Time required string
  if (time < 60) {
    // Use minutes
    snprintf(string_time, sizeof(string_time), "%i min", time);
  } else {
    // Convert to hours / minutes in the form 1 h 8 min
    snprintf(string_time, sizeof(string_time), "%i h %i min", time / 60, time % 60);
  }

  // Get the colors
  #ifdef PBL_COLOR
    GColor color_time = GColorBlack;
  #else
    GColor color_time = GColorWhite;
    color = GColorBlack;
  #endif

  // White background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the more indicator and get the new bounds size
  bounds = directions_draw_more_indicator(ctx, bounds);

  // Draw the UI
  #ifdef PBL_ROUND
    // ************
    // * Round UI *
    // ************
    // Header bg (takes 1/2 of screen height)
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_circle(ctx, (GPoint) {bounds.size.w / 2, bounds.size.h / (-2)}, bounds.size.h);
    // Time
    GRect time_box = GRect(0, 23, bounds.size.w, 28);
    graphics_context_set_text_color(ctx, color_time);
    graphics_draw_text(ctx, string_time, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), time_box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Distance
    GRect distance_box = GRect(0, 23 + time_box.size.h, bounds.size.w, 18);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, string_distance, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), distance_box, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    // Address
    GRect address_box = GRect(0, 7 + bounds.size.h / 2, bounds.size.w, bounds.size.h / 2 - 14);
    GTextAttributes *text_attributes = graphics_text_attributes_create();
    graphics_text_attributes_enable_screen_text_flow(text_attributes, 7);
    graphics_text_attributes_enable_paging(text_attributes, address_box.origin, address_box);
    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, string_address, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), address_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, text_attributes);
    graphics_text_attributes_destroy(text_attributes);
  #else
    // *************
    // * Square UI *
    // *************
    // Header bg
    GRect header_box = GRect(0, 0, bounds.size.w, 60 + STATUS_BAR_LAYER_HEIGHT);
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, header_box, 0, GCornerNone);
    // Time
    GRect time_box = GRect(7, STATUS_BAR_LAYER_HEIGHT, bounds.size.w - 14, 28);
    graphics_context_set_text_color(ctx, color_time);
    graphics_draw_text(ctx, string_time, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), time_box, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    // Distance
    GRect distance_box = GRect(7, STATUS_BAR_LAYER_HEIGHT + time_box.size.h, bounds.size.w - 14, 18);
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, string_distance, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), distance_box, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    // Address
    GRect address_box = GRect(7, 7 + header_box.size.h, bounds.size.w - 14, bounds.size.h - header_box.size.h - 14);
    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, string_address, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), address_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  #endif
}

// Draw a step
void directions_draw_step(GContext *ctx, GRect bounds, GColor color, char *text, GBitmap *step_icon, int16_t data_index, int16_t data_count) {
  // Get the colors
  #ifdef PBL_BW
    color = GColorWhite;
  #endif
  GColor color_text = GColorBlack;

  // White background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the more indicator and get the new bounds size (if this is not the last cell)
  if (data_index + 1 < data_count) {
    bounds = directions_draw_more_indicator(ctx, bounds);
  }

  // Draw the UI
  #ifdef PBL_ROUND
    // ************
    // * Round UI *
    // ************
    // Draw header / icon bg (takes 60px of screen height)
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, -80), 140);
    // Draw icon
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, step_icon, GRect((bounds.size.w - 24) / 2, 28, 24, 24));
    // Draw text
    GRect text_box = GRect(0, 67, bounds.size.w, bounds.size.h - 74);
    GTextAttributes *text_attributes = graphics_text_attributes_create();
    graphics_text_attributes_enable_screen_text_flow(text_attributes, 7);
    graphics_text_attributes_enable_paging(text_attributes, text_box.origin, text_box);
    graphics_context_set_text_color(ctx, color_text);
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), text_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, text_attributes);
    graphics_text_attributes_destroy(text_attributes);
  #else
    // *************
    // * Square UI *
    // *************
    // Draw header/icon bg
    GRect header_box = GRect(0, 0, bounds.size.w, 52);
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, header_box, 0, GCornerNone);
    // Draw icon
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, step_icon, GRect((bounds.size.w - 24) / 2, 20, 24, 24));
    // Draw text
    GRect text_box = GRect(7, 7 + header_box.size.h, bounds.size.w - 14, bounds.size.h - header_box.size.h - 14);
    graphics_context_set_text_color(ctx, color_text);
    graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), text_box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  #endif
}
