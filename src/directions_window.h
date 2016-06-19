#pragma once
#include <pebble.h>

enum SelectedType {
  Car = 0,
  Bike = 1,
  Train = 2,
  Walk = 3
};

int selected_type_enum;

void directions_window_push();
