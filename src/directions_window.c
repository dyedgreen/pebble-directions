#include "directions_window.h"


#define MESSAGE_PADDING 10
#define MAX_STEP_COUNT 40
#define MAX_STEP_CHARS 128
#define ADDRESS_MAX_CHARS 128

// Progress display vals
#define PROGRESS_SEARCH_SEND 50
#define PROGRESS_DISTANCE_RECIVED 80
#define PROGRESS_TIME_RECIVED PROGRESS_DISTANCE_RECIVED
#define PROGRESS_ICONS_RECIVED PROGRESS_DISTANCE_RECIVED
#define PROGRESS_STEP_RECIVED 95
#define PROGRESS_SUCCESS_RECIVED 100

// The RouteData struct
struct RouteData {
  // Meta fields
  bool complete;
  int current;
  // Data fields
  int distance;
  int time;
  char icons[MAX_STEP_COUNT + 1];
  char steps[MAX_STEP_COUNT][MAX_STEP_CHARS];
  int count;
};

// The main window
static Window *window;
static int window_height;

static MenuLayer *directions_list;
static GColor selected_type_color;

static StatusBarLayer *status_bar;

// Step icons
static GBitmap *icon_step_type;
static GBitmap *icon_step_forward;
static GBitmap *icon_step_right;
static GBitmap *icon_step_left;
static GBitmap *icon_step_uright;
static GBitmap *icon_step_uleft;
static GBitmap *icon_step_attr;
static GBitmap *icon_step_final;

// Dictation input
static DictationSession *dictation_session;
static char address[ADDRESS_MAX_CHARS];

// Route data / App message
struct RouteData *route_data;
ProgressLayer *progress_layer;
int message_number = -1;

// Function declarations
static void app_message_send_search_data();
void window_display_error(enum ErrorType err);
static void window_update_data();
static void window_update_step();


// ******************************
// * DIRECTON LIST (MENU LAYER) *
// ******************************

// Callback for number of rows
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return route_data != NULL ? route_data->count + 1 : 0;
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return directions_get_cell_height(window_height, cell_index->row);
}

// Draw menu cell callback
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Local vars
  int data_index = cell_index->row - 1;
  GBitmap *icon;
  // Determine what row to draw
  switch (cell_index->row) {
    // Summary
    case 0:
      directions_draw_summary(ctx, layer_get_bounds(cell_layer), selected_type_color, route_data->distance, route_data->time, address);
      break;
    // Step description
    default:
      // Select the correct icon
      switch (route_data->icons[data_index]) {
        case 'a':
          icon = icon_step_type;
          break;
        case 'b':
          icon = icon_step_forward;
          break;
        case 'c':
          icon = icon_step_right;
          break;
        case 'd':
          icon = icon_step_left;
          break;
        case 'e':
          icon = icon_step_uright;
          break;
        case 'f':
          icon = icon_step_uleft;
          break;
        case 'g':
          icon = icon_step_attr;
          break;
        case 'h':
          icon = icon_step_final;
          break;
        default:
          icon = icon_step_type;
      }
      directions_draw_step(ctx, layer_get_bounds(cell_layer), selected_type_color, route_data->steps[data_index], icon, data_index, (int16_t)route_data->count);
  }
}


// ***********************
// * DICTATION API STUFF *
// ***********************

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcript, void *context) {
  // Test if dication was successfull
  if (status == DictationSessionStatusSuccess) {
    // Store the address and load a route
    strcpy(address, transcript);
    // Send the search data
    app_message_send_search_data();
  } else {
    // Dictation failed, remove this window
    window_stack_pop(false);
  }
}

static void dictation_session_start_handler() {
  // Start the dictation session and handel all possible errors
  DictationSessionStatus dictation_status = dictation_session_start(dictation_session);
  if (dictation_status != DictationSessionStatusSuccess) {
    // Display error
    window_display_error(Dictation);
  }
}


// *********************
// * APP MESSAGE STUFF *
// *********************

