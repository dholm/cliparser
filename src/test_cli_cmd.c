/**
 * \file     test_parser.c
 * \brief    Test program for parser library. 
 * \details  This is a test program with a simple CLI that serves as a demo 
 *           as well.
 * \version  \verbatim $Id: test_parser.c 33 2009-01-22 06:45:33Z henry $ \endverbatim
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
#include "parser.h"
#include "parser_token.h"

int interactive = 0;
#define PRINTF(args...)                    \
    if (interactive) {                     \
        printf(args);                      \
    } else {                               \
        memset(output, 0, sizeof(output)); \
        sprintf(output, args);             \
    }

#define MAX_NAME        (128)
#define MAX_EMPLOYEES   (100)
#define MAX_TITLE       (32)

char output[2000]; /* buffer for sprintf */

/**
 * Employee record.
 */
typedef struct employee_ {
    char         name[MAX_NAME];
    uint32_t     height;
    uint32_t     weight;
    double       bonus_factor;
    uint32_t     pc_ip_addr;
    uint8_t      mac_ip_addr[6];
    uint32_t     id;
    struct dob_ {
        uint8_t  month;
        uint8_t  day;
        uint8_t  year;
    } dob;
    char         title[MAX_TITLE];
} employee_t;

employee_t roster[MAX_EMPLOYEES];

/**
 * Add a new employee record to the roster.
 *
 * \param    roster  Pointer to the array of employee records.
 * \param    id      New employee id.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_OUT_OF_RES if there is
 *           no more available slot.
 */ 
static parser_result_t
test_employee_add (employee_t *roster, const uint32_t id)
{
    int n;

    assert(roster && id);

    /* Look for an unused slot */
    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (!roster[n].id) {
            /* Found one. Use it */
            memset(&roster[n], 0, sizeof(roster[n]));
            roster[n].id = id;
            return PARSER_OK;
        }
    }

    return PARSER_ERR_OUT_OF_RES;
}

/**
 * Remove an existing employee record from the roster.
 *
 * \param    roster  Pointer to the array of employee records.
 * \param    id      Employee id to be deleted.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_NOT_EXIST if there is
 *           no employee with that id.
 */
static parser_result_t
test_employee_remove (employee_t *roster, const uint32_t id)
{
    int n;

    assert(roster && id);
    
    /* Look for a match of the name */
    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (id == roster[n].id) {
            memset(&roster[n], 0, sizeof(*roster));
            return PARSER_OK;
        }
    }

    return PARSER_ERR_NOT_EXIST;
}

/**
 * Search for an existing employee record.
 *
 * \param    roster  Pointer to the array of employee records.
 * \param    id      Employee id to search.
 *
 * \return   PARSER_OK if succeeded; PARSER_ERR_NOT_EXIST if there is
 *           no employee with that id.
 */
static employee_t *
test_employee_search (employee_t *roster, uint32_t id)
{
    int n;

    assert(roster && id);
    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (id == roster[n].id) {
            return &roster[n];
        }
    }
    return NULL;
}

/**
 * Initialize a roster of employee records.
 *
 * \param    roster Pointer to the array of employee records.
 */
void
test_employee_init (employee_t *roster)
{
    int n;

    assert(roster);
    for (n = 0; n < MAX_EMPLOYEES; n++) {
        memset(&roster[n], 0, sizeof(*roster));
    }
}

/**
 * Handle "show employees".
 */
parser_result_t
parser_cmd_show_employees (parser_context_t *context)
{
    int n, num_shown = 0;

    assert(context);

    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (!roster[n].id) {
            continue;
        }
        PRINTF("0x%08X %s\n", roster[n].id, roster[n].name);
        num_shown++;
    }
    if (!num_shown) {
        PRINTF("No employee in the roster.\n");
    }
    return PARSER_OK;
}

/**
 * Handle "show employees all".
 */
parser_result_t
parser_cmd_show_employees_all (parser_context_t *context)
{
    int n, num_shown = 0;

    assert(context);

    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (!roster[n].id) {
            continue;
        }
        PRINTF("%s\n", roster[n].name);
        PRINTF("   ID: 0x%08X\n", roster[n].id);
        PRINTF("   Height: %3d\"   Weight: %3d lbs.\n",
               (int)roster[n].height, (int)roster[n].weight);
        num_shown++;
    }
    if (!num_shown) {
        PRINTF("No employee in the roster.\n");
    }
    return PARSER_OK;
}

