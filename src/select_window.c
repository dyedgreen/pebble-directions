#include "select_window.h"


#define COLOR_FIRST COLOR_CAR

// The main window
static Window *window;

static MenuLayer *transit_mode_menu;

// Transit mode icons
static GBitmap *icon_car;
static GBitmap *icon_bike;
static GBitmap *icon_train;
static GBitmap *icon_walk;

// Callback for number of rows
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_TRANSIT_METHODS;
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  // This part is for round watches only
  #ifdef PBL_ROUND
    // Round
    bool selected = menu_layer_is_index_selected(menu_layer, cell_index);
    if (selected) {
      return 69;
    } else {
      return 44;
    }
  #endif
  // Square fallback
  return 44;
}

// Draw menu cell callback
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Determine what row to draw
  switch (cell_index->row) {
    // Driving / Car
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Driving", NULL, icon_car);
      break;
    // Riding the bike
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Cycling", NULL, icon_bike);
      break;
    // Going by train / public transit
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Transit", NULL, icon_train);
      break;
    // walk
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Walk", NULL, icon_walk);
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
      // walk
      case 3:
        menu_layer_set_highlight_colors(transit_mode_menu, COLOR_WALK, GColorWhite);
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
      selected_type_enum = Car;
      break;
    // Riding the bike
    case 1:
      selected_type_enum = Bike;
      break;
    // Going by train / public transit
    case 2:
      selected_type_enum = Train;
      break;
    // walk
    case 3:
      selected_type_enum = Walk;
      break;
  }
  // Open the directions window
  directions_window_push();
}

// Window unload handler
static void window_unload() {
  // Destroy the menu layer
  menu_layer_destroy(transit_mode_menu);

  // Destroy all images
  gbitmap_destroy(icon_car);
  gbitmap_destroy(icon_bike);
  gbitmap_destroy(icon_train);
  gbitmap_destroy(icon_walk);

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
    .get_cell_height = get_cell_height_callback,
    .draw_row = draw_row_callback,
    .select_click = select_cell_callback,
    .selection_will_change = selection_will_change_callback,
  });

  // Add the menu layer to the window
  layer_add_child(window_layer, menu_layer_get_layer(transit_mode_menu));

  // Load the icon images
  icon_car = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR);
  icon_bike = gbitmap_create_with_resource(RESOURCE_ID_ICON_BIKE);
  icon_train = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRAIN);
  icon_walk = gbitmap_create_with_resource(RESOURCE_ID_ICON_WALK);
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
