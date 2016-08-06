#include <pebble.h>

static Window* window;
static TextLayer* text_layer;
static int sensitivity;

static void accel_data_handler(AccelData* data, uint32_t num_samples) {
  // calculate y-axis derivative
  static char buf[128];
  int dy = data[num_samples-1].y - data[0].y;
  if (dy > sensitivity) {
    snprintf(buf, sizeof(buf), "Delta:%d\nHandshake!",dy);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d\nHandshake!",dy);
    text_layer_set_text(text_layer, buf);
    vibes_double_pulse();
  } else {
    snprintf(buf, sizeof(buf), "Delta: %d", dy);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d", dy);
    text_layer_set_text(text_layer, buf);
  }
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    text_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 60 } });
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
}

static void init(void) {
    window = window_create();
//    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
            });
    sensitivity = 850;
    accel_data_service_subscribe(5, accel_data_handler);
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    deinit();
}

// static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//     // Increase threshold
//     static char buf[128];
//     sensitivity += 25;
//     snprintf(buf, sizeof(buf), "Threshold: %d", sensitivity);
//     text_layer_set_text(text_layer, buf);
// }

// static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//     // Resume shock display
//     static char buf[128];
//     sensitivity -= 25;
//     snprintf(buf, sizeof(buf), "Threshold: %d", sensitivity);
//     text_layer_set_text(text_layer, buf);
// }
// static void click_config_provider(void *context) {
//     window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//     window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
// }

