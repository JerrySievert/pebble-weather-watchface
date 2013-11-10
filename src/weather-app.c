#include <pebble.h>

static Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_temp_layer;

#define NUMBER_OF_IMAGES 11
static GBitmap *image = NULL;
static BitmapLayer *image_layer;

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_CLEAR_DAY,
  RESOURCE_ID_CLEAR_NIGHT,
  RESOURCE_ID_CLOUDY,
  RESOURCE_ID_FOG,
  RESOURCE_ID_PARTLY_CLOUDY_DAY,
  RESOURCE_ID_PARTLY_CLOUDY_NIGHT,
  RESOURCE_ID_RAIN,
  RESOURCE_ID_SLEET,
  RESOURCE_ID_SNOW,
  RESOURCE_ID_WIND,
  RESOURCE_ID_ERROR
};

enum {
  WEATHER_TEMPERATURE_F,
  WEATHER_TEMPERATURE_C,
  WEATHER_ICON,
  WEATHER_ERROR,
  LOCATION
};



void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  Tuple *temperature = dict_find(received, WEATHER_TEMPERATURE_F);
  Tuple *icon = dict_find(received, WEATHER_ICON);

  if (temperature) {
    text_layer_set_text(text_temp_layer, temperature->value->cstring);
  }

  if (icon) {
    // figure out which resource to use
    int8_t id = icon->value->int8;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "icon length %d (%d)", icon->length, icon->value->int8);
    if (image != NULL) {
      gbitmap_destroy(image);
      layer_remove_from_parent(bitmap_layer_get_layer(image_layer));
    }

    Layer *window_layer = window_get_root_layer(window);

    image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[id]);
    image_layer = bitmap_layer_create(GRect(10, 92, 60, 60));
    bitmap_layer_set_bitmap(image_layer, image);
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  }
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  // create time layer - this is where time goes
  text_time_layer = text_layer_create(GRect(7, 0, 144-7, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  // create date layer - this is where the date goes
  text_date_layer = text_layer_create(GRect(8, 60, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  // create temperature layer - this is where the temperature goes
  text_temp_layer = text_layer_create(GRect(80, 108, 144-80, 168-108));
  text_layer_set_text_color(text_temp_layer, GColorWhite);
  text_layer_set_background_color(text_temp_layer, GColorClear);
  text_layer_set_font(text_temp_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_39)));
  layer_add_child(window_layer, text_layer_get_layer(text_temp_layer));

}

static void window_unload(Window *window) {
  // destroy the text layers - this is good
  text_layer_destroy(text_date_layer);
  text_layer_destroy(text_time_layer);

  // destroy the image layers
  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);

  // destroy the window
  window_destroy(window);
}

static void app_message_init(void) {
  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  // Init buffers
  app_message_open(64, 16);
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
}

// show the date and time every minute
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}

static void init(void) {
  window = window_create();
  app_message_init();

  window_set_background_color(window, GColorBlack);

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  // subscribe to update every minute
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();

  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
