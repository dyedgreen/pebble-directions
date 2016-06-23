#pragma once
#include <pebble.h>

#define PROGRESS_BAR_WIDTH 80
#define PROGRESS_BAR_HEIGHT 6
#define PROGRESS_ANIM_DELTA 33

// For B/W Pebbels: bg white, bar white, fill black
#define COLOR_PROGRESS_BG GColorLightGray
#define COLOR_PROGRESS_BAR GColorBlack
#define COLOR_PROGRESS_FILL GColorWhite

typedef struct {
  Layer *layer;
  AppTimer *timer;
  int16_t progress;
  int16_t target;
  bool present;
} ProgressLayer;

// Create the progress layer
ProgressLayer *progress_layer_create(GRect frame);
// Destroy the progress layer (call in window unload)
void progress_layer_destroy(ProgressLayer *progress_layer);
// Set the progress of the progress layer
void progress_layer_set_progress(ProgressLayer *progress_layer, int16_t progress, bool animated);
// This allows the set_progress function to start timers!
void progress_layer_present(Window *window, ProgressLayer *progress_layer);
// This call should always be called in the windows disappear! (will stop the timer)
void progress_layer_remove(ProgressLayer *progress_layer);
