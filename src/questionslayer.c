#include <pebble.h>
#include "questionslayer.h"

MenuLayer *answer_layer;
Question menuQuestion;

uint16_t answer_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
	if(menuQuestion.amount_of_answers == 4)
	{
		return 1;
	}
	return 2;
}

uint16_t answer_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
    switch(section_index)
		{
			case 0:
				return menuQuestion.amount_of_answers;
			case 1:
				return 1;
			default:
				return 0;
		}
}

int16_t answer_get_header_height_callback(MenuLayer *questions_layer, uint16_t section_index, void *data)
{
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void answer_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
    switch(section_index)
		{
			case 0:
				menu_cell_basic_header_draw(ctx, cell_layer, menuQuestion.question_text[0]);
				break;
			case 1:
				menu_cell_basic_header_draw(ctx, cell_layer, "Post Options");
				break;
		}
}

void answer_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
    switch(cell_index->section)
		{
			case 0:
				menu_cell_basic_draw(ctx, cell_layer, menuQuestion.answers[cell_index->row], NULL, NULL);
				break;
			case 1:
				menu_cell_basic_draw(ctx, cell_layer, "Asnwer Question", NULL, NULL);
				break;
		}
}

void sendAnswer(char *answer)
{
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is NULL, refusing to send message.");
		return;
	}
	
	APP_LOG(APP_LOG_LEVEL_ERROR, "%d hi", REQUEST_TYPE_SUBMIT_ANSWER);

	dict_write_uint16(iter, MESSAGE_KEY_requestType, REQUEST_TYPE_SUBMIT_ANSWER);
	dict_write_cstring(iter, MESSAGE_KEY_answerText, answer);
	dict_write_cstring(iter, MESSAGE_KEY_questionID, menuQuestion.id[0]);
	dict_write_end(iter);
	
	app_message_outbox_send();
}

void answer_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context)
{
    //It checks if it's all good and in the clear
    if(status == DictationSessionStatusSuccess)
		{
			APP_LOG(APP_LOG_LEVEL_INFO, "got success %s", transcription);
			sendAnswer(transcription);
			window_stack_pop(true);
		}
		else
		{
    }
}


void answer_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
    if(cell_index->section == 1 && cell_index->row == 0)
		{
			APP_LOG(APP_LOG_LEVEL_INFO, "got stuff");
			DictationSession *answerSession = dictation_session_create(512, answer_session_callback, NULL);
  		dictation_session_start(answerSession);
		}
}

void menu_setup(Question question)
{
	menuQuestion = question;
	if(answer_layer){
		menu_layer_reload_data(answer_layer);
	}
}

void setup_answer_layer(Window *answerWindow)
{
	Layer *window_layer = window_get_root_layer(answerWindow);

    answer_layer = menu_layer_create(GRect(0, 0, 144, 168));
    menu_layer_set_callbacks(answer_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = answer_get_num_sections_callback,
        .get_num_rows = answer_get_num_rows_callback,
        .get_header_height = answer_get_header_height_callback,
        .draw_header = answer_draw_header_callback,
        .draw_row = answer_draw_row_callback,
        .select_click = answer_select_callback,
    });

    menu_layer_set_click_config_onto_window(answer_layer, answerWindow);

    layer_add_child(window_layer, menu_layer_get_layer(answer_layer));
}
