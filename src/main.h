#pragma once

typedef struct {
	char question_text[1][24];
	char watch_token[1][30];
	char answers[4][48];
	uint8_t amount_of_answers;
} Question;

void menu_cfg_provider(void *context);
void setup_menu_layer(Window *questions);