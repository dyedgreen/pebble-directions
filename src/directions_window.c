#include <pebble.h>
#include "colors.h"
#include "select_window.h"
#include "directions_window.h"
#include "error_window.h"
#include "loading_window.h"

#define MAX_STEP_COUNT 20
#define MAX_STEP_CHARS 128

// The RouteData struct
struct RouteData {
  // App message state TODO: Is this necessary? (-> probably not!)
  bool ready;
  bool callback;
  // Data fields
  int distance;
  int time;
  char steps[MAX_STEP_COUNT][MAX_STEP_CHARS];
  int count;
};

// The main window
static Window *window;
static int window_height;

static MenuLayer *directions_list;
static GColor selected_type_color;

static DictationSession *dictation_session;
static char *address;

// TODO: implement a timer, that kills the proccess if the message is never send (needed?)
struct RouteData *route_data;

// Function declarations
static void app_message_send_search_data();
void window_display_error(enum ErrorType err);
static void window_update_data();


// ******************************
// * DIRECTON LIST (MENU LAYER) *
// ******************************

// Callback for number of rows
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return route_data != NULL ? route_data->count + 1 : 0;
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  // This part is for round watches only
  #ifdef PBL_ROUND
    // Round
    return window_height;
  #endif
  // Square fallback
  return window_height / 2;
}

// Draw menu cell callback TODO: use a custom ui
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Determine what row to draw TODO: implement nicer UI
  switch (cell_index->row) {
    // Summary
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "11 min", "600 m", NULL);
      break;
    // Step description
    default:
      menu_cell_basic_draw(ctx, cell_layer, route_data->steps[cell_index->row - 1], "sub title", NULL);
  }
}

// FIXME: Will probably not be necessary later!
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
    // Send the search data
    app_message_send_search_data();
  } else {
    // Dictation failed, remove this window
    window_stack_remove(window, false);
  }
}


// *********************
// * APP MESSAGE STUFF *
// *********************

// Accept data from the watch
static void app_message_inbox_recived_callback(DictionaryIterator *iter, void *context) {
  // Test all possible message types
  Tuple *message;

  // Test if the recived message is for key READY
  message = dict_find(iter, MESSAGE_KEY_READY);
  if (message) {
    // Set status to ready
    route_data->ready = (bool)message->value->int16;
    // Send pending search data
    if (route_data->callback) {
      app_message_send_search_data();
    }
  }

  // Test if the recived message is for key SUCCESS
  message = dict_find(iter, MESSAGE_KEY_SUCCESS);
  if (message) {
    // Respond with the correct UI
    switch ((int)message->value->int32) {
      // Success
      case 0:
        window_update_data();
        break;
      // Route not found / api error
      case 1:
        window_display_error(Api);
        break;
      // Too many steps (== route not found error)
      case 2:
        window_display_error(Api);
        break;
      default:
        window_display_error(Other);
    }
  }

  // Test if the recived message is for key DISTANCE
  message = dict_find(iter, MESSAGE_KEY_DISTANCE);
  if (message) {
    route_data->distance = (int)message->value->int32;
  }

  // Test if the recived message is for key TIME
  message = dict_find(iter, MESSAGE_KEY_TIME);
  if (message) {
    route_data->time = (int)message->value->int32;
  }

  // Test if the recived message is for key INSTRUCTIONS
  for (int i = 0; i < MAX_STEP_COUNT; i++) {
    message = dict_find(iter, MESSAGE_KEY_INSTRUCTIONS + i);
    if (message) {
      // Copy the string into the string array FIXME
      strcpy(route_data->steps[i], message->value->cstring);
      route_data->steps[i][MAX_STEP_CHARS - 1] = '\0';
      // Store the new length of the RouteDataSteps
      route_data->count = i + 1;
    }
  }
}

// Respond with error, if any data is lost
static void app_message_inbox_dropped_callback(AppMessageResult reason, void *context) {
  // Display the network error
  window_display_error(Network);
}

// Respond with error, if data can not be send to watch
static void app_message_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  // Display the network error
  window_display_error(Network);
}

// Send the search data to the phone
static void app_message_send_search_data() {
  // Send the data to the phone if conn is ready
  if (route_data->ready) {
    // Create a string with the correct length
    char message[sizeof(address) + 1];
    // Format the string FIXME (cuts of after 'mee' for the test string ???)
    snprintf(message, sizeof(message), "%i%s", selected_type_enum, address);
    // Make sure the string is terminated correctely (just in case)
    message[sizeof(message) - 1] = '\0';

    // Write string to bluetooth storage
    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) == APP_MSG_OK) {
      dict_write_cstring(iter, MESSAGE_KEY_SEARCH, message);
      // Send the outbox
      app_message_outbox_send();
      // Display loading anim window
      loading_window_push();
    } else {
      // Display network error
      window_display_error(Network);
    }
  } else {
    route_data->callback = true;
  }
}

// Set up the whole app message thing (once the address is worked out)
static void app_message_start() {
  // Set up the data
  route_data = malloc(sizeof(struct RouteData));

  // Set initial values
  route_data->ready = true;
  route_data->callback = false;
  route_data->distance = 0;
  route_data->time = 0;
  route_data->count = 0;
  // Register all callbacks
  app_message_register_inbox_received(app_message_inbox_recived_callback);
  app_message_register_inbox_dropped(app_message_inbox_dropped_callback);
  app_message_register_outbox_failed(app_message_outbox_failed_callback);
  // Open the app-message
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

static void app_message_destroy_resources() {
  // Remove all app message callbacks
  app_message_deregister_callbacks();
  // Clear the data
  free(route_data);
}


// ********************
// * WINDOW LIFECYCLE *
// ********************

// Network error callback
void window_display_error(enum ErrorType err) {
  // Remove the loading window
  loading_window_finish();
  // Show the error window
  error_window_push(err);
  // Remove this window from the window stack
  window_stack_remove(window, false);
}

// Update the data in the window
static void window_update_data() {
  // Update the menu layer
  #ifdef PBL_ROUND
    menu_layer_set_selected_index(directions_list, (MenuIndex){ .section = 0, .row = 0 }, MenuRowAlignCenter, false);
  #else
    menu_layer_set_selected_index(directions_list, (MenuIndex){ .section = 0, .row = 0 }, MenuRowAlignTop, false);
  #endif
  menu_layer_reload_data(directions_list);
  // Hide the loading view
  loading_window_finish();
}

// Window unload handler
static void window_unload() {
  // Destroy the menu layer
  menu_layer_destroy(directions_list);

  // Destroy the dictation session
  dictation_session_destroy(dictation_session);

  // Destroy app message stuff
  app_message_destroy_resources();
  route_data = NULL;

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

  // Open the connection to the phone
  app_message_start();
  // TODO: Remove this for final version; this is a playeholder implementation (skipping the dictation)
  app_message_send_search_data();
}
