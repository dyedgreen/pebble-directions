#include <pebble.h>
#include <select_window.h>

static void init(void) {
  // Open the transit mode window
  select_window_push();
}

static void deinit(void) {

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
