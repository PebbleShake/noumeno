#include <pebble.h>

static Window* window;
static TextLayer* text_layer;
static TextLayer* time_layer, *date_layer;

static int sensitivity;

/*
	detect handshake
*/
static void accel_data_handler(AccelData* data, uint32_t num_samples) {
  // calculate y-axis derivative
  static char buf[128];
  int dy = data[num_samples-1].y - data[0].y;
	//if the handshake happens
  if (dy > sensitivity) {
    snprintf(buf, sizeof(buf), "HANDSHAKE!");
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d Handshake!",dy);
    //update text view
		text_layer_set_text(text_layer, buf);
    
		//double vibration
		vibes_double_pulse();
  }
	//if it doesn't
	else {
    snprintf(buf, sizeof(buf), "Delta: %d", dy);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d", dy);
    text_layer_set_text(text_layer, buf);
  }
}

/*
	update the time
*/
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(time_layer, s_buffer);
	
	//Write the current date
	static char date_buffer[16];
	strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

	// Show the date
	text_layer_set_text(date_layer, date_buffer);
}

/*
	tick handler event
*/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

/*
	initialise window
*/
static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
	
		//display text layer
    text_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h-20 }, .size = { bounds.size.w, 20 } });
		text_layer_set_text_color(text_layer, GColorWhite);
		text_layer_set_background_color(text_layer, GColorClear);
		layer_add_child(window_layer, text_layer_get_layer(text_layer));
	
		//contruct time layer
		time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(62, 57), bounds.size.w, 50));
		text_layer_set_background_color(time_layer, GColorClear);
  	text_layer_set_text_color(time_layer, GColorWhite);
  	text_layer_set_text(time_layer, "00:00");
  	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
		//display it
		layer_add_child(window_layer, text_layer_get_layer(time_layer));
	
		//construct date layer
		date_layer = text_layer_create(GRect(0, 45, bounds.size.w, 30));
		text_layer_set_text_color(date_layer, GColorWhite);
		text_layer_set_text(date_layer, "Today");
		text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
		text_layer_set_background_color(date_layer, GColorClear);
		text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
		// display it
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
}

/*
	destroy window when unused
*/
static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
		text_layer_destroy(time_layer);
		text_layer_destroy(date_layer);
}

/*
	initialise watchface
*/
static void init(void) {
    //init window
		window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
            });
		//black background
		window_set_background_color(window, GColorBlack);

		//set sensitivity and start 
		sensitivity = 850;
    accel_data_service_subscribe(5, accel_data_handler);
    const bool animated = true;
    window_stack_push(window, animated);
		
		//update time at the start
		update_time();
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();		
    app_event_loop();
    deinit();
}
