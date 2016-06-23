#pragma once
#include <pebble.h>

// Required functions
#include "colors.h"

enum ErrorType {
  // Problem connection to phone
  Network = 0,
  // Error response from api
  Api = 1,
  // Something simply went wrong
  Other = 2
};

void error_window_push(enum ErrorType);
