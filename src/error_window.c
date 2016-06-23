#include "error_window.h"


// The main window
static Window *window;
static BitmapLayer *error_icon_layer;
static TextLayer *error_text_layer;

// The error to be displayed
int display_error_type;

// Error icon / text
static GBitmap *icon_error;
static char *text_error;


static void window_unload() {
  // Destroy the text layer & the error icon layer
  bitmap_layer_destroy(error_icon_layer);
  text_layer_destroy(error_text_layer);

  // Destroy the image resource
  gbitmap_destroy(icon_error);

  // Destroy the window
  window_destroy(window);
  window = NULL;
}

static void window_load() {
  // Get the root layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  switch(display_error_type) {
    // Network
    case 0:
      icon_error = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR_NETWORK);
      text_error = "Could not reach phone";
      break;
    // Api
    case 1:
      icon_error = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR_API);
      text_error = "No route could be found";
      break;
    // Other / Undefined
    case 2:
      icon_error = gbitmap_create_with_resource(RESOURCE_ID_ICON_ERROR_OTHER);
      text_error = "Something went wrong";
      break;
  }

  // Set up the error image layer GRect(x, y, w, h)
  #ifdef PBL_ROUND
    // Round spacing / Align
    error_icon_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, 110));
    bitmap_layer_set_alignment(error_icon_layer, GAlignBottom);
  #else
    // Square spacing / Align
    error_icon_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, 100));
    bitmap_layer_set_alignment(error_icon_layer, GAlignCenter);
  #endif
  bitmap_layer_set_bitmap(error_icon_layer, icon_error);
  bitmap_layer_set_compositing_mode(error_icon_layer, GCompOpSet);
  #ifdef PBL_COLOR
    // Color only
    bitmap_layer_set_background_color(error_icon_layer, COLOR_ERROR);
  #endif

  layer_add_child(window_layer, (Layer *)error_icon_layer);

  // Set up the error text
  #ifdef PBL_ROUND
    // Round spacing
    error_text_layer = text_layer_create(GRect(0, 110, bounds.size.w, bounds.size.h - 110));
  #else
    // Square spacing
    error_text_layer = text_layer_create(GRect(0, 100, bounds.size.w, bounds.size.h - 100));
  #endif
  text_layer_set_text(error_text_layer, text_error);
  text_layer_set_font(error_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(error_text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(error_text_layer, GColorBlack);
  #ifdef PBL_COLOR
    // Color only
    text_layer_set_background_color(error_text_layer, COLOR_ERROR);
  #endif

  layer_add_child(window_layer, (Layer *)error_text_layer);
  #ifdef PBL_ROUND
    // Round text insets
    text_layer_enable_screen_text_flow_and_paging(error_text_layer, 0);
  #endif
}

void error_window_push(enum ErrorType error_type) {
  display_error_type = error_type;
  if (!window) {
    // Create the window
    window = window_create();

    // Set up the window handlers
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  // Display the window
  window_stack_push(window, true);
}
