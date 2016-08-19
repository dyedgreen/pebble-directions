#pragma once
#include <pebble.h>

// Required functions
#include "colors.h"

enum ErrorType {
  // Problem connection to phone
  Network = 0,
  // Error response from api
  Api = 1,
  // Error when trying to start the dictation
  Dictation = 2,
  // Error when a feature is unavailable
  Unavailable = 3,
  // Something simply went wrong
  Other = 4
};

void error_window_push(enum ErrorType);
