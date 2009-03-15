/**
 * \file     parser_line.c
 * \brief    Parser line buffer.
 * \version  \verbatim $Id: parser_line.c 51 2009-03-12 22:33:20Z henry $ \endverbatim
 */
/*
 * Copyright (c) 2008, Henry Kwok
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the project nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY HENRY KWOK ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL HENRY KWOK BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <string.h>
#include "parser.h"
#include "parser_priv.h"

#define LINE_ALL(line)       &((line)->buf[0])
#define LINE_CURRENT(line)   &(line)->buf[(line)->current]


parser_result_t
parser_line_reset (parser_line_t *line)
{
    if (!line) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    line->last = 0;
    line->current = 0;
    memset(line->buf, 0, sizeof(line->buf));

    return PARSER_OK;
}

parser_result_t
parser_line_insert (parser_t *parser, char ch)
{
    int n;
    parser_line_t *line;

    if (!VALID_PARSER(parser) || !ch) {
        return PARSER_ERR_INVALID_PARAMS;
    }
    line = &parser->lines[parser->cur_line];
    if (PARSER_MAX_LINE_SIZE <= line->last) {
        return PARSER_ERR_OUT_OF_RES;
    }

    /* Move all characters from current to last back by 1 */
    for (n = line->last; n > line->current; n--) {
        line->buf[n] = line->buf[n-1];
    }
    line->buf[++line->last] = '\0';

    /* 
     * Insert the new character and update the line display. We do not 
     * have full curse support here. Instead, we simply assume all 
     * characters are on the same line and use backspace to move the 
     * cursor.
     */
    line->buf[line->current] = ch;
    parser_putc(parser, ch);
    line->current++; /* update current position */
    parser_puts(parser, LINE_CURRENT(line));

    /* Move cursor back to the current position */
    for (n = line->current; n < line->last; n++) {
        parser_putc(parser, '\b');
    }
    
    return PARSER_OK;
}

parser_result_t
parser_line_delete (parser_t *parser)
{
    parser_line_t *line;
    int n;

    if (!VALID_PARSER(parser)) {
        return PARSER_ERR_INVALID_PARAMS;
    }
    line = &parser->lines[parser->cur_line];
    assert(line->current <= line->last);
    if (!line->last || !line->current) {
         /* Line is empty or we're at the beginning of the line */
        return PARSER_ERR_NOT_EXIST;
    }

    /* Move all character after current position back by one */
    for (n = line->current; n < line->last; n++) {
        line->buf[n-1] = line->buf[n];
    }
    line->current--;
    line->last--;
    line->buf[line->last] = '\0';

    /* Update the display */
    parser_putc(parser, '\b');
    parser_puts(parser, LINE_CURRENT(line));
    parser_puts(parser, " \b");
    for (n = line->current; n < line->last; n++) {
        parser_putc(parser, '\b');
    }
    return PARSER_OK;
}

void
parser_line_print (const parser_t *parser, int print_prompt, int new_line)
{
    const parser_line_t *line;
    int n;

    assert(VALID_PARSER(parser));
    if (new_line) {
        parser_putc(parser, '\n');
    }
    if (print_prompt) {
        parser_puts(parser, parser->prompt[parser->root_level]);
    }
    line = &parser->lines[parser->cur_line];
    parser_puts(parser, line->buf);

    /* Move the cursor back the current position */
    for (n = line->current; n < line->last; n++) {
        parser_putc(parser, '\b');
    }
}

short
parser_line_current (const parser_t *parser)
{
    assert(VALID_PARSER(parser));
    return parser->lines[parser->cur_line].current;
}

short
parser_line_last (const parser_t *parser)
{
    assert(VALID_PARSER(parser));
    return parser->lines[parser->cur_line].last;
}

char
parser_line_current_char (const parser_t *parser)
{
    const parser_line_t *line;
    assert(VALID_PARSER(parser));
    line = &parser->lines[parser->cur_line];
    return line->buf[line->current];
}

char
parser_line_char (const parser_t *parser, short pos)
{
    const parser_line_t *line;
    assert(VALID_PARSER(parser));
    line = &parser->lines[parser->cur_line];
    assert(pos < line->last);
    return line->buf[pos];
}

char
parser_line_next_char (parser_t *parser)
{
    char retval;
    parser_line_t *line;

    assert(VALID_PARSER(parser));
    line = &parser->lines[parser->cur_line];
    if (line->last == line->current) {
        /* Already at the end of the line */
        return 0;
    }

    retval = line->buf[line->current];
    parser_putc(parser, retval);
    line->current++;

    return retval;
}

char
parser_line_prev_char (parser_t *parser)
{
    parser_line_t *line;

    assert(VALID_PARSER(parser));
    line = &parser->lines[parser->cur_line];
    if (!line->current) {
        /* Already at the beginning of the line */
        return 0;
    }

    parser_putc(parser, '\b');
    line->current--;

    return '\b';
}

parser_result_t
parser_line_next_line (parser_t *parser)
{
    int n;

    if (!VALID_PARSER(parser)) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    /* Erase the current line */
    for (n = 0; n < parser_line_last(parser); n++) {
        parser_puts(parser, "\b \b");
    }

    /* Go to the next line */
    parser->cur_line++;
    if (parser->max_line < parser->cur_line) {
        parser->cur_line = 0;
    }

    /* Print out the new line */
    parser_line_print(parser, 0, 0);

    return PARSER_OK;
}

parser_result_t
parser_line_prev_line (parser_t *parser)
{
    int n;

    if (!VALID_PARSER(parser)) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    /* Erase the current line */
    for (n = 0; n < parser_line_last(parser); n++) {
        parser_puts(parser, "\b \b");
    }

    /* Go to the previous line */
    parser->cur_line--;
    if (0 > parser->cur_line) {
        parser->cur_line = parser->max_line;
    }

    /* Print out the new line */
    parser_line_print(parser, 0, 0);

    return PARSER_OK;
}

parser_result_t
parser_line_advance (parser_t *parser)
{
    int rc;

    if (!VALID_PARSER(parser)) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    parser->cur_line++;
    if (PARSER_MAX_LINES <= parser->cur_line) {
        parser->cur_line = 0;
    }
    if (parser->max_line < parser->cur_line) {
        parser->max_line = parser->cur_line;
    }

    rc = parser_line_reset(&parser->lines[parser->cur_line]);
    assert(0 == rc);

    return PARSER_OK;
}
