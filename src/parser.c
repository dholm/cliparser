/**
 * \file     parser.c
 * \brief    parser top-level API
 * \version  \verbatim $Id: parser.c 51 2009-03-12 22:33:20Z henry $ \endverbatim
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
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "parser.h"
#include "parser_priv.h"
#include "parser_token.h"
#include "parser_io.h"
#include "parser_fsm.h"

static void
parser_help_print_node (parser_t *parser, parser_node_t *node)
{
    assert(parser && node);
    parser_putc(parser, '\n');
    switch (node->type) {
        case PARSER_NODE_ROOT:
            assert(0); /* this should never happen */
        case PARSER_NODE_END:
            parser_puts(parser, "<LF>");
            break;
        default:
            parser_puts(parser, node->param);
            if (node->desc) {
                parser_puts(parser, " - ");
                parser_puts(parser, node->desc);
            }
            break;
    }
}

static parser_result_t
parser_execute_cmd (parser_t *parser)
{
    int n, m;

    assert(VALID_PARSER(parser));

    /* 
     * Enter a command. There are three possibilites:
     * 1. If we are in WHITESPACE state, we check if there is
     *    only one child of keyword type. If yes, we recurse 
     *    into it and repeat until either: a) there is more than
     *    one choice, b) We are at a END node. If there is more
     *    than one choice, we look for an END node. In either
     *    case, if an END node is found, we execute the action
     *    function.
     *
     * 2. If we are in TOKEN state, we check if we have an unique
     *    match. If yes, re recurse into it and repeat just
     *    like WHITESPACE state until we find an END node.
     *
     * 3. If we are in ERROR state, we print out an error.
     *
     * Afterward, we reset the parser state and move to the
     * next line buffer.
     */
    if ((PARSER_STATE_TOKEN == parser->state) ||
        (PARSER_STATE_WHITESPACE == parser->state)) {
        parser_node_t *child;
        parser_result_t rc;

        if (PARSER_STATE_TOKEN == parser->state) {
            parser_token_t *token;
            parser_node_t *match;
            int is_complete;

            token = CUR_TOKEN(parser);
            if ((1 <= parser_match(token->buf, token->token_len,
                                   parser->cur_node, &match, 
                                   &is_complete)) &&
                (is_complete)) {
                token->parent = parser->cur_node;
                token->buf[token->token_len] = '\0';

                parser->token_tos++;
                assert(PARSER_MAX_NUM_TOKENS > parser->token_tos);
                memset(CUR_TOKEN(parser), 0, sizeof(parser_token_t));

                parser->last_good = parser_line_current(parser);
                parser->cur_node = match;
            } else {
                parser_putc(parser, '\n');
                m = strlen(parser->prompt[parser->root_level]);
                for (n = 0; n < m+parser->last_good; n++) {
                    parser_putc(parser, ' ');
                }
                parser_putc(parser, '^');
                parser_puts(parser, "Incomplete command\n");
                    
                /* Reset the internal buffer, state and cur_node */
                parser_fsm_reset(parser);
                parser_puts(parser, parser->prompt[parser->root_level]);
                return PARSER_OK;
            }
        }

        /* Look for a single keyword node child */
        child = parser->cur_node->children;
        assert(child);
        while ((PARSER_NODE_KEYWORD == child->type) &&
               (!child->sibling)) {
            parser->cur_node = child;
            child = parser->cur_node->children;
            assert(child);
        }

        /* Look for an end node */
        child = parser->cur_node->children;
        while ((NULL != child) && (PARSER_NODE_END != child->type)) {
            child = child->sibling;
        }
        if (child) {
            assert(PARSER_NODE_END == child->type);

            /* Execute the glue function */
            parser->cur_node = child;
            parser_putc(parser, '\n');
            rc = ((parser_glue_fn)child->param)(parser);
        } else {
            if (parser->token_tos) {
                parser_putc(parser, '\n');
                m = strlen(parser->prompt[parser->root_level]);
                for (n = 0; n < m+parser->last_good; n++) {
                    parser_putc(parser, ' ');
                }
                parser_putc(parser, '^');
                parser_puts(parser, "Incomplete command\n");
            }
                    
            /* Reset FSM states and advance to the next line */
            parser_fsm_reset(parser);
            parser_line_advance(parser);
            parser_putc(parser, '\n');
            parser_puts(parser, parser->prompt[parser->root_level]);
            return PARSER_OK;
        }
    } else if (PARSER_STATE_ERROR == parser->state) {
        parser_putc(parser, '\n');
        m = strlen(parser->prompt[parser->root_level]);
        for (n = 0; n < m+parser->last_good; n++) {
            parser_putc(parser, ' ');
        }
        parser_putc(parser, '^');
        parser_puts(parser, "Parse error\n");
    }

    /* Reset FSM states and advance to the next line */
    parser_fsm_reset(parser);
    parser_line_advance(parser);
    parser_puts(parser, parser->prompt[parser->root_level]);
    return PARSER_OK;
}

