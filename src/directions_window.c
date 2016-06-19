#include <pebble.h>
#include "colors.h"
#include "directions_window.h"

#include <pebble.h>
#include "colors.h"
#include "select_window.h"
#include "directions_window.h"

// The main window
static Window *window;
static int window_height;

static MenuLayer *directions_list;
static GColor selected_type_color;

static DictationSession *dictation_session;
static char *address;


// ******************************
// * DIRECTON LIST (MENU LAYER) *
// ******************************

// Callback for number of rows
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 3;
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  // This part is for round watches only
  #if defined(PBL_ROUND)
    // Round
    return window_height;
  #endif
  // Square fallback
  return window_height / 2;
}

// Draw menu cell callback TODO: use a custom ui
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Determine what row to draw
  switch (cell_index->row) {
    // Summary
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "11 min", "600 m", NULL);
      break;
    // Step description
    default:
      menu_cell_basic_draw(ctx, cell_layer, address, "30 m", NULL);
  }
}

static void selection_will_change_callback(struct MenuLayer *menu_layer, MenuIndex *new_index, MenuIndex old_index, void *context) {
  // Change the highlight color
  #ifdef PBL_COLOR
    switch (new_index->row) {
      // Initial summary
      case 0:
        menu_layer_set_highlight_colors(directions_list, selected_type_color, GColorWhite);
        break;
      // All other cells
      default:
        menu_layer_set_highlight_colors(directions_list, GColorWhite, selected_type_color);
        break;
    }
  #endif
}


// ***********************
// * DICTATION API STUFF *
// ***********************

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcript, void *context) {
  // Test if dication was successfull
  if (status == DictationSessionStatusSuccess) {
    // Store the address and load a route
    address = transcript;
    // TODO load route data
  } else {
    // TODO display proper error window
    window_stack_remove(window, true);
  }
}


// ******************
// * APP SYNC STUFF *
// ******************

// ********************
// * WINDOW LIFECYCLE *
// ********************

// Window unload handler
static void window_unload() {
  // Destroy the menu layer
  menu_layer_destroy(directions_list);

  // Destroy the dictation session
  dictation_session_destroy(dictation_session);

  // Destroy the window
  window_destroy(window);
  window = NULL;
}

// Window load stuff
static void window_load() {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_height = bounds.size.h;

  // Work out the correct type color
  switch (selected_type_enum) {
    case 0:
      selected_type_color = COLOR_CAR;
      break;
    case 1:
      selected_type_color = COLOR_BIKE;
      break;
    case 2:
      selected_type_color = COLOR_TRAIN;
      break;
    case 3:
      selected_type_color = COLOR_WALK;
      break;
    default:
      selected_type_color = GColorBlack;
  }

  // Set up the dictation session
  dictation_session = dictation_session_create(0, dictation_session_callback, NULL);
  dictation_session_enable_confirmation(dictation_session, true);
  dictation_session_enable_error_dialogs(dictation_session, true);

  // Create the menu layer
  directions_list = menu_layer_create(bounds);
  // Set the click event handles to the correct window
  menu_layer_set_click_config_onto_window(directions_list, window);
  // If color, set the starting highlighted color
  #ifdef PBL_COLOR
    menu_layer_set_highlight_colors(directions_list, selected_type_color, GColorWhite);
  #endif

  // Define menu callbacks
  menu_layer_set_callbacks(directions_list, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .get_cell_height = get_cell_height_callback,
    .draw_row = draw_row_callback,
    .selection_will_change = selection_will_change_callback,
  });

  // Add the menu layer to the window
  layer_add_child(window_layer, menu_layer_get_layer(directions_list));
}

// Push the window to the window stack
void directions_window_push() {
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

  // Start the dictation session TODO: Change this, once app sync stuff is working
  // dictation_session_start(dictation_session);
  address = "Meerbusch Brockhofweg 9"; // WIP address string
}
