/**
 * \file     cparser_io.h
 * \brief    Platform-dependent parser I/O routines header and definition.
 * \version  \verbatim $Id: cparser_io.h 71 2009-03-19 07:27:43Z henry $ \endverbatim
 *
 * \details  CLI parser library requires some platform-dependent API to
 *           provide access to the terminal. These API can vary greatly
 *           among different OSs.
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

#ifndef __CPARSER_IO_H__
#define __CPARSER_IO_H__

/**
 * Character type return by cparser_getch().
 */
typedef enum {
    CPARSER_CHAR_UNKNOWN = 0, /**< Character that cannot be classified */
    CPARSER_CHAR_REGULAR,     /**< All alpha-numeric + punctuation charcters */
    CPARSER_CHAR_UP_ARROW,    /**< Up arrow (recall previous command) */
    CPARSER_CHAR_DOWN_ARROW,  /**< Down arrow (recall next command) */
    CPARSER_CHAR_LEFT_ARROW,  /**< Left arrow (prev. character in the command) */
    CPARSER_CHAR_RIGHT_ARROW, /**< Right arrow (next character in the command) */
    CPARSER_MAX_CHAR
} cparser_char_t;

/**
 * Initialize I/O interface to the parser.
 *
 * \param    parser Pointer to the parser structure.
 */
void cparser_io_init(cparser_t *parser);

/**
 * Cleanup I/O interface to the parser.
 *
 * \param    parser Pointer to the parser structure.
 */
void cparser_io_cleanup(cparser_t *parser);

/**
 * Get a character input from the keyboard.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \retval   ch   Pointer to the returned character code.
 * \retval   type Type of the returned character. If it is CPARSER_CHAR_REGULAR,
 *                *ch holds the ASCII code.
 * \return   None.
 */
void cparser_getch(cparser_t *parser, int *ch, cparser_char_t *type);

/**
 * Print a single character to the output file descriptor.
 *
 * \param   parser Pointer to the parser structure.
 * \param   ch     Character to be printed.
 */
void cparser_putc(const cparser_t *parser, const char ch);

/**
 * Print a character string to the output file descriptor.
 *
 * \param   parser Pointer to the parser structure.
 * \param   s      Pointer to the string to be printed.
 */
void cparser_puts(const cparser_t *parser, const char *s);

#endif /* __CPARSER_IO_H__ */