parser_result_t
parser_help (parser_t *parser)
{
    parser_node_t *node, *match;
    parser_token_t *token;
    int local_is_complete;

    assert(VALID_PARSER(parser));
    if (PARSER_STATE_WHITESPACE == parser->state) {
        /* Just print out every children */
        for (node = parser->cur_node->children; NULL != node;
             node = node->sibling) {
            parser_help_print_node(parser, node);
        }
    } else {
        /* We have a partial match */
        node = parser->cur_node->children;
        match = NULL;
        token = CUR_TOKEN(parser);
        for (node = parser->cur_node->children; NULL != node; 
             node = node->sibling) {
            if (PARSER_OK == 
                parser_match_fn_tbl[node->type](token->buf, token->token_len, 
                                                node, &local_is_complete)) {
                parser_help_print_node(parser, node);
            }
        }
    }
    parser_line_print(parser, 1, 1);
    return PARSER_OK;
}

parser_result_t
parser_input (parser_t *parser, char ch, parser_char_t ch_type)
{
    int n;
    parser_token_t *token;
    parser_result_t rc;

    if (!VALID_PARSER(parser)) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    switch (ch_type) {
        case PARSER_CHAR_REGULAR:
        {
            if ((parser->cfg.ch_complete == ch) || 
                (parser->cfg.ch_help == ch)) {
                /* 
                 * Completion and help character do not go into the line
                 * buffer. So, do nothing.
                 */
                break;
            }
            if ((parser->cfg.ch_erase == ch) || (parser->cfg.ch_del == ch)) {
                rc = parser_line_delete(parser);
                assert(PARSER_ERR_INVALID_PARAMS != rc);
                if (PARSER_ERR_NOT_EXIST == rc) {
                    return PARSER_OK;
                }
            } else if ('\n' == ch) {
                /* Put the rest of the line into parser FSM */
                for (n = parser_line_current(parser); 
                     n < parser_line_last(parser); n++) {
                    rc = parser_fsm_input(parser, parser_line_char(parser, n));
                    assert(PARSER_OK == rc);
                }
            } else {
                (void)parser_line_insert(parser, ch);
            }
            break;
        }
        case PARSER_CHAR_UP_ARROW:
        {
            rc = parser_line_prev_line(parser);
            assert(PARSER_OK == rc);

            /* Reset the token stack and re-enter the command */
            parser_fsm_reset(parser);
            for (n = 0; n < parser_line_current(parser); n++) {
                rc = parser_fsm_input(parser, parser_line_char(parser, n));
                assert(PARSER_OK == rc);
            }
            
            return PARSER_OK;
        }
        case PARSER_CHAR_DOWN_ARROW:
        {
            rc = parser_line_next_line(parser);
            assert(PARSER_OK == rc);

            /* Reset the token stack and re-enter the command */
            parser_fsm_reset(parser);
            for (n = 0; n < parser_line_current(parser); n++) {
                rc = parser_fsm_input(parser, parser_line_char(parser, n));
                assert(PARSER_OK == rc);
            }
            
            return PARSER_OK;
        }
        case PARSER_CHAR_LEFT_ARROW:
        {
            ch = parser_line_prev_char(parser);
            if (!ch) {
                parser_putc(parser, '\a');
                return PARSER_OK;
            }
            break;
        }
        case PARSER_CHAR_RIGHT_ARROW:
        {
            ch = parser_line_next_char(parser);
            if (!ch) {
                parser_putc(parser, '\a');
                return PARSER_OK;
            }
            break;
        }
        default:
        {
            /* An unknown character. Alert and continue */
            parser_putc(parser, '\a');
            return PARSER_NOT_OK;
        }
    } /* switch (ch_type) */

    /* Handle special characters */
    if (ch == parser->cfg.ch_complete) {
        parser_node_t *match;
        int is_complete = 0, num_matches;
        char *ch_ptr;

        switch (parser->state) {
            case PARSER_STATE_ERROR:
                /* If we are in ERROR, there cannot be a match. So, just quit */
                parser_putc(parser, '\a');
                break;
            case PARSER_STATE_WHITESPACE:
                /* 
                 * If we are in WHITESPACE, just dump all children. Since there is no
                 * way any token can match to a NULL string.
                 */
                parser_help(parser);
                break;
            case PARSER_STATE_TOKEN:
            {
                /* Complete a command */
                token = CUR_TOKEN(parser);
                num_matches = parser_match(token->buf, token->token_len,
                                           parser->cur_node, &match, 
                                           &is_complete);
                if ((1 == num_matches) && (is_complete)) {
                    /*
                     * If the only matched node is a keyword, we feel the rest of
                     * keyword in. Otherwise, we assume this parameter is complete
                     * and just insert a space.
                     */
                    if (PARSER_NODE_KEYWORD == match->type) {
                        ch_ptr = match->param + token->token_len;
                        while (*ch_ptr) {
                            rc = parser_input(parser, *ch_ptr, PARSER_CHAR_REGULAR);
                            assert(PARSER_OK == rc);
                            ch_ptr++;
                        }
                    }
                    rc = parser_input(parser, ' ', PARSER_CHAR_REGULAR);
                    assert(PARSER_OK == rc);
                } else {
                    parser_help(parser);
                }
                break;
            }
            default: assert(0);
        }
        return PARSER_OK;
    } else if (ch == parser->cfg.ch_help) {
        /* Ask for context sensitve help */
        parser_help(parser);
        return PARSER_OK;
    } else if ('\n' == ch) {
        return parser_execute_cmd(parser);
    }

    return parser_fsm_input(parser, (char)ch);
}

