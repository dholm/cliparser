/**
 * \file     parser.h
 * \brief    Parser library definitions and function prototypes.
 * \version  \verbatim $Id: parser.h 51 2009-03-12 22:33:20Z henry $ \endverbatim
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
/**
 * \mainpage CLI Parser
 *
 * \section intro I. INTRODUCTION
 *
 * Many embedded system require a command-line interface (CLI). CLI 
 * commands are useful for debugging and testing. This library provides 
 * a simply yet feature-rich CLI infrastructure.
 *
 * The goal of CLI parser is to centralize the complexity of CLI features
 * such that developers do not need to spend a lot of time coding in order
 * to get the convenience of a modern, Cisco-like CLI.
 *
 * \section overview II. OVERVIEW
 *
 * CLI parser simplifies command creation by introducing a .cli file 
 * syntax. Users create their commands by writing .cli file. A python 
 * script (provided by CLI parser) converts these .cli files into a 
 * list of C structures forming a parse tree. The parse tree is
 * then used by CLI parser to provide CLI in run-time.
 *
 * Users are responsible for providing the following:
 *
 * - .cli files - These files define CLI commands and are compiled by 
 *   mk_parser.py into a parse tree used by libparser.
 *
 * - Action functions - They are callback functions when a CLI command
 *   is accepted by CLI parser.
 *
 * - I/O functions - Prototypes of these functions are listed in parser_io.h.
 *   These functions connect CLI parser to the system I/O. For Linux/UNIX,
 *   these functions are provided. For other systems, you will have to provide
 *   your own.
 *
 * CLI parser provides the following features:
 *
 * - Command completion
 * - Command recall
 * - Context sensitive help
 * - Automatic help generation
 *
 * \section cli III. CLI FILES
 *
 * CLI files have suffix of .cli. Each CLI contains four types lines:
 * - Empty line
 * - Comment - Any line preceeded by '//' is a line of comment. '//'
 *   must be 1st character of the line.
 * - Directive - C style \#ifdef, \#ifndef, \#include, \#submode,
 *   \#endsubmode and \#end are available. The label is defined via 
 *   command-line arguments to mk_parser.py.
 * - Command definition - A command definition contains a sequence of 
 *   tokens separated by spaces.
 *
 * The following are a list of currently supported tokens. There are
 * two classes of tokens - keywords and parameters. Keywords are
 * strings that form CLI commands. For example, the command
 * "show roster" consists of two keyword tokens ("show" and "roster"). 
 * Each keyword should be a string consists of alphabet, digits (0-9), 
 * '_' and '-'. Parameters are values entered by users. They have 
 * the form of <[type]:[var]> where [type] is the token type and [var] 
 * is a legal C variable name.
 *
 * CLI parser provides the following parameter token types:
 *
 * - STRING - A string without ' '.
 * - UINT - 32-bit unsigned integer in decimal or 
 *   hexadecimal. E.g. 0x1234, 312476.
 * - INT - 32-bit signed integer. E.g., -1234, 13469.
 * - HEX - 32-bit unsigned integer in form of hexadecimal 
 *   only. E.g., 0xabc1234.
 * - FLOAT - 64-bit floating point value. E.g., 0.134,
 *   -123.406.
 * - MACADDR - 48-bit IEEE 802 MAC address. E.g., 00:11:AA:EE:FF.
 * - IPV4ADDR - IPv4 address. E.g., 10.1.1.1 or 192.168.1.1.
 * - FILE - A string that represents the path of a file. It
 *   is a specialized version of STRING that provides file 
 *   completion.
 * 
 * \subsection extending A. Extending CLI Parser
 *
 * (<i>In the future, user can extend CLI parser by adding new token 
 * types.</i>)
 *
 * \subsection optional B. Optional Parameters
 *
 * It is possible to include optional parameters in CLI commands. To
 * make a parameter optional, wrap it inside a pair of { }. For example,
 * suppose we define a command that adds 2 or 3 numbers:
 *
 * add <INT:a> <INT:b> { <INT:c> }
 *
 * The following are valid inputs: 
 *
 * <pre>
 * add 1 2 3
 * add 1 2
 * </pre>
 *
 * It is possible to have multiple optional parameters in a command but
 * they must be nested. For example, suppose we have a command that adds
 * two to four numbers:
 *
 * <pre>
 * LEGAL: add <INT:a> <INT:b> { <INT:c> { <INT:d> } }
 * ILLEGAL: add <INT:a> <INT:b> { <INT:c> } { <INT:d> }
 * </pre>
 *
 * There are couple reasons for imposing such restrictions. First,
 * a command with N optional parameters has only N possible acceptable
 * forms with the nesting constraint but 2^N forms without it. Second,
 * there are unresolvable ambiguities without the constraint.
 * 
 * Suppose user inputs "add 1 2 3". Is it c=3 and d is omitted or
 * c is omitted and d=3? With the nesting constraint, c must be 3
 * and d is omitted.
 * 
 * \subsection submode C. Submode
 *
 * submode allows the parser to switch to a different parse tree.
 * submode is a useful feature for creating hierarchy in CLI.
 * Sometimes, a "flat" CLI structure can make it difficult to read
 * configuration.
 *
 * Suppose we have a network router with interfaces. We can
 * configure it using a flat hierarchy.
 *
 * interface eth0 ip-address 20.1.1.1 255.0.0.0
 * interface eth0 acl acl1
 * interface eth0 qos qos-profile1
 *
 * or it can use submode to wrap all interface configurations.
 *
 * interface eth0  <- enter interface submode.
 *  ip-adress 20.1.1.1 255.0.0.0
 *  acl acl1
 *  qos qos-profile1
 *
 * By grouping all interface configuration commands inside interface
 * submode, it reduces the number of top-level commands. This has
 * an implication of effectiveness of context sensitive help. If
 * a first-time user want to see what top-level commands are 
 * available, he can get overwhelmed if all commands belong to
 * the top-level.
 *
 * To define commands in a submode, wrap those commands between
 * \#submode and \#endsubmode.
 *
 * A \#submode directive requires a submode name. All submode action
 * functions are prefixed by this name after "parser_cmd_". For example, 
 * \#submode "emp" means all its action functions are prefixed by 
 * "parser_cmd_emp_".
 *
 * Submodes can be nested. The total number of submode plus the top-
 * level is equal to PARSER_MAX_NESTED_LEVELS.
 *
 * \subsection example D. Example
 *
 * The following is an example of a CLI for a sample employee 
 * database program.
 *
 * <pre>
 * // This is the CLI syntax file of a test/sample program that handles
 * // an employee roster of a company.
 *
 *
 * // This line tests if comments are handled correctly
 * 
 * // List a summary of employees.
 * show employees
 * 
 * // List detail records of all employees.
 * show employees all
 *
 * // List all employees within a certain range of employee ids
 * show employees-by-id <UINT:min> { <UINT:max> }
 * 
 * // Add a new employee or enter the record of an existing employee
 * employee <HEX:id>
 *
 * \#ifdef TEST_LABEL1
 * // This next line tests if file inclusion is handled correctly.
 * \#include "test_included.cli"
 * \#endif
 *
 * \#ifdef TEST_LABEL2
 * // This next line tests if not existing label is handled correctly.
 * \#include "test_not_included.cli"
 * \#endif
 *
 * no employee <HEX:id>
 *
 * // Save the current roster to a file
 * save roster <STRING:filename>
 *
 * // Load roster file
 * load roster <FILE:filename>
 *
 * // List all available commands with a substring 'filter' in it.
 * help { <STRING:filter> }
 *
 * // Leave the database
 * quit
 *
 * </pre>
 * 
 * \section action_fn IV. ACTION FUNCTIONS
 *
 * An action function is the function called when a command is accepted
 * by CLI parser. This function name is automatically generated from the
 * command itself by concatenating all keywords and parameter variable
 * name, converting all '-' to '_' and prepending 'parser_cmd_' to it.
 *
 * For example, "save roster <STRING:filename>" expects an action function
 * "parser_cmd_save_roster_filename". The first argument of an action
 * function is parser_context_t which provides some information about
 * the parser state. (More on this later). Each parameter token type
 * corresponds to the following C data type:
 *
 * - STRING -> char *
 * - UINT -> uint32_t 
 * - INT -> int32_t 
 * - HEX -> uint32_t 
 * - FLOAT -> double
 * - MACADDR -> parser_macaddr_t
 * - IPV4ADDR -> uint32_t 
 * - FILE -> char *
 *
 * All parameter tokens are converted into pointers of their corresponding
 * C argument of the action function in order. For example, "save roster 
 * <STRING:filename>" expects the corresponding action function:
 *
 * <pre>
 * parser_result_t 
 * parser_cmd_save_roster(parser_context_t *context, char **filename);
 * </pre>
 *
 * The extra pointer is used to indicate if the parameter is present.
 * For optional parameter that is omitted, the pointer will be NULL.
 * For parameters that exist (mandatory or optional), they will point to
 * valid data. You should check against NULL for each optional parameter
 * in your action functions.
 *
 * \subsection action_fn_submode A. Entering And Leaving Submode
 *
 * If submode is used, there must be a command for each submode to
 * enter it. Action function of that command should call 
 * parser_submode_enter(). An opaque pointer 'cookie' is provide to allow
 * some context of the submode. For example, going back to our interface
 * submode example, 'cookie' can point to the interface structure.
 * The command prompt can also change to indicate what submode you
 * are in; simply provide a string in 'prompt'.
 *
 * When user enters a command to exit the submode, call 
 * parser_submode_exit().
 *
 * \section api V. OTHER USEFUL API
 *
 * parser_load_cmd() feeds a text file into the parser. It automatically
 * exits submode by examining indentation. Submodes are indented from
 * its parent mode. If a line indentation is less than the previous line,
 * it automatically exits the submode. This behavior is identical to
 * Cisco CLI.
 *
 * \section build VI.. BUILDING YOUR APPLICATION
 *
 * To run CLI parser in your application,
 * - Fill out parser configuration structure (parser_cfg_t) in a parser_t.
 * - Call parser_init() to create the parser.
 * - Run parser when you are ready. There are two interfaces available -
 *   parser_input() and parser_run(). parser_input() is a low-level input.
 *   You are responsible for getting all characters (from whatever source)
 *   and feed them into the parser one character at a time. parser_run()
 *   is a more automated interface. The user provides a set of parser I/O
 *   functions (see parser_io.h) and parser_run() will call them to get
 *   characters. It returns when the CLI session terminates.
 *
 * To build your application, include a rule for building parser_tree.c.
 * This file and parser_tree.h is the output of mk_parser.py. For example, 
 * "mk_parser.py -D TEST_LABEL1 test.cli" compiles test.cli into 
 * parser_tree.c and parser_tree.h. parser_tree.c contains the parser tree 
 * data structures and parser_tree.h contains function prototypes for all 
 * action functions and the root node of parser tree(s).
 *
 * Then, include parser_tree.c in your build and include parser_tree.h in
 * your files for action functions. Compile and build your application.
 *
 * \section port VI. PORTING
 * 
 * CLI parser is mostly platform-independent. The only exception being
 * the I/O interface. The default I/O assumes input comes from a UNIX
 * terminate as stdin and output goes to stdout. If these are not true,
 * you need to provide a set of functions to initialize, cleanup I/O
 * subsystem, to read a character and to write a character and a string.
 *
 * It also assumes that <stdint.h> exists to provide int32_t, uint32_t,
 * int16_t, uint16_t, uint8_t, int8_t.
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include "parser_options.h"
#include <stdint.h>

/*
 * This is to match Cisco CLI behavior. For example, if there is a
 * command "show crypto interfaces", one can just enter "sh cry int"
 * if no other command has the same prefix form.
 */