/**
 * Handle "show employees-by-id <HEX:min> <HEX:max>".
 */
parser_result_t
parser_cmd_show_employees_by_id_min_max (parser_context_t *context, 
                                         uint32_t *min, uint32_t *max)
{
    assert(context && min); /* don't assert on 'max' because it is optional */
    return PARSER_OK;
}

/**
 * Handle "emp -> name <STRING:name>".
 */
parser_result_t
parser_cmd_emp_name_name (parser_context_t *context, char **name)
{
    employee_t *emp;

    assert(context && name && (*name));
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    strncpy(emp->name, *name, MAX_NAME);
    return PARSER_OK;
}

/**
 * Handle "emp -> height <UINT:inches>".
 */
parser_result_t
parser_cmd_emp_height_inches (parser_context_t *context, uint32_t *inches)
{
    employee_t *emp;

    assert(context && inches);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    if (120 <= *inches) {
        PRINTF("Too tall!\n");
        return PARSER_NOT_OK;
    }
    emp->height = *inches;
    return PARSER_OK;
}

/**
 * Handle "emp -> weight <UINT:lbs>".
 */
parser_result_t
parser_cmd_emp_weight_lbs (parser_context_t *context, uint32_t *lbs)
{
    employee_t *emp;

    assert(context && lbs);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    emp->weight = *lbs;
    return PARSER_OK;
}

/**
 * Handle "emp -> bonuns-factor <FLOAT:scale>".
 */
parser_result_t
parser_cmd_emp_bonus_factor_scale (parser_context_t *context, double *scale)
{
    employee_t *emp;

    assert(context && scale);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    emp->bonus_factor = *scale;
    return PARSER_OK;
}

/**
 * Handle "emp -> pc-ip-address <IPV4ADDR:ipv4>".
 */
parser_result_t
parser_cmd_emp_pc_ip_address_ipv4 (parser_context_t *context,
                                   uint32_t *ipv4)
{
    employee_t *emp;

    assert(context && ipv4);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    emp->pc_ip_addr = *ipv4;
    return PARSER_OK;
}


/**
 * Handle "emp -> pc-mac-address <MACADDR:addr>".
 */
parser_result_t
parser_cmd_emp_pc_mac_address_addr (parser_context_t *context,
                                    parser_macaddr_t *addr)
{
    assert(context && addr);
    return PARSER_OK;
}

/**
 * Handle "emp -> employee-id <HEX:id>".
 */
parser_result_t
parser_cmd_emp_employee_id_id (parser_context_t *context, uint32_t *id)
{
    employee_t *emp;

    assert(context && id);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    emp->id = *id;
    return PARSER_OK;
}

/**
 * Handle "emp -> date-of-birth <INT:month> <INT:day> <INT:year>".
 */
parser_result_t
parser_cmd_emp_date_of_birth_month_day_year (parser_context_t *context, 
                                             int32_t *month, int32_t *day, 
                                             int32_t *year)
{
    employee_t *emp;

    assert(context && month && day && year);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    if (!(*month) || (12 < (*month))) {
        PRINTF("Month must be from 1-12.\n");
        return PARSER_NOT_OK;
    }
    /* This is not totally correct but you get the point */
    if (!(*day) || (31 < (*day))) {
        PRINTF("Day must be from 1-31.\n");
    }
    emp->dob.month = *month;
    emp->dob.day = *day;
    emp->dob.year = *year;
    return PARSER_OK;
}

/**
 * Handle "emp -> title <STRING:title>".
 */
parser_result_t
parser_cmd_emp_title_title (parser_context_t *context, char **title)
{
    employee_t *emp;

    assert(context && title);
    emp = (employee_t *)context->cookie[1];
    assert(emp);
    strncpy(emp->name, *title, MAX_NAME);
    emp->name[MAX_NAME-1] = '\0';
    return PARSER_OK;
}

/**
 * Handle "emp -> exit".
 */