// Accept data from the watch (will also move to the correct progress indications)
static void app_message_inbox_recived_callback(DictionaryIterator *iter, void *context) {
  // Test all possible message types
  Tuple *message;

  // Test if the recived message is for key SUCCESS
  message = dict_find(iter, MESSAGE_KEY_SUCCESS);
  if (message) {
    // Set the progress
    progress_layer_set_progress(progress_layer, PROGRESS_SUCCESS_RECIVED, true);
    // Convert the message value
    int success_response = (int)message->value->int32;
    success_response -= message_number * MESSAGE_PADDING;
    // Drop response, if it is old (smaller than 0)
    if (success_response >= 0) {
      // Respond with the correct UI
      switch (success_response) {
        // Success
        case 0:
          route_data->complete = true;
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
  }

  // Test if the recived message is for key DISTANCE
  message = dict_find(iter, MESSAGE_KEY_DISTANCE);
  if (message) {
    // Set the progress
    progress_layer_set_progress(progress_layer, PROGRESS_DISTANCE_RECIVED, true);
    route_data->distance = (int)message->value->int32;
  }

  // Test if the recived message is for key TIME
  message = dict_find(iter, MESSAGE_KEY_TIME);
  if (message) {
    // Set the progress
    progress_layer_set_progress(progress_layer, PROGRESS_TIME_RECIVED, true);
    route_data->time = (int)message->value->int32;
  }

  // Test if the recived message is fof key INSTRUCTION_ICONS
  message = dict_find(iter, MESSAGE_KEY_INSTRUCTION_ICONS);
  if (message) {
    // Set the progress
    progress_layer_set_progress(progress_layer, PROGRESS_ICONS_RECIVED, true);
    // Copy the icon string
    strcpy(route_data->icons, message->value->cstring);
    route_data->icons[MAX_STEP_COUNT] = '\0';
  }

  // Test if the recived message is for key INSTRUCTION_LIST
  message = dict_find(iter, MESSAGE_KEY_INSTRUCTION_LIST);
  if (message) {
    // Set the progress
    progress_layer_set_progress(progress_layer, PROGRESS_STEP_RECIVED, true);
    // Copy the string into the string array
    strcpy(route_data->steps[route_data->count], message->value->cstring);
    route_data->steps[route_data->count][MAX_STEP_CHARS - 1] = '\0';
    // Increment the number of recived steps
    route_data->count += 1;
  }

  // Test if the recived message is for key CURRENT
  message = dict_find(iter, MESSAGE_KEY_CURRENT);
  if (message) {
    // Set the current and update the ui if the data is already complete
    int new_step = (int)message->value->int32;
    if (route_data->complete && route_data->current < new_step && new_step < route_data->count) {
      route_data->current = new_step;
      window_update_step();
    }
  }
}

// Respond with error, if any data is lost
static void app_message_inbox_dropped_callback(AppMessageResult reason, void *context) {
  if (!route_data->complete) {
    // Display the network error (if the route data is not allready complete)
    window_display_error(Network);
  }
}

// Respond with error, if data can not be send to watch
static void app_message_outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  // Display the network error
  window_display_error(Network);
}

static void app_message_outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // Display the progress
  progress_layer_set_progress(progress_layer, PROGRESS_SEARCH_SEND, true);
  // Increment the message number
  message_number ++;
}

// Send the search data to the phone
static void app_message_send_search_data() {
  // Display the progress view
  progress_layer_set_progress(progress_layer, 0, false);
  progress_layer_present(window, progress_layer);
  // Create a string with the correct length (len is the length w/o the '/0' char)
  const int len = strlen(address) + 1;
  char message[len + 1];
  // Format the string FIXME (cuts of after 'mee' for the test string ???)
  snprintf(message, sizeof(message), "%i%s", selected_type_enum, address);
  // Make sure the string is terminated correctely (just in case)
  message[len] = '\0';

  // Write string to bluetooth storage
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) == APP_MSG_OK) {
    dict_write_cstring(iter, MESSAGE_KEY_SEARCH, message);
    // Send the outbox
    app_message_outbox_send();
  } else {
    // Display network error
    window_display_error(Network);
  }
}

