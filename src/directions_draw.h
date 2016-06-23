#pragma once
#include <pebble.h>

// Return the height of a cell (this groupes layout functions in one file)
int16_t directions_get_cell_height(int16_t window_height, int16_t cell_index);
// Draw cells
void directions_draw_summary(GContext *ctx, GRect bounds, GColor color, int distance, int time);
void directions_draw_step(GContext *ctx, GRect bounds, GColor color, char *text, int16_t data_index, int16_t data_count);