parser_result_t
parser_cmd_emp_exit (parser_context_t *context)
{
    assert(context && context->parser);
    return parser_submode_exit(context->parser);
}

/**
 * Handle "employee <UINT:id>".
 */
parser_result_t
parser_cmd_employee_id (parser_context_t *context, uint32_t *id)
{
    employee_t *new_emp;
    char prompt[PARSER_MAX_PROMPT];

    assert(context && id && (*id));

    /* Check that there is no existing employee by that name */
    new_emp = test_employee_search(roster, *id);
    if (!new_emp) {
        /* Add a new record with that name */
        if (PARSER_OK != test_employee_add(roster, *id)) {
            PRINTF("Fail to add employee.\n");
            return PARSER_NOT_OK;
        }
        new_emp = test_employee_search(roster, *id);
    }

    /* Enter the submode */
    assert(new_emp);
    snprintf(prompt, PARSER_MAX_PROMPT, "0x%08X: ", *id);
    return parser_submode_enter(context->parser, new_emp, prompt);
}

/**
 * Handle "no employee <UINT:id>".
 */
parser_result_t
parser_cmd_no_employee_id (parser_context_t *context, uint32_t *id)
{
    employee_t *emp;

    assert(context && id && *id);

    /* Check that the name exists */
    emp = test_employee_search(roster, *id);
    if (!emp) {
        PRINTF("Employee 0x%08X does not exist in the database.\n", *id);
        return PARSER_ERR_NOT_EXIST;
    }

    /* Delete it */
    return test_employee_remove(roster, *id);
}

/**
 * Load a roster file into memory.
 */
parser_result_t
parser_cmd_load_roster_filename (parser_context_t *context, char **filename)
{
    assert(context && filename && (*filename));
    return parser_load_cmd(context->parser, *filename);
}

/**
 * Save the roster in memory into a file.
 */
parser_result_t
parser_cmd_save_roster_filename (parser_context_t *context, char **filename)
{
    FILE *fp;
    int n;

    assert(context && filename);
    fp = fopen(*filename, "w");
    if (!fp) {
        PRINTF("Fail to open %s.\n", *filename);
        return PARSER_NOT_OK;
    }

    for (n = 0; n < MAX_EMPLOYEES; n++) {
        if (!roster[n].id) {
            continue;
        }
        fprintf(fp, "employee 0x%08X\n", roster[n].id);
        if (strlen(roster[n].name)) {
            fprintf(fp, " name %s\n", roster[n].name);
        }
        fprintf(fp, " height %u\n", roster[n].height);
        fprintf(fp, " weight %u\n", roster[n].weight);
        if (roster[n].bonus_factor) {
            fprintf(fp, " bonus-factor %f\n", roster[n].bonus_factor);
        }
        if (roster[n].pc_ip_addr) {
            fprintf(fp, " pc-ip-address %d.%d.%d.%d\n", 
                    (int)((roster[n].pc_ip_addr >> 24) & 0xff),
                    (int)((roster[n].pc_ip_addr >> 16) & 0xff),
                    (int)((roster[n].pc_ip_addr >> 8) & 0xff),
                    (int)((roster[n].pc_ip_addr >> 0) & 0xff));
        }
        if (roster[n].dob.month && roster[n].dob.day && roster[n].dob.year) {
            fprintf(fp, " date-of-birth %d %d %d\n", roster[n].dob.month,
                    roster[n].dob.day, roster[n].dob.year);
        }
        if (strlen(roster[n].title)) {
            fprintf(fp, " title %s\n", roster[n].title);
        }
    }

    fclose(fp);
    return PARSER_OK;
}

/**
 * List all available commands
 */
parser_result_t
parser_cmd_help_filter (parser_context_t *context, char **filter)
{
    assert(context);
    return parser_help_cmd(context->parser, filter ? *filter : NULL);
}

parser_result_t
parser_cmd_emp_help_filter (parser_context_t *context, char **filter)
{
    assert(context);
    return parser_help_cmd(context->parser, filter ? *filter : NULL);
}

/**
 * Exit the parser test program.
 */
parser_result_t
parser_cmd_quit (parser_context_t *context)
{
    assert(context);
    return parser_quit(context->parser);
}