#define SHORTEST_UNIQUE_KEYWORD

/**
 * \brief    Parser API result code.
 */
typedef enum parser_result_ {
    PARSER_OK = 0,
    PARSER_NOT_OK,
    PARSER_ERR_INVALID_PARAMS, /**< Invalid input parameters to a call */
    PARSER_ERR_NOT_EXIST,
    PARSER_ERR_OUT_OF_RES,
    PARSER_MAX_RESULTS
} parser_result_t;

typedef struct parser_ parser_t;
typedef struct parser_node_ parser_node_t;

#include "parser_line.h"
#include "parser_io.h"

/*
 * parser_context_t is a structure passed back in the action 
 * callback function. It contains the parser where the command
 * is accepted and some cookie that can be set the caller.
 */
typedef struct {
    parser_t *parser;
    void     *cookie[PARSER_MAX_COOKIES];
} parser_context_t;

#define PARSER_FLAGS_DEBUG        (1 << 0)

/**
 * \struct   parser_cfg_
 * \brief    Contains all configurable parameters of a parser.
 */
typedef struct parser_cfg_ {
    parser_node_t   *root;
    char            ch_complete;
    char            ch_erase;
    char            ch_del;
    char            ch_help;
    char            prompt[PARSER_MAX_PROMPT];
    int             fd;
    unsigned long   flags;
} parser_cfg_t;

