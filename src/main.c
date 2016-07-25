#include <pebble.h>
#include "main.h"

#define MENU_NUM_ITEMS 4
#define MENU_NUM_SECTIONS 1


Window *title;
Window *menu;
Window *search;
Window *questions;
Window *answers;
Window *help;

BitmapLayer *title_layer;
GBitmap *title_bitmap;
BitmapLayer *search_layer;
GBitmap *search_bitmap;

SimpleMenuLayer *main_menu_layer;
SimpleMenuItem main_menu_items[MENU_NUM_ITEMS];
SimpleMenuSection main_menu_sections[MENU_NUM_SECTIONS];

TextLayer *menu_text;
TextLayer *select_text;
TextLayer *dictation_text;
TextLayer *help_text;

DictationSession *dictation_session;

int selected = 1;
int height = 10;
static char last_text[512];


char *get_readable_dictation_status(DictationSessionStatus status)
{
    switch(status)
		{
        case DictationSessionStatusSuccess:
            return "Success";
        case DictationSessionStatusFailureTranscriptionRejected:
            return "User rejected success";
        case DictationSessionStatusFailureTranscriptionRejectedWithError:
            return "User rejected error";
        case DictationSessionStatusFailureSystemAborted:
            return "Too many errors, UI gave up";
        case DictationSessionStatusFailureNoSpeechDetected:
            return "No speech, UI exited";
        case DictationSessionStatusFailureConnectivityError:
            return "No BT/internet connection";
        case DictationSessionStatusFailureDisabled:
            return "Voice dictation disabled";
        case DictationSessionStatusFailureInternalError:
            return "Internal error";
        case DictationSessionStatusFailureRecognizerError:
            return "Failed to transcribe speech";
    }
    return "Unknown";
}

void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) 
{
    //It checks if it's all good and in the clear
    if(status == DictationSessionStatusSuccess) 
		{
        //Prints the transcription into the buffer
        snprintf(last_text, sizeof(last_text), "%s", transcription);
        //Sets it onto the text layer
        text_layer_set_text(dictation_text, last_text);
		}
		else
		{
        static char failed_buff[128];
        //Prints why to the failed buffer
        snprintf(failed_buff, sizeof(failed_buff), "Transcription failed because:\n%s", get_readable_dictation_status(status));
        //Sets it onto the text layer
        text_layer_set_text(dictation_text, failed_buff);
    }
}
	
static void search_select_callback(int index, void *ctx)
{
	window_stack_push(search, true);
	dictation_session = dictation_session_create(sizeof(last_text), dictation_session_callback, NULL);
  dictation_session_start(dictation_session);
}

static void help_select_callback(int index, void *ctx)
{
	window_stack_push(help, true);
}

void menu_load(Window *menu)
{
	int num_menu_items = 0;
	
	main_menu_items[num_menu_items++] = (SimpleMenuItem) {
		.title = "Ask",
		.callback = search_select_callback,
	};
	
	main_menu_items[num_menu_items++] = (SimpleMenuItem) {
		.title = "Answers",
		//.callback = //callback to what you want this item to do when clicked
	};
	
	main_menu_items[num_menu_items++] = (SimpleMenuItem) {
		.title = "Questions",
		//.callback = //callback to what you want this item to do when clicked
	};
	
	main_menu_items[num_menu_items++] = (SimpleMenuItem) {
		.title = "Help",
		.callback = help_select_callback,
	};
	
	main_menu_sections[0] = (SimpleMenuSection) {
		.num_items = MENU_NUM_ITEMS,
		.items = main_menu_items
	};
	
	GRect bounds = layer_get_bounds(window_get_root_layer(menu));
	main_menu_layer = simple_menu_layer_create(bounds, menu, main_menu_sections, MENU_NUM_SECTIONS, NULL);
	
	layer_add_child(window_get_root_layer(menu), simple_menu_layer_get_layer(main_menu_layer));
}

void menu_unload(Window *menu)
{
	text_layer_destroy(menu_text);
	text_layer_destroy(select_text);
}

void title_load(Window *title)
{
	title_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REDDIT_TITLE_IMAGE);
	title_layer = bitmap_layer_create(GRect(0, 0, 144, 180));
	bitmap_layer_set_bitmap(title_layer, title_bitmap);
	layer_add_child(window_get_root_layer(title), bitmap_layer_get_layer(title_layer));
}

void title_unload(Window *title)
{
	bitmap_layer_destroy(title_layer);
}

void title_click_handler(ClickRecognizerRef recognizer, void *context)
{
	window_stack_push(menu, true);
}

void menu_select_handler(ClickRecognizerRef recognizer, void *context)
{
	
}

void title_cfg_provider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_DOWN, title_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, title_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, title_click_handler);
}

void app_init()
{
	window_set_click_config_provider(title, title_cfg_provider);
}

void search_load(Window *search)
{
	search_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIC_ICON);
	search_layer = bitmap_layer_create(GRect(0,0,144,180));
	bitmap_layer_set_bitmap(search_layer, search_bitmap);
	layer_add_child(window_get_root_layer(search), bitmap_layer_get_layer(search_layer));
	
	dictation_text = text_layer_create(GRect(10,10,144,168));
	text_layer_set_text(dictation_text, last_text);
	text_layer_set_text_color(dictation_text, GColorBlack);
	layer_add_child(window_get_root_layer(search), text_layer_get_layer(dictation_text));
}

void search_unload(Window *search)
{
	bitmap_layer_destroy(search_layer);
	text_layer_destroy(dictation_text);
}

void answers_load(Window *search)
{
	
}

void answers_unload(Window *search)
{
	
}

void questions_load(Window *search)
{
	
}

void questions_unload(Window *search)
{
	
}

void help_load(Window *search)
{
	help_text = text_layer_create(GRect(0,0,144,180));
	text_layer_set_text(help_text, "Instructions:\n\nSelect either ANSWER\nor QUESTION on the\nmenu using the\nup and down button.\n\nUse your voice\nto ask or\nanswer questions.");
	text_layer_set_text_color(help_text, GColorBlack);
	layer_add_child(window_get_root_layer(help), text_layer_get_layer(help_text));
}

void help_unload(Window *search)
{
	text_layer_destroy(help_text);
}

int main()
{
	title = window_create();
	menu = window_create();
	search = window_create();
	answers = window_create();
	questions = window_create();
	help = window_create();
	
	app_init();
	
	window_set_window_handlers(title, (WindowHandlers){.load = title_load, .unload = title_unload});
	window_set_window_handlers(menu, (WindowHandlers){.load = menu_load, .unload = menu_unload});
	window_set_window_handlers(search, (WindowHandlers){.load = search_load, .unload = search_unload});
	window_set_window_handlers(answers, (WindowHandlers){.load = answers_load, .unload = answers_unload});
	window_set_window_handlers(questions, (WindowHandlers){.load = questions_load, .unload = questions_unload});
	window_set_window_handlers(help, (WindowHandlers){.load = help_load, .unload = help_unload});
	
	window_stack_push(title, true);
	
	app_event_loop();
}