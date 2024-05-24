#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "codeEdit.h"

void code_edit_del() {
    FileStatus *status = opened_file_info->focus;
    CodeLine *cur_line = get_cur_code_line();
    int tail_flag = 0;

    if(cur_line == NULL) {
        return;
    }
    
    //erase line
    if(status->col == 0) {
        int len_save = 0;
        if(cur_line->prev == NULL) {
            return; // first line
        }
        // prev + cur line
        CodeLine *prev_line = cur_line->prev;
        len_save = prev_line->len;

        // tail row check
        if(status->buf->tail == cur_line) {
            tail_flag = 1;
        }

        // empty line check
        if(cur_line->len == 0) {
            if(tail_flag == 1) {
                prev_line->next = NULL;
                status->buf->tail = prev_line;
            } else {
                prev_line->next = cur_line->next;
                cur_line->next->prev = prev_line;
            }
        } else if(prev_line->len == 0) {
            CodeLine *pprev_line = prev_line->prev;
            if(pprev_line == NULL) {
                status->buf->head = status->buf->cur = cur_line;
                cur_line->prev = NULL;
            } else {
                pprev_line->next = cur_line;
                cur_line->prev = pprev_line;
            }

            // adjust
            cur_line = prev_line;
        } else {
            prev_line->next = cur_line->next;
            if(tail_flag == 0) {
                cur_line->next->prev = prev_line;
            } else {
                status->buf->tail = prev_line;
            }
            prev_line->line = realloc(prev_line->line, sizeof(char) * (prev_line->len + cur_line->len) + 1);
            strcat(prev_line->line, cur_line->line);
            prev_line->len = strlen(prev_line->line);
        }

        // adjust cursor position
        status->row--;
        status->buf->tail_row--;
        
        if(status->start_row > status->row) {
            status->start_row--;
            CodeLine *ptr = status->buf->cur;
            if(ptr->prev != NULL) {
                status->buf->cur = ptr->prev;
            }
        }
        status->col = len_save;

        if(cur_line != NULL) {
            free(cur_line->line);
            free(cur_line);
        }
    } else {
        // if cursor place one end of line
        if(status->col >= cur_line->len) {
            cur_line->line[status->col - 1] = '\0';
            cur_line->len = strlen(cur_line->line);

            status->col--;
            if(status->start_col > status->col) {
                status->start_col--;
            }
        } else {
            // erase char in a line
            fprintf(stderr, "cur:%s\n", cur_line->line);
            char *mid = &(cur_line->line[status->col]);
            cur_line->line[status->col - 1] = '\0';
            fprintf(stderr, "head:%s\n", cur_line->line);
            fprintf(stderr, "mid:%s\n", mid);
            strcat(cur_line->line, mid);
            fprintf(stderr, "cat:%s\n", cur_line->line);
            cur_line->len = strlen(cur_line->line);

            status->col--;
            if(status->start_col > status->col) {
                status->start_col--;
            }
        }
    }
}

void code_edit_char_append(int _input_char) {
    FileStatus *status = opened_file_info->focus;
    CodeLine *cur_line = get_cur_code_line();

    char input_char = (char)_input_char;

    // make new line
    int new_len = cur_line->len + 2;
    char *new_line = calloc(sizeof(char), new_len);
    strncat(new_line, cur_line->line, status->col);
    new_line[status->col] = input_char;
    new_line[status->col + 1] = '\0';
    strcat(new_line, &(cur_line->line[status->col]));

    // substitute
    free(cur_line->line);
    cur_line->line = new_line;
    cur_line->len = strlen(new_line);

    // col adjust
    status->col++;
    if(status->start_col - status->col >= win_col) {
        status->start_col++;
    }
}

void code_edit_append_new_line() {
    FileStatus *status = opened_file_info->focus;
    CodeLine *prev_line = get_cur_code_line();
    CodeLine *next_line = prev_line->next;

    char* start_pos = &(prev_line->line[status->col]);

    CodeLine *cur_line = malloc(sizeof(CodeLine));
    cur_line->len = strlen(start_pos);
    cur_line->line = malloc(sizeof(char) * (cur_line->len + 1));
    strcpy(cur_line->line, start_pos);
    prev_line->line[status->col] = '\0';
    prev_line->len = strlen(prev_line->line);

    cur_line->prev = prev_line;
    prev_line->next = cur_line;

    if(next_line == NULL) {
        status->buf->tail = cur_line;
    } else {
        cur_line->next = next_line;
        next_line->prev = cur_line;
    }

    status->buf->tail_row++;

    // row & col adjust
    status->col = 0;
    status->start_col = 0;

    status->row++;
    if(status->row - status->start_row >= win_row - 3) {
        status->start_row++;
        CodeLine *ptr = status->buf->cur;
        if(ptr->next != NULL) {
            status->buf->cur = ptr->next;
        }
    }
}