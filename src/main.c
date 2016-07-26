#include <pebble.h>
#include "main.h"
#include "questionslayer.h"

#define MENU_NUM_ITEMS 4
#define MENU_NUM_SECTIONS 1

void setup_menu_layer(Window *questionsWindow);

Window *title;
Window *menu;
Window *search;
Window *questionsWindow;
Window *answers;
Window *help;

BitmapLayer *title_layer;
GBitmap *title_bitmap;
BitmapLayer *search_layer;
GBitmap *search_bitmap;

SimpleMenuLayer *main_menu_layer;
MenuLayer *questions_layer;
SimpleMenuItem main_menu_items[MENU_NUM_ITEMS];
SimpleMenuSection main_menu_sections[MENU_NUM_SECTIONS];

TextLayer *menu_text;
TextLayer *select_text;
TextLayer *dictation_text;
TextLayer *help_text;

DictationSession *dictation_session;

int selected = 1;
int height = 10;
int answer_row_pushed;
static char last_text[512];

Question questions[AMOUNT_OF_QUESTIONS];
Question currentQuestion;
uint8_t questionStackCount = 0;
uint8_t currentQuestionLoaded = 0;

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

void sendQuestion(char *question)
{
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is NULL, refusing to send message.");
		return;
	}

	dict_write_uint16(iter, MESSAGE_KEY_requestType, REQUEST_TYPE_SEND_QUESTION);
	dict_write_cstring(iter, MESSAGE_KEY_questionText, question);
	dict_write_end(iter);
	
	app_message_outbox_send();
}

bool isMain = false;

void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context)
{
    //It checks if it's all good and in the clear
    if(status == DictationSessionStatusSuccess)
		{
			if(isMain){
				APP_LOG(APP_LOG_LEVEL_INFO, "got in main %s", transcription);
					//Prints the transcription into the buffer
					snprintf(last_text, sizeof(last_text), "%s", transcription);
					//Sets it onto the text layer
					sendQuestion(last_text);
			}
			else{
				APP_LOG(APP_LOG_LEVEL_INFO, "Main issue rejected");
			}
		}
		else
		{
        
    }
	isMain = false;
}



static void search_select_callback(int index, void *ctx)
{
	isMain = true;
	dictation_session = dictation_session_create(sizeof(last_text), dictation_session_callback, NULL);
  dictation_session_start(dictation_session);
}

static void questions_select_callback(int index, void *ctx)
{
	setup_menu_layer(questionsWindow);
	window_stack_push(questionsWindow, true);

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
		.title = "Questions",
		.callback = questions_select_callback,
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

void splash_dismiss()
{
	window_stack_push(menu, true);
	window_stack_remove(title, true);
}

void title_load(Window *title)
{
	title_bitmap = gbitmap_create_with_resource(RESOURCE_ID_COIN_FLIP);
	title_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(title_layer, title_bitmap);
	layer_add_child(window_get_root_layer(title), bitmap_layer_get_layer(title_layer));
	
	app_timer_register(1500, splash_dismiss, NULL);
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
	window_stack_push(answers, true);
}

void title_cfg_provider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_DOWN, title_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, title_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, title_click_handler);
}

void process_tuple(Tuple *t){
    uint32_t key = t->key;
    int value = t->value->int32;

    if(key == MESSAGE_KEY_requestType){
        switch(value){
            //Clear the questions
            case REQUEST_TYPE_GET_QUESTIONS:
                for(int i = 0; i < AMOUNT_OF_QUESTIONS; i++){
                    questions[i] = (Question){
                        .amount_of_answers = 0
                    };
                }
								questionStackCount = 0;
                break;
        }
    }

    /*
    Everything below this is copying the data to the currentQuestion struct for prep
    to go into the questions array
    */

		else if(key == MESSAGE_KEY_questionID){
        strncpy(currentQuestion.id[0], t->value->cstring, sizeof(currentQuestion.id[0]));
    }
	
    else if(key == MESSAGE_KEY_questionText){
        strncpy(currentQuestion.question_text[0], t->value->cstring, sizeof(currentQuestion.question_text[0]));
    }
    else if(key == MESSAGE_KEY_questionAnswer0){
        if(strcmp(t->value->cstring, "") != 0){
            strncpy(currentQuestion.answers[0], t->value->cstring, sizeof(currentQuestion.answers[0]));
        }
        currentQuestion.amount_of_answers++;
    }
    else if(key == MESSAGE_KEY_questionAnswer1){
        if(strcmp(t->value->cstring, "") != 0){
            strncpy(currentQuestion.answers[1], t->value->cstring, sizeof(currentQuestion.answers[1]));
        }
        currentQuestion.amount_of_answers++;
    }
    else if(key == MESSAGE_KEY_questionAnswer2){
        if(strcmp(t->value->cstring, "") != 0){
            strncpy(currentQuestion.answers[2], t->value->cstring, sizeof(currentQuestion.answers[2]));
        }
        currentQuestion.amount_of_answers++;
    }
    else if(key == MESSAGE_KEY_questionAnswer3){
        if(strcmp(t->value->cstring, "") != 0){
            strncpy(currentQuestion.answers[3], t->value->cstring, sizeof(currentQuestion.answers[3]));
        }
        currentQuestion.amount_of_answers++;
    }
    else if(key == MESSAGE_KEY_watchtoken){
        strncpy(currentQuestion.watchtoken[0], t->value->cstring, sizeof(currentQuestion.watchtoken[0]));
    }
    else if(key == MESSAGE_KEY_questionSubmitResult){
        bool successfulSubmission = (value == 1); //The question was submitted successfully
        if(!successfulSubmission){
            //Some error warning here
        }
    }
    else if(key == MESSAGE_KEY_answerSubmitResult){
        bool successfulSubmission = (value == 1); //The answer was submitted successfully
        if(!successfulSubmission){
            //Some error warning here
        }
    }
}

