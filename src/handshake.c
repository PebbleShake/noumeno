#include <pebble.h>

//constants
#define DELTA 33 
#define sensitivity 850

static void load_animation();
static void send_message();

static Window* window;
static TextLayer* text_layer;
static TextLayer* time_layer, *date_layer;

static GDrawCommandSequence *s_command_seq;
static Layer *s_canvas_layer;
static int frame_index = 0;

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
		APP_LOG(APP_LOG_LEVEL_DEBUG, "DeltaY: %d Handshake!",dy);
		
		//send messge to andorid app
		app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
		send_message();
		
    //update text view
		text_layer_set_text(text_layer, buf);
		
		//double vibration
		vibes_double_pulse();
		
		load_animation();
  }
	//if it doesn't
	else {
    snprintf(buf, sizeof(buf), "DeltaY: %d", dy);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "DeltaY: %d", dy);
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

/**
	** ANIMATION STUFF **
**/
static void next_frame_handler(void *context) {
  // Draw the next frame
  layer_mark_dirty(s_canvas_layer);

  // Continue the sequence
  app_timer_register(DELTA, next_frame_handler, NULL);
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Get the next frame
  GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(s_command_seq, frame_index);

  // If another frame was found, draw it
  if (frame) {
    gdraw_command_frame_draw(ctx, s_command_seq, frame, GPoint(0, 30));
  }

  // Advance to the next frame, wrapping if neccessary
  int num_frames = gdraw_command_sequence_get_num_frames(s_command_seq);
  frame_index++;
	if (frame_index == num_frames) {
			//restore background color
  	  window_set_background_color(window, GColorBlack);
	}
}

static void load_animation(){
	frame_index = 0;
	window_set_background_color(window, GColorWhite);
	
  // Set the LayerUpdateProc
 	layer_set_update_proc(s_canvas_layer, update_proc);
	// Add to parent Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
	
}

/*
	send string to android app
*/
static void send_message(){
 
	DictionaryIterator *iter;

	AppMessageResult result=app_message_outbox_begin(&iter);

	if(result==APP_MSG_OK){
		dict_write_cstring(iter, 1, "ciao");
		dict_write_end(iter);

		AppMessageResult result2=app_message_outbox_send();
		APP_LOG(APP_LOG_LEVEL_ERROR, "invio fatto\nerrore outbox_send %d", (int)result2);
	}
	else{
		APP_LOG(APP_LOG_LEVEL_ERROR, "errore %d", (int)result);
	}
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
	
		// Create the canvas Layer
  	s_canvas_layer = layer_create(GRect(30, 20, bounds.size.w, bounds.size.h));
}

/*
	destroy window compunents when done
*/
static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
		text_layer_destroy(time_layer);
		text_layer_destroy(date_layer);
	
		//destroy animation
		gdraw_command_sequence_destroy(s_command_seq);
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

		//start 
    accel_data_service_subscribe(5, accel_data_handler);
    const bool animated = true;
	
		//init animation
		s_command_seq = gdraw_command_sequence_create_with_resource(RESOURCE_ID_CONFIRM_SEQUENCE);
	
		//start
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