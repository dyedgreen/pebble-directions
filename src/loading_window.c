#include <pebble.h>
#include "loading_window.h"

// TODO: Tidy this! (will do later)
// TODO: disconnect loading / display window to make things work better soon! (or move the 'loading window' to a 'loading layer'...)
// Some stuff added by me
static bool will_close;

// The loading window, implementation stolen from pebble UI examples
typedef Layer ProgressLayer;

#define MIN(a,b) (((a)<(b))?(a):(b))

ProgressLayer* progress_layer_create(GRect frame);
void progress_layer_destroy(ProgressLayer* progress_layer);
void progress_layer_increment_progress(ProgressLayer* progress_layer, int16_t progress);
void progress_layer_set_progress(ProgressLayer* progress_layer, int16_t progress_percent);
void progress_layer_set_corner_radius(ProgressLayer* progress_layer, uint16_t corner_radius);
void progress_layer_set_foreground_color(ProgressLayer* progress_layer, GColor color);
void progress_layer_set_background_color(ProgressLayer* progress_layer, GColor color);

typedef struct {
  int16_t progress_percent;
  int16_t corner_radius;
  GColor foreground_color;
  GColor background_color;
} ProgressLayerData;

static int16_t scale_progress_bar_width_px(unsigned int progress_percent, int16_t rect_width_px) {
  return ((progress_percent * (rect_width_px)) / 100);
}

static void progress_layer_update_proc(ProgressLayer* progress_layer, GContext* ctx) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  GRect bounds = layer_get_bounds(progress_layer);

  int16_t progress_bar_width_px = scale_progress_bar_width_px(data->progress_percent, bounds.size.w);
  GRect progress_bar = GRect(bounds.origin.x, bounds.origin.y, progress_bar_width_px, bounds.size.h);

  graphics_context_set_fill_color(ctx, data->background_color);
  graphics_fill_rect(ctx, bounds, data->corner_radius, GCornersAll);

  graphics_context_set_fill_color(ctx, data->foreground_color);
  graphics_fill_rect(ctx, progress_bar, data->corner_radius, GCornersAll);

#ifdef PBL_PLATFORM_APLITE
  graphics_context_set_stroke_color(ctx, data->background_color);
  graphics_draw_rect(ctx, progress_bar);
#endif
}

ProgressLayer* progress_layer_create(GRect frame) {
  ProgressLayer *progress_layer = layer_create_with_data(frame, sizeof(ProgressLayerData));
  layer_set_update_proc(progress_layer, progress_layer_update_proc);
  layer_mark_dirty(progress_layer);

  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = 0;
  data->corner_radius = 1;
  data->foreground_color = GColorBlack;
  data->background_color = GColorWhite;

  return progress_layer;
}

void progress_layer_destroy(ProgressLayer* progress_layer) {
  if (progress_layer) {
    layer_destroy(progress_layer);
  }
}

void progress_layer_increment_progress(ProgressLayer* progress_layer, int16_t progress) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = MIN(100, data->progress_percent + progress);
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_progress(ProgressLayer* progress_layer, int16_t progress_percent) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->progress_percent = MIN(100, progress_percent);
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_corner_radius(ProgressLayer* progress_layer, uint16_t corner_radius) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->corner_radius = corner_radius;
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_foreground_color(ProgressLayer* progress_layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->foreground_color = color;
  layer_mark_dirty(progress_layer);
}

void progress_layer_set_background_color(ProgressLayer* progress_layer, GColor color) {
  ProgressLayerData *data = (ProgressLayerData *)layer_get_data(progress_layer);
  data->background_color = color;
  layer_mark_dirty(progress_layer);
}

static Window *s_window;
static ProgressLayer *s_progress_layer;

static AppTimer *s_timer;
static int s_progress;

static void progress_callback(void *context);

static void next_timer() {
  s_timer = app_timer_register(PROGRESS_LAYER_WINDOW_DELTA, progress_callback, NULL);
}

static void progress_callback(void *context) {
  s_progress += (s_progress < 85 || will_close) ? 1 : 0;
  progress_layer_set_progress(s_progress_layer, s_progress);
  next_timer();
  // Close when progress hits 100
  if (s_progress == 100) {
    window_stack_remove(s_window, true);
  }
}

static void window_load(Window *window) {
  will_close = false;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_progress_layer = progress_layer_create(GRect((bounds.size.w - PROGRESS_LAYER_WINDOW_WIDTH) / 2, (bounds.size.h - 3) / 2, PROGRESS_LAYER_WINDOW_WIDTH, 6));
  progress_layer_set_progress(s_progress_layer, 0);
  progress_layer_set_corner_radius(s_progress_layer, 2);
  progress_layer_set_foreground_color(s_progress_layer, GColorWhite);
  progress_layer_set_background_color(s_progress_layer, GColorBlack);
  layer_add_child(window_layer, s_progress_layer);
}

static void window_unload(Window *window) {
  progress_layer_destroy(s_progress_layer);

  window_destroy(window);
  s_window = NULL;
}

static void window_appear(Window *window) {
  s_progress = 0;
  next_timer();
}

static void window_disappear(Window *window) {
  if(s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
}

void loading_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .appear = window_appear,
      .disappear = window_disappear,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void loading_window_finish() {
  will_close = true;
}
