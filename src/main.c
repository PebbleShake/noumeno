#include <pebble.h>

#define sensitivity 850

static Window* window;

static TextLayer* text_layer;

static TextLayer* time_layer, *date_layer, *label_layer;

static GDrawCommandSequence *s_command_seq;

static void inviaMessaggio(){

DictionaryIterator *iter;

    int value=1;

  AppMessageResult result=app_message_outbox_begin(&iter);

  if(result==APP_MSG_OK){

    dict_write_int(iter, 1, &value, sizeof(int), true);

    dict_write_end(iter);

    
  AppMessageResult result2=app_message_outbox_send();

     APP_LOG(APP_LOG_LEVEL_ERROR, "invio fatto\nerrore outbox_send %d", (int)result2);

  }else{

    APP_LOG(APP_LOG_LEVEL_ERROR, "errore %d", (int)result);

  }

}  

static void accel_data_handler(AccelData* data, uint32_t num_samples) {

  // calculate y-axis derivative

  static char buf[128];

  int dy = data[num_samples-1].y - data[0].y;

  if (dy > sensitivity) {

    snprintf(buf, sizeof(buf),"Delta: %d\nHandshake!",dy);

        APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d\nHandshake!",dy);

      app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    inviaMessaggio();

    text_layer_set_text(text_layer, buf);

    vibes_double_pulse();

  } else {

    snprintf(buf, sizeof(buf), "Delta: %d", dy);

        APP_LOG(APP_LOG_LEVEL_DEBUG, "Delta: %d", dy);

    text_layer_set_text(text_layer, buf);

  }

}

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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

  update_time();

}

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);

    GRect bounds = layer_get_bounds(window_layer);

    text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 0, 0 } });

    layer_add_child(window_layer, text_layer_get_layer(text_layer));

  

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

static void window_unload(Window *window) {

    text_layer_destroy(time_layer);

  text_layer_destroy(date_layer);

  text_layer_destroy(text_layer);

}

static void init(void) {

    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {

            .load = window_load,

            .unload = window_unload,

            });

    accel_data_service_subscribe(5, accel_data_handler);

    const bool animated = true;

    window_stack_push(window, animated);

    update_time();

  window_set_background_color(window, GColorBlack);

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