//When a message is received from the phone
void appmessage_inbox(DictionaryIterator *iter, void *context){
    APP_LOG(APP_LOG_LEVEL_INFO, "Got message from the phone.");

    Tuple *t = dict_read_first(iter);
    if(t){
        process_tuple(t);
    }
    while(t != NULL){
        t = dict_read_next(iter);
        if(t){
            process_tuple(t);
        }
    }

    //It was a question and not any other type of message
    if(strcmp(currentQuestion.question_text[0], "") != 0){
        questions[questionStackCount] = currentQuestion;
        questionStackCount++;
				currentQuestion = (Question){
					.amount_of_answers = 0
			};
    }
	if(questions_layer){
		menu_layer_reload_data(questions_layer);
	}
	menu_setup(questions[answer_row_pushed]);
}

void app_init()
{
	window_set_click_config_provider(title, title_cfg_provider);

    app_message_open(512, 512);
    app_message_register_inbox_received(appmessage_inbox);
}

void search_load(Window *search)
{
}

void search_unload(Window *search)
{
}

void answers_load(Window *search)
{
	setup_answer_layer(answers);
	menu_setup(questions[answer_row_pushed]);
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

uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
	return 1;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
    switch(section_index)
		{
			case 0:
				return questionStackCount;
			default:
				return 0;
		}
}

int16_t menu_get_header_height_callback(MenuLayer *questions_layer, uint16_t section_index, void *data)
{
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
    switch(section_index)
		{
			case 0:
				menu_cell_basic_header_draw(ctx, cell_layer, "Questions");
				break;
		}
}

void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
    switch(cell_index->section)
		{
			case 0:
				menu_cell_basic_draw(ctx, cell_layer, questions[cell_index->row].question_text[0], questions[cell_index->row].watchtoken[0], NULL);
				break;
		}
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
		answer_row_pushed = cell_index->row;
    window_stack_push(answers, true);
}

void setup_menu_layer(Window *questionsWindow)
{
	Layer *window_layer = window_get_root_layer(questionsWindow);

    questions_layer = menu_layer_create(GRect(0, 0, 144, 168));
    menu_layer_set_callbacks(questions_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });

    menu_layer_set_click_config_onto_window(questions_layer, questionsWindow);

    layer_add_child(window_layer, menu_layer_get_layer(questions_layer));
}

int main()
{
	//light_enable(true);
	title = window_create();
	menu = window_create();
	search = window_create();
	answers = window_create();
	questionsWindow = window_create();
	help = window_create();

	app_init();

	window_set_window_handlers(title, (WindowHandlers){.load = title_load, .unload = title_unload});
	window_set_window_handlers(menu, (WindowHandlers){.load = menu_load, .unload = menu_unload});
	window_set_window_handlers(search, (WindowHandlers){.load = search_load, .unload = search_unload});
	window_set_window_handlers(answers, (WindowHandlers){.load = answers_load, .unload = answers_unload});
	window_set_window_handlers(questionsWindow, (WindowHandlers){.load = questions_load, .unload = questions_unload});
	window_set_window_handlers(help, (WindowHandlers){.load = help_load, .unload = help_unload});

	window_stack_push(title, true);

	app_event_loop();
}