parser_result_t
parser_run (parser_t *parser)
{
    int ch;
    parser_char_t ch_type = 0;

    if (!VALID_PARSER(parser)) return PARSER_ERR_INVALID_PARAMS;

    parser_io_init(parser);
    parser_puts(parser, parser->prompt[parser->root_level]);
    parser->done = 0;

    while (!parser->done) {
        parser_getch(parser, &ch, &ch_type);
        parser_input(parser, ch, ch_type);
    } /* while not done */

    parser_io_cleanup(parser);

    return PARSER_OK;
}

parser_result_t 
parser_init (parser_cfg_t *cfg, parser_t *parser)
{
    int n;

    if (!parser || !cfg || !cfg->root || !cfg->ch_erase) {
	return PARSER_ERR_INVALID_PARAMS;
    }

    parser->cfg = *cfg;
    parser->cfg.prompt[PARSER_MAX_PROMPT-1] = '\0';

    /* Initialize sub-mode states */
    parser->root_level = 0;
    parser->root[0] = parser->cfg.root;
    strcpy(parser->prompt[0], parser->cfg.prompt);
    for (n = 0; n < PARSER_MAX_COOKIES; n++) {
        parser->context.cookie[n] = NULL;
    }
    parser->context.parser = parser;

    /* Initialize line buffering states */
    parser->max_line = 0;
    parser->cur_line = 0;
    for (n = 0; n < PARSER_MAX_LINES; n++) {
        parser_line_reset(&parser->lines[n]);
    }

    /* Initialize parser FSM state */
    parser_fsm_reset(parser);

    return PARSER_OK;
}

parser_result_t
parser_quit (parser_t *parser)
{
    if (!parser) {
        return PARSER_ERR_INVALID_PARAMS;
    }
    parser->done = 1;
    return PARSER_OK;
}

parser_result_t
parser_submode_enter (parser_t *parser, void *cookie, char *prompt)
{
    parser_node_t *new_root;

    if (!parser) {
        return PARSER_ERR_INVALID_PARAMS;
    }
    if ((PARSER_MAX_NESTED_LEVELS-1) == parser->root_level) {
        return PARSER_NOT_OK;
    }
    parser->root_level++;
    new_root = parser->cur_node->children;
    assert(new_root);
    assert(PARSER_NODE_ROOT == new_root->type);
    parser->root[parser->root_level] = new_root;
    strcpy(parser->prompt[parser->root_level], prompt); // hack alert - check length
    parser->context.cookie[parser->root_level] = cookie;
    
    return PARSER_OK;
}

parser_result_t
parser_submode_exit (parser_t *parser)
{
    if (!parser) {
        return PARSER_ERR_INVALID_PARAMS;
    }
    if (!parser->root_level) {
        return PARSER_NOT_OK;
    }
    parser->root_level--;
    return PARSER_OK;
}

