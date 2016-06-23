#include <pebble.h>
#include "progress_layer.h"

// Local state (used to ransfer the progress to the update proc)
int16_t l_progress = 0;

// Local functions
void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  // Get the layers bounds
  GRect bounds = layer_get_bounds(layer);
  // Work out the colors
  #ifdef PBL_COLOR
    GColor color_bg = COLOR_PROGRESS_BG;
    GColor color_bar = COLOR_PROGRESS_BAR;
    GColor color_fill = COLOR_PROGRESS_FILL;
  #else
    GColor color_bg = GColorWhite;
    GColor color_bar = GColorWhite;
    GColor color_fill = GColorBlack;
  #endif

  // Draw the bg
  graphics_context_set_fill_color(ctx, color_bg);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  GRect bar = GRect((bounds.size.w - PROGRESS_BAR_WIDTH) / 2, (bounds.size.h - PROGRESS_BAR_HEIGHT) / 2, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT);
  graphics_context_set_fill_color(ctx, color_bar);
  graphics_fill_rect(ctx, bar, 2, GCornersAll);

  // Draw the progress
  l_progress = l_progress > 100 ? 100 : l_progress < 0 ? 0 : l_progress; // Cage l_progress between 0 <-> 100
  int16_t progress_width = PROGRESS_BAR_WIDTH * (l_progress / 100.0);
  GRect fill = GRect((bounds.size.w - PROGRESS_BAR_WIDTH) / 2, (bounds.size.h - PROGRESS_BAR_HEIGHT) / 2, progress_width, PROGRESS_BAR_HEIGHT);
  graphics_context_set_fill_color(ctx, color_fill);
  graphics_fill_rect(ctx, fill, 2, GCornersAll);
}

// Handle the mark dirty and the l_progress transfer
void progress_layer_mark_dirty(ProgressLayer *progress_layer) {
  // Store the progress locally
  l_progress = progress_layer->progress;
  // Update the ui
  layer_mark_dirty(progress_layer->layer);
}

void progress_layer_animate(void *data) {
  ProgressLayer *progress_layer = (ProgressLayer *)data;
  if (progress_layer->progress < progress_layer->target) {
    // Increment progress & call update proc
    progress_layer->progress ++;
    progress_layer_mark_dirty(progress_layer);
    // Shedule the timer
    progress_layer->timer = app_timer_register(PROGRESS_ANIM_DELTA, progress_layer_animate, progress_layer);
  } else if (progress_layer->progress > progress_layer->target) {
    // Decrement progress & call update proc
    progress_layer->progress --;
    progress_layer_mark_dirty(progress_layer);
    // Shedule the timer
    progress_layer->timer = app_timer_register(PROGRESS_ANIM_DELTA, progress_layer_animate, progress_layer);
  }
}

// *********************
// * EXPOSED FUNCTIONS *
// *********************

// Create the progress layer
ProgressLayer *progress_layer_create(GRect frame) {
  // Allocate the progress layer
  ProgressLayer *progress_layer = malloc(sizeof(ProgressLayer));
  // Allocate the layer component and register its update proc
  progress_layer->layer = layer_create(frame);
  layer_set_update_proc(progress_layer->layer, progress_layer_update_proc);
  // Set the timer to be NULL
  progress_layer->timer = NULL;
  // Set the initial progress
  progress_layer->progress = 0;
  progress_layer->target = 0;
  progress_layer->present = false;
  // Return the progress layer
  return progress_layer;
}

// Destroy the progress layer (call in window unload)
void progress_layer_destroy(ProgressLayer *progress_layer) {
  // Destroy the layer component (the timer destroys itself I guess?)
  if (progress_layer != NULL) {
    layer_destroy(progress_layer->layer);
    free(progress_layer);
  }
}

// Set the progress of the progress layer
void progress_layer_set_progress(ProgressLayer *progress_layer, int16_t progress, bool animated) {
  if (animated && progress_layer->present) {
    // Call the animate function
    progress_layer->target = progress;
    progress_layer_animate(progress_layer);
  } else {
    // Set the new progress & update the layer if present
    progress_layer->progress = progress;
    if (progress_layer->present) progress_layer_mark_dirty(progress_layer);
  }
}

// This allows the set_progress function to start timers
void progress_layer_present(Window *window, ProgressLayer *progress_layer) {
  // Add to the windows root layer
  Layer *window_root =  window_get_root_layer(window);
  layer_add_child(window_root, progress_layer->layer);
  // Mark the layer as present
  progress_layer->present = true;
}

// This call should always be called in the windows disappear! (will stop the timer)
void progress_layer_remove(ProgressLayer *progress_layer) {
  // Mark the layer as not present
  progress_layer->present = false;
  // Stop all timers
  app_timer_cancel(progress_layer->timer);
  // Remove the layer from the parent
  layer_remove_from_parent(progress_layer->layer);
}