// Set up the whole app message thing (once the address is worked out)
static void app_message_start() {
  // Set up the data
  route_data = malloc(sizeof(struct RouteData));

  // Set meta values
  route_data->complete = false;
  route_data->current = 0;
  // Set initial values
  route_data->distance = 0;
  route_data->time = 0;
  route_data->count = 0;
  // Initialise the icons string (has the length of MAX_STEP_COUNT + 1 for correct null termination)
  for (int i = 0; i < MAX_STEP_COUNT; i ++) {
    route_data->icons[i] = 'a';
  }
  route_data->icons[MAX_STEP_COUNT] = '\0';
  // Register all callbacks
  app_message_register_inbox_received(app_message_inbox_recived_callback);
  app_message_register_inbox_dropped(app_message_inbox_dropped_callback);
  app_message_register_outbox_failed(app_message_outbox_failed_callback);
  app_message_register_outbox_sent(app_message_outbox_sent_callback);
  // Open the app-message
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

static void app_message_resources_destroy() {
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
  // Pop this window from the window stack (window_stack_remove causes errors)
  window_stack_pop(false);
  // Show the error window
  error_window_push(err);
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
  // Hide the progress layer
  progress_layer_remove(progress_layer);
}

// Update the window display to reflrect the new current step and vibrate
static void window_update_step() {
  // Update the position of the step list
  #ifdef PBL_ROUND
    menu_layer_set_selected_index(directions_list, (MenuIndex){ .section = 0, .row = route_data->current + 1 }, MenuRowAlignCenter, true);
  #else
    menu_layer_set_selected_index(directions_list, (MenuIndex){ .section = 0, .row = route_data->current + 1 }, MenuRowAlignTop, true);
  #endif
  // Play a short vibration
  vibes_long_pulse();
}

// Window unload handler
static void window_unload() {
  // Destroy the menu layer
  menu_layer_destroy(directions_list);
  directions_list = NULL;

  // Destroy the status bar layer
  status_bar_layer_destroy(status_bar);
  status_bar = NULL;

  // Destroy the progress layer
  progress_layer_destroy(progress_layer);
  progress_layer = NULL;

  // Destroy the dictation session
  dictation_session_destroy(dictation_session);
  dictation_session = NULL;

  // Destroy app message stuff
  app_message_resources_destroy();
  route_data = NULL;

  // Destroy the window
  window_destroy(window);
  window = NULL;

  // Destroy the step icons
  gbitmap_destroy(icon_step_type);
  gbitmap_destroy(icon_step_forward);
  gbitmap_destroy(icon_step_right);
  gbitmap_destroy(icon_step_left);
  gbitmap_destroy(icon_step_uright);
  gbitmap_destroy(icon_step_uleft);
  gbitmap_destroy(icon_step_attr);
  gbitmap_destroy(icon_step_final);
}

static void window_disappear() {
  // Remove the progress layer (to kill all timers that are still running)
  progress_layer_remove(progress_layer);
}

// Window load stuff
static void window_load() {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_height = bounds.size.h;

  // Work out the correct type color
  #ifdef PBL_COLOR
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
  #else
    selected_type_color = GColorBlack;
  #endif

  // Create the menu layer
  directions_list = menu_layer_create(bounds);
  // Set the click event handles to the correct window
  menu_layer_set_click_config_onto_window(directions_list, window);
  menu_layer_set_highlight_colors(directions_list, GColorWhite, GColorBlack);

  // Define menu callbacks
  menu_layer_set_callbacks(directions_list, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .get_cell_height = get_cell_height_callback,
    .draw_row = draw_row_callback,
  });

  // Add the menu layer to the window
  layer_add_child(window_layer, menu_layer_get_layer(directions_list));

  // Create and set up the status bar layer
  status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(status_bar, selected_type_color, GColorWhite);
  status_bar_layer_set_separator_mode(status_bar, StatusBarLayerSeparatorModeNone);
  layer_add_child(window_layer, status_bar_layer_get_layer(status_bar));

  // Create the progress layer
  progress_layer = progress_layer_create(bounds);

  // Set up the dictation session
  dictation_session = dictation_session_create(ADDRESS_MAX_CHARS, dictation_session_callback, NULL);
  dictation_session_enable_confirmation(dictation_session, true);
  dictation_session_enable_error_dialogs(dictation_session, true);

  // Create the step icon resources
  switch (selected_type_enum) {
    case 0:
      // Car icon
      icon_step_type = gbitmap_create_with_resource(RESOURCE_ID_ICON_CAR);
      break;
    case 1:
      // Bike icon
      icon_step_type = gbitmap_create_with_resource(RESOURCE_ID_ICON_BIKE);
      break;
    case 2:
      // Train icon
      icon_step_type = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRAIN);
      break;
    case 3:
      // Walk icon
      icon_step_type = gbitmap_create_with_resource(RESOURCE_ID_ICON_WALK);
      break;
    default:
      // Forward icon
      icon_step_type = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_FORWARD);
  }
  icon_step_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_FORWARD);
  icon_step_right = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_RIGHT);
  icon_step_left = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_LEFT);
  icon_step_uright = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_URIGHT);
  icon_step_uleft = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_ULEFT);
  icon_step_attr = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_ATTR);
  icon_step_final = gbitmap_create_with_resource(RESOURCE_ID_ICON_STEP_FINAL);
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
      .disappear = window_disappear,
    });
  }

  // Push window to screen
  window_stack_push(window, true);

  // Start the dictation session
  dictation_session_start_handler();

  // Open the connection to the phone
  app_message_start();
}
