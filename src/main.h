#pragma once

#define AMOUNT_OF_QUESTIONS 14

typedef struct {
    char question_text[1][24];
    char watchtoken[1][30];
    char answers[4][48];
    uint8_t amount_of_answers;
} Question;

typedef enum {
    REQUEST_TYPE_SEND_QUESTION = 10,
    REQUEST_TYPE_GET_QUESTIONS = 20,
    REQUEST_TYPE_SUBMIT_ANSWER = 30
} RequestType;

void menu_cfg_provider(void *context);