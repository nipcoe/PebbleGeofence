#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
char message[100];
char* name;

//Pebble app for geofencing

// Handler for select button, does nothing
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

// Sends 1, with device name when we click up to the app
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  Tuplet value = TupletCString(1, name);
  Tuplet on = TupletInteger(2, 1);
  dict_write_tuplet(iter, &value);
  dict_write_tuplet(iter, &on);

  app_message_outbox_send();

  text_layer_set_text(text_layer, name);
}

// Sends 0, with device name when we click down
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet value = TupletCString(1, name);
  Tuplet off = TupletInteger(2, 0);
  dict_write_tuplet(iter, &value);
  dict_write_tuplet(iter, &off);
  
  app_message_outbox_send();
  
  text_layer_set_text(text_layer, name);
}

// Sets handlers for buttons
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

// Loads initial window. Just a sign that says waiting
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 60 } });
  text_layer_set_text(text_layer, "Waiting for instructions");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

// Hanlders for sending messages and if they fail. Does nothing
 void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }

// If we receive a message from the app this handles it. We get the name of the device, and location
// Then we display if we would like to turn on the device or not.
 void in_received_handler(DictionaryIterator *received, void *context) {
   Tuple *in_tuple_name = dict_find(received, 1);
   Tuple *in_tuple_app = dict_find(received, 2);
   strcpy(message, "");
   if(in_tuple_app){
     strcat(message, "Turn on ");
     strcat(message, in_tuple_app->value->cstring);
     strcat(message, " in ");
   }
   
   if(in_tuple_name){
     strcat(message, in_tuple_name->value->cstring);
     strcat(message, "?");
     name = in_tuple_name->value->cstring;
   }
   text_layer_set_text(text_layer, message);
}


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }


static void init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 64;
  const uint32_t outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
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
