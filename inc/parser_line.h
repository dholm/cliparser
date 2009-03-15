/**
 * \file     parser_line.h
 * \brief    Parser line buffer API definitions and prototypes.
 * \version  \verbatim $Id: parser_line.h 51 2009-03-12 22:33:20Z henry $ \endverbatim
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

#ifndef __PARSER_LINE_H__
#define __PARSER_LINE_H__

#include "parser_options.h"

typedef struct {
    short  last;      /**< Point to the last character in the line buffer */
    short  current;   /**< Point to the current character */
    char   buf[PARSER_MAX_LINE_SIZE+1];
} parser_line_t;

/**
 * Reset a line buffer into an "empty" state.
 *
 * \param    line Pointer to a line structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if the line 
 *           is NULL.
 */ 
parser_result_t parser_line_reset(parser_line_t *line);

/**
 * Insert a character into a line buffer at the current position.
 *
 * \param    parser Pointer to a parser structure.
 * \param    ch     Character to be inserted.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if inputs
 *           are invalid; PARSER_ERR_OUT_OF_RES if the line buffer is full.
 */
parser_result_t parser_line_insert(parser_t *parser, char ch);

/**
 * Delete a character from the line buffer immediately before
 * the current position. The current position is moved back by one.
 *
 * \param    parser Pointer to a parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if inputs
 *           are invalid; PARSER_ERR_NOT_EXIST if the line buffer is empty.
 */
parser_result_t parser_line_delete(parser_t *parser);

/**
 * Move the current position of the current line to the next character.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   A character to be fed into parser FSM; NULL if no character
 *           is to be fed.
 */
char parser_line_next_char(parser_t *parser);

/**
 * Move the current position of the current line to the previous character.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   A character to be fed into parser FSM; NULL if no character
 *           is to be fed.
 */
char parser_line_prev_char(parser_t *parser);

/**
 * Move the current line to the next line.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if inputs
 *           are invalid.
 */
parser_result_t parser_line_next_line(parser_t *parser);

/**
 * Move the current line to the previous line.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if inputs
 *           are invalid.
 */
parser_result_t parser_line_prev_line(parser_t *parser);

/**
 * Print the current line including the current line.
 *
 * \param    parser       Pointer to the parser structure.
 * \param    print_prompt 1 if the parser prompt should be printed first.
 * \param    new_line     1 if a linefeed should be printed first;
 *                        0 otherwise.
 *
 * \return   None. Crash on failure.
 */
void parser_line_print(const parser_t *parser, int print_prompt, int new_line);

/**
 * Return the current position of the current line.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   Current position of the current line.
 */
short parser_line_current(const parser_t *parser);

/**
 * Return the last position of the current line.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   Last position of the current line. This index points to
 *           the terminating NULL character of the current line.
 */
short parser_line_last(const parser_t *parser);

/**
 * Return the character of the the current position of the current line.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   Character of the current position of the current line.
 */
char parser_line_current_char(const parser_t *parser);

char parser_line_char(const parser_t *parser, short pos);

parser_result_t parser_line_advance(parser_t *parser);

#endif /* __PARSER_LINE_H__ */