/**
 * \struct   parser_token_
 * \brief    A parsed token.
 */
typedef struct parser_token_ {
    /* Index (in the line) of the beginning of the token */
    short         begin_ptr;
    short         token_len;  /**< Number of character in the token */
    char          buf[PARSER_MAX_TOKEN_SIZE]; /**< Local copy of the token */
    parser_node_t *parent;
} parser_token_t;

/**
 * \brief    Parser FSM states.
 * \details  There are 3 possible states in parser FSM.
 */
typedef enum parser_state_ {
    PARSER_STATE_WHITESPACE = 0,
    PARSER_STATE_TOKEN,
    PARSER_STATE_ERROR,
    PARSER_MAX_STATES
} parser_state_t;

struct parser_ {
    parser_cfg_t     cfg;
    int              root_level;
    parser_node_t    *root[PARSER_MAX_NESTED_LEVELS];
    char             prompt[PARSER_MAX_NESTED_LEVELS][PARSER_MAX_PROMPT];
    parser_node_t    *cur_node;

    /* FSM states */
    parser_state_t   state;
    short            token_tos;
    short            current_pos;
    short            last_good;
    parser_token_t   tokens[PARSER_MAX_NUM_TOKENS]; /* parsed tokens */

    /* Line buffering states */
    short            max_line;
    short            cur_line;
    parser_line_t    lines[PARSER_MAX_LINES];

