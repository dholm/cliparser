/**
 * \file     test_parser.c
 * \brief    Test program for parser library. 
 * \details  This is a test program with a simple CLI that serves as a demo 
 *           as well.
 * \version  \verbatim $Id: test_parser.c 81 2009-03-20 10:10:22Z henry $ \endverbatim
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "cparser.h"
#include "cparser_priv.h"
#include "cparser_token.h"
#include "cparser_tree.h"

/** Zeroize a data structure */
#define BZERO_OUTPUT memset(output, 0, sizeof(output))

extern char output[2000];
extern int interactive;
int num_passed = 0, num_failed =0;

/**
 * Feed a string into the parser (skipping line buffering) 
 */
static void
feed_parser (cparser_t *parser, const char *str)
{
    int m, n;
    m = strlen(str);
    for (n = 0; n < m; n++) {
        cparser_input(parser, str[n], CPARSER_CHAR_REGULAR);
    }
}

/**
 * Update pass/fail counters and display a status string 
 */
static void
update_result (int pass, const char *test)
{
    if (pass) {
        printf("\nPASS: %s\n", test);
        num_passed++;
    } else {
        printf("\nFAIL: %s\n", test);
        num_failed++;
    }
}

int
main (int argc, char *argv[])
{
    cparser_t parser;
    char *config_file = NULL;
    int ch, debug = 0;

    while (-1 != (ch = getopt(argc, argv, "pic:d"))) {
        switch (ch) {
            case 'p':
                printf("pid = %d\n", getpid());
                break;
            case 'i':
                interactive = 1;
                break;
            case 'c':
                config_file = optarg;
                break;
            case 'd':
                debug = 1;
                break;
        }
    }

    parser.cfg.root = &cparser_root;
    parser.cfg.ch_complete = '\t';
    /* 
     * Instead of making sure the terminal setting of the target and 
     * the host are the same. ch_erase and ch_del both are treated
     * as backspace.
     */
    parser.cfg.ch_erase = '\b';
    parser.cfg.ch_del = 127;
    parser.cfg.ch_help = '?';
    parser.cfg.flags = (debug ? CPARSER_FLAGS_DEBUG : 0);
    strcpy(parser.cfg.prompt, "TEST>> ");
    parser.cfg.fd = STDOUT_FILENO;

    if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
        printf("Fail to initialize parser.\n");
	return -1;
    }
    if (interactive) {
        if (config_file) {
            (void)cparser_load_cmd(&parser, config_file);
        }
        cparser_run(&parser);
    } else {
        /* Run the scripted tests */
        /* Test 1 - WHITESPACE -> TOKEN -> END */
        BZERO_OUTPUT;
        feed_parser(&parser, "show employees\n");
        update_result(0 == strcmp(output, "No employee in the roster.\n"),
                      "WHITESPACE -> TOKEN -> END");

        /* Test 2 - WHITESPACE -> TOKEN -> WHITESPACE -> END */
        BZERO_OUTPUT;
        feed_parser(&parser, "show employees \n");
        update_result(0 == strcmp(output, "No employee in the roster.\n"),
                      "WHITESPACE -> TOKEN -> WHITESPACE -> END");

        /* Test 3 - Test SHORTEST_UNIQUE_KEYWORD */
        BZERO_OUTPUT;
        feed_parser(&parser, "sh employees a\n");
        update_result(0 == strcmp(output, "No employee in the roster.\n"),
                      "shortest unique keyword #1");

        /* Test 4 - Test SHORTEST_UNIQUE_KEYWORD */
        BZERO_OUTPUT;
        feed_parser(&parser, "sh employees a \n");
        update_result(0 == strcmp(output, "No employee in the roster.\n"),
                      "shortest unique keyword #2");

        /* Test 5 - Command completion */
        BZERO_OUTPUT;
        feed_parser(&parser, "sh\temployees\t a\t\n");
        update_result(0 == strcmp(output, "No employee in the roster.\n"),
                      "command completion");

        /* Test 6 - Test one command being prefix of another */
        BZERO_OUTPUT;
        feed_parser(&parser, "show employees\n");
        update_result( 0 == strcmp(output, "No employee in the roster.\n"), "prefixing commands #1");

        BZERO_OUTPUT;
        feed_parser(&parser, "show employees \n");
        update_result( 0 == strcmp(output, "No employee in the roster.\n"), "prefixing commands #2");

        BZERO_OUTPUT;
        /* hack alert - this is not the intention of the test. make a new command that fits */
        feed_parser(&parser, "show employees-by-id 0x0 0x1\n");
        update_result( 0 == strcmp(output, ""), "prefixing commands #3");


        printf("Total=%d  Passed=%d  Failed=%d\n", num_passed + num_failed,
               num_passed, num_failed);
    }
    return 0;
}