parser_result_t 
parser_load_cmd (parser_t *parser, char *filename)
{
    FILE *fp;
    char buf[128];
    size_t rsize, n;
    int fd, indent = 0, last_indent = -1, new_line = 1, m;

    if (!VALID_PARSER(parser) || !filename) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    fd = parser->cfg.fd;
    parser->cfg.fd = -1;

    fp = fopen(filename, "r");
    if (!fp) {
        return PARSER_NOT_OK;
    }
    
    parser_fsm_reset(parser);
    while (!feof(fp)) {
        rsize = fread(buf, 1, sizeof(buf), fp);
        for (n = 0; n < rsize; n++) {
            /* Examine the input characters to maintain indent level */
            if ('\n' == buf[n]) {
                indent = 0;
                new_line = 1;
                (void)parser_execute_cmd(parser);
                continue;
            } else if (' ' == buf[n]) {
                if (new_line) {
                    indent++;
                }
            } else {
                if (new_line) {
                    new_line = 0;
                    if (indent < last_indent) {
                        for (m = indent; m < last_indent; m++) {
                            if (PARSER_OK != parser_submode_exit(parser)) {
                                break;
                            }
                            parser_fsm_reset(parser);
                        }
                    }
                    last_indent = indent;
                }
            }
            (void)parser_fsm_input(parser, buf[n]);
        }
    }
    fclose(fp);

    while (parser->root_level) {
        (void)parser_submode_exit(parser);
        parser_fsm_reset(parser);
    }
    parser->cfg.fd = fd;
    return PARSER_OK;
}

static parser_result_t
parser_walk_internal (parser_t *parser, parser_node_t *node, 
                      parser_walker_fn pre_fn, parser_walker_fn post_fn, 
                      void *cookie)
{
    parser_result_t rc;
    parser_node_t *cur_node;

    if (pre_fn) {
        rc = pre_fn(parser, node, cookie);
        if (PARSER_OK != rc) {
            return rc;
        }
    }

    if (PARSER_NODE_END != node->type) {
        cur_node = node->children;
        while (cur_node) {
            parser_walk_internal(parser, cur_node, pre_fn, post_fn, cookie);
            cur_node = cur_node->sibling;
        }
    }
        
    if (post_fn) {
        rc = post_fn(parser, node, cookie);
        if (PARSER_OK != rc) {
            return rc;
        }
    }

    return PARSER_OK;
}

parser_result_t
parser_walk (parser_t *parser, parser_walker_fn pre_fn, 
             parser_walker_fn post_fn, void *cookie)
{
    if (!VALID_PARSER(parser) || (!pre_fn && !post_fn)) {
        return PARSER_ERR_INVALID_PARAMS;
    }

    return parser_walk_internal(parser, parser->root[parser->root_level], 
                                pre_fn, post_fn, cookie);
}

typedef struct help_stack_ {
    char *filter;
    int  tos;
    parser_node_t *nodes[PARSER_MAX_NUM_TOKENS+2];
} help_stack_t;

static parser_result_t
parser_help_pre_walker (parser_t *parser, parser_node_t *node, void *cookie)
{
    help_stack_t *hs = (help_stack_t *)cookie;

    assert(parser && node && hs);
    hs->nodes[hs->tos] = node;
    hs->tos++;

    return PARSER_OK;
}

static parser_result_t
parser_help_post_walker (parser_t *parser, parser_node_t *node, void *cookie)
{
    help_stack_t *hs = (help_stack_t *)cookie;
    int n, do_print;

    assert(parser && node && hs);
    if ((PARSER_NODE_END == node->type) && 
        (!(node->flags & PARSER_NODE_FLAGS_OPT_PARTIAL))) {
        do_print = 0;
        if (hs->filter) {
            /* We have a filter string. Check if it matches any keyword */
            for (n = 0; n < hs->tos; n++) {
                if (PARSER_NODE_KEYWORD != hs->nodes[n]->type) {
                    continue;
                }
                if (strstr(node->param, hs->filter)) {
                    do_print = 1; /* Yes, print it */
                    break;
                }
            }
        } else {
            do_print = 1;
        }
        if (do_print) {
            parser_node_t *cur_node;

            parser_puts(parser, node->desc);
            parser_puts(parser, "\r\n");
            for (n = 0; n < hs->tos; n++) {
                cur_node = hs->nodes[n];
                if ((PARSER_NODE_ROOT == cur_node->type) ||
                    (PARSER_NODE_END == cur_node->type)) {
                    continue;
                }
                parser_puts(parser, cur_node->param);
                parser_putc(parser, ' ');
                if (cur_node->flags & PARSER_NODE_FLAGS_OPT_START) {
                    parser_puts(parser, "{ ");
                }
                if (cur_node->flags & PARSER_NODE_FLAGS_OPT_END) {
                    parser_puts(parser, "} ");
                }
            }
            parser_puts(parser, "\r\n\n");
        }
    }

    /* Pop the stack */
    hs->tos--;
    return PARSER_OK;
}

parser_result_t
parser_help_cmd (parser_t *parser, char *str)
{
    help_stack_t help_stack;

    assert(parser);
    memset(&help_stack, 0, sizeof(help_stack));
    help_stack.filter = str;
    return parser_walk(parser, parser_help_pre_walker, 
                       parser_help_post_walker, &help_stack);
}
