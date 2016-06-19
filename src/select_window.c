#include <pebble.h>
#include "select_window.h"

#define COLOR_FIRST COLOR_CAR

// The main window
static Window *window;

static MenuLayer *transit_mode_menu;

// Transit mode icons
static GBitmap *icon_car;
static GBitmap *icon_car_black;
static GBitmap *icon_bike;
static GBitmap *icon_bike_black;
static GBitmap *icon_train;
static GBitmap *icon_train_black;
static GBitmap *icon_walking;
static GBitmap *icon_walking_black;

// Callback for number of rows
static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_TRANSIT_METHODS;
}

// Draw menu cell callback
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Is the current cell highlighted?
  bool highlighted = menu_cell_layer_is_highlighted(cell_layer);
  // Determine what row to draw
  switch (cell_index->row) {
    // Driving / Car
    case 0:
      if (highlighted) {
        menu_cell_basic_draw(ctx, cell_layer, "Driving", NULL, NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "Driving", NULL, NULL);
      }
      break;
    // Riding the bike
    case 1:
      if (highlighted) {
        menu_cell_basic_draw(ctx, cell_layer, "Cycling", NULL, NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "Cycling", NULL, NULL);
      }
      break;
    // Going by train / public transit
    case 2:
      if (highlighted) {
        menu_cell_basic_draw(ctx, cell_layer, "Transit", NULL, NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "Transit", NULL, NULL);
      }
      break;
    // Walking
    case 3:
      if (highlighted) {
        menu_cell_basic_draw(ctx, cell_layer, "Walking", NULL, NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "Walking", NULL, NULL);
      }
      break;
  }
}

static void selection_will_change_callback(struct MenuLayer *menu_layer, MenuIndex *new_index, MenuIndex old_index, void *context) {
  // Change the highlight color
  #ifdef PBL_COLOR
    switch (new_index->row) {
      // Driving / Car
      case 0:
        menu_layer_set_highlight_colors(transit_mode_menu, COLOR_CAR, GColorWhite);
        break;
      // Riding the bike
      case 1:
        menu_layer_set_highlight_colors(transit_mode_menu, COLOR_BIKE, GColorWhite);
        break;
      // Going by train / public transit
      case 2:
        menu_layer_set_highlight_colors(transit_mode_menu, COLOR_TRAIN, GColorWhite);
        break;
      // Walking
      case 3:
        menu_layer_set_highlight_colors(transit_mode_menu, COLOR_WALKING, GColorWhite);
        break;
    }
  #endif
}

// Select cell callback
static void select_cell_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  // Tell which action to perform TODO: Transfer this info to the js part!
  switch (cell_index->row) {
    // Driving / Car
    case 0:
      printf("Car\n");
      break;
    // Riding the bike
    case 1:
      printf("Bike\n");
      break;
    // Going by train / public transit
    case 2:
      printf("Train\n");
      break;
    // Walking
    case 3:
      printf("Walking\n");
      break;
  }
  // Open the speak stuff...
}

// Window unload handler
static void window_unload() {
  // Destroy the menu layer
  menu_layer_destroy(transit_mode_menu);

  // Destroy all images
  gbitmap_destroy(icon_car);
  gbitmap_destroy(icon_car_black);
  gbitmap_destroy(icon_bike);
  gbitmap_destroy(icon_bike_black);
  gbitmap_destroy(icon_train);
  gbitmap_destroy(icon_train_black);
  gbitmap_destroy(icon_walking);
  gbitmap_destroy(icon_walking_black);

  // Destroy the window
  window_destroy(window);
  window = NULL;
}

// Window load stuff
static void window_load() {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the menu layer
  transit_mode_menu = menu_layer_create(bounds);
  // Set the click event handles to the correct window
  menu_layer_set_click_config_onto_window(transit_mode_menu, window);
  // If color, set the starting highlighted color
  #ifdef PBL_COLOR
    menu_layer_set_highlight_colors(transit_mode_menu, COLOR_FIRST, GColorWhite);
  #endif

  // Define menu callbacks
  menu_layer_set_callbacks(transit_mode_menu, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .select_click = select_cell_callback,
    .selection_will_change = selection_will_change_callback,
  });

  // Add the menu layer to the window
  layer_add_child(window_layer, menu_layer_get_layer(transit_mode_menu));

  // Load the icon images
  /*icon_car = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_WHITE);
  icon_car_black = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_BLACK);
  icon_bike = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_WHITE);
  icon_bike_black = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_BLACK);
  icon_train = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRAIN_WHITE);
  icon_train_black = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRAIN_BLACK);
  icon_walking = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_WHITE);
  icon_walking_black = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR_BLACK);*/
}

// Push the window to the window stack
void select_window_push() {
  if (!window) {
    // Create the window
    window = window_create();

    // Initialise the window event handlers
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  // Push window to screen
  window_stack_push(window, true);
}
