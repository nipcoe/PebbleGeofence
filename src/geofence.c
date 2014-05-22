#include <pebble.h>

static Window *window;
MenuLayer *menu_layer;
static TextLayer *text_layer;
char message[100];
char* name;
char list[10][24];
int size = 0;

//Pebble app for geofencing

// Draws the menu list for possible actions. WHen turned on, size is 0, so doesn't display anything at first.
// Max number of actions is 10
void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context){
  //Which row is it?
  switch(cell_index->row)
    {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "1.", list[0], NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "2.", list[1], NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "3.", list[2], NULL);
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "4.", list[3], NULL);
      break;
    case 4:
      menu_cell_basic_draw(ctx, cell_layer, "5.", list[4], NULL);
      break;
    case 5:
      menu_cell_basic_draw(ctx, cell_layer, "6.", list[5], NULL);
      break;
    case 6:
      menu_cell_basic_draw(ctx, cell_layer, "7.", list[6], NULL);
      break;
    case 7:
      menu_cell_basic_draw(ctx, cell_layer, "8.", list[7], NULL);
      break;
    case 8:
      menu_cell_basic_draw(ctx, cell_layer, "9.", list[8], NULL);
      break;
    case 9:
      menu_cell_basic_draw(ctx, cell_layer, "10.", list[9], NULL);
      break;
    }
}

// Returns number of rows we have total. Number of actions for a device 
uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context){
  return size;
}

// Sends data back to the android phone, saying what the action should be.
// This is called when we click an item in the menu
void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
  //Get which row
  int which = cell_index->row;
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet value = TupletCString(1, name);
  Tuplet on = TupletCString(2, list[which]);
  dict_write_tuplet(iter, &value);
  dict_write_tuplet(iter, &on);
  
  app_message_outbox_send();
}


// Sets handlers for buttons, we do nothing here, since our buttons are handled in the menulist handlers
static void click_config_provider(void *context) {

}

// Loads initial window. Just a sign that says waiting
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 60 } });
  text_layer_set_text(text_layer, "Waiting for instructions");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

   //Create menu layer
   menu_layer = menu_layer_create(GRect(0, 50, 144, 100));

   //Let it receive clicks
   menu_layer_set_click_config_onto_window(menu_layer, window);
   
   //Give it its callbacks
   MenuLayerCallbacks callbacks = {
     .select_click = (MenuLayerSelectCallback) select_click_callback,
     .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
     .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
   };
   menu_layer_set_callbacks(menu_layer, NULL, callbacks);
   layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  menu_layer_destroy(menu_layer);
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
   Tuple *in_tuple_size = dict_find(received, 3);
   int i = 4;
   size = 0;
   if(in_tuple_size){
     size = in_tuple_size->value->uint32;
   }
   int x;
   for(x = 0; x<size; x++){
     Tuple *in_tuple_choice = dict_find(received, i+x);
     if(in_tuple_choice){
       strcpy(list[x], "");
       strcpy(list[x], in_tuple_choice->value->cstring);
     }
   }
   
   strcpy(message, "");
   if(in_tuple_app){
     strcat(message, "Use ");
     strcat(message, in_tuple_app->value->cstring);
     strcat(message, " in ");
   }
   
   if(in_tuple_name){
     strcat(message, in_tuple_name->value->cstring);
     strcat(message, "?");
     name = in_tuple_name->value->cstring;
   }
   text_layer_set_text(text_layer, message);
   
   menu_layer_reload_data(menu_layer);
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }


static void init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  const uint32_t inbound_size = 128;
  const uint32_t outbound_size = 60;
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