    int              done;
    parser_context_t context; /* passed back to action function */
};

typedef parser_result_t (*parser_glue_fn)(parser_t *parser);
typedef parser_result_t (*parser_token_fn)(char *token, int token_len,
                                           int *is_complete);
typedef parser_result_t (*parser_walker_fn)(parser_t *parser, 
                                            parser_node_t *node, void *cookie);

/**
 * Initialize a parser.
 *
 * \param    cfg Pointer to the parser configuration structure.
 *
 * \retval   parser Pointer to the initialized parser.
 * \return   PARSER_OK if succeeded; PARSER_NOT_OK if failed.
 */
parser_result_t parser_init(parser_cfg_t *cfg, parser_t *parser);

/**
 * Input a character to the parser.
 *
 * \param    parser  Pointer to the parser structure.
 * \param    ch      Character to be input.
 * \param    ch_type Character type.
 *
 * \return   PARSER_OK if succeeded; PARSER_NOT_OK if failed.
 */
parser_result_t parser_input(parser_t *parser, char ch, parser_char_t ch_type);

/**
 * Run the parser. This function is a wrapper around parser_input(). It
 * first calls parser_io_init(). Then, it calls parser_getch() and feeds
 * character into the parser until it quits.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_NOT_OK if failed.
 */
parser_result_t parser_run(parser_t *parser);

/**
 * Walk the parse tree in the parser.
 *
 * \param    parser  Pointer to the parser structure.
 * \param    pre_fn  Walker function that called before tranverse its children.
 * \param    post_fn Walker function that called after transvere its children.
 * \param    cookie  An opaque pointer that is passed back to the caller via
 *                   callback functions 'pre_fn' and 'post_fn'.
 *
 * \return   PARSER_OK if succeeded; PARSER_NOT_OK if failed.
 */
parser_result_t parser_walk(parser_t *parser, parser_walker_fn pre_fn,
                            parser_walker_fn post_fn, void *cookie);

/**
 * Walk the parser tree and generate a list of all available commands.
 *
 * \param    parser Pointer to the parser structure.
 * \param    str    Pointer to a filter string. If it is NULL, all
 *                  commands in the parse tree are displayed. Otherwise,
 *                  only commands with keywords that contain 'str' as
 *                  a substring are displayed.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if 
 *           the parser structure is invalid.
 */
parser_result_t parser_help_cmd(parser_t *parser, char *str);

/**
 * Load a command/config file to the parser. A command/config file is 
 * just a text file with CLI commands. (One command per line.) The only 
 * difference is that submode is automatically exited if the 
 * indentation changes. This behavior is the same as Cisco CLI.
 *
 * \param    parser   Pointer to the parser structure.
 * \param    filename Pointer to the filename.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS
 *           if the input parameters are NULL; PARSER_NOT_OK if the file
 *           cannot be opened.
 */
parser_result_t parser_load_cmd(parser_t *parser, char *filename);

/**
 * Exit a parser session.
 *
 * \param    parser - Pointer to the parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if failed.
 */
parser_result_t parser_quit(parser_t *parser);

/**
 * Enter a submode.
 *
 * \param    parser Pointer to the parser structure.
 * \param    cookie A parameter that is passed back to each submode command
 *                  via parser_context_t.
 * \param    prompt The new submode prompt.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if the input 
 *           parameters are invalid; PARSER_NOT_OK if there too many levels 
 *           of submode already.
 */
parser_result_t parser_submode_enter(parser_t *parser, void *cookie, 
                                     char *prompt);

/**
 * Leave a submode. The previous mode context and prompt are 
 * automatically restored.
 *
 * \param    parser Pointer to the parser structure.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_INVALID_PARAMS if the input 
 *           parameters are invalid; PARSER_NOT_OK if the parser has not 
 *           entered any submode.
 */
parser_result_t parser_submode_exit(parser_t *parser);

#endif /* __PARSER_H__ */
