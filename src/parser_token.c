/**
 * \file     parser_token.c
 * \brief    Parser token parsing and completion routines.
 * \version  \verbatim $Id: parser_token.c 51 2009-03-12 22:33:20Z henry $ \endverbatim
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
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "parser.h"
#include "parser_priv.h"
#include "parser_token.h"

/***********************************************************************
 * TOKEN MATCH FUNCTIONS - These functions are used by parser_match()
 *     to check if a token matches a node type.
 ***********************************************************************/
/*
 * parser_match_root - Token matching function for root. There is no
 *     parsing for root node. Parser should crash if this function 
 *     is ever called.
 */
parser_result_t 
parser_match_root (const char *token, const int token_len, 
		   parser_node_t *node, int *is_complete)
{
    assert(node && (PARSER_NODE_ROOT == node->type));
    assert(0); /* we should never try to match a root node */
    return PARSER_NOT_OK;
}

/*
 * parser_match_end - Token matching function for end. There is no
 *     parsing for end node. It always returns no match.
 */
parser_result_t 
parser_match_end (const char *token, const int token_len, 
                  parser_node_t *node, int *is_complete)
{
    assert(node && (PARSER_NODE_END == node->type));
    return PARSER_NOT_OK;
}

/*
 * parser_match_keyword - Token matching function for keywords. It can 
 *     be alphanumeric string with '-' and '_'.
 */
parser_result_t
parser_match_keyword (const char *token, const int token_len, 
		      parser_node_t *node, int *is_complete)
{
    int kw_len, match_len;

    assert(token && node && (PARSER_NODE_KEYWORD == node->type) && is_complete);

    //kw_len = strnlen(node->param, PARSER_MAX_TOKEN_SIZE);
    kw_len = strlen(node->param);
    if (token_len > kw_len) {
	*is_complete = 0;
	return PARSER_NOT_OK;
    }
    match_len = (kw_len < token_len ? kw_len : token_len);
    
    if (!strncmp(token, node->param, match_len)) {
	*is_complete = (match_len == kw_len);
	return PARSER_OK;
    }

    *is_complete = 0;
    return PARSER_NOT_OK;
}

/*
 * parser_match_string - Token matching function for strings. There is 
 *     no restriction on strings.
 */
parser_result_t
parser_match_string (const char *token, const int token_len, 
		     parser_node_t *node, int *is_complete)
{
    assert(token && node && (PARSER_NODE_STRING == node->type) && is_complete);
    *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_uint - Token matching function for 32-bit unsigned decimals or 
 *     hexadecimals. Match against /[0-9]+|0x[0-9a-fA-F]+/.
 */
parser_result_t
parser_match_uint (const char *token, const int token_len, 
		   parser_node_t *node, int *is_complete)
{
    int n, is_dec = 1;
 
    assert(token && node && (PARSER_NODE_UINT == node->type) && is_complete);

    *is_complete = 0;
    assert(token_len > 0);
    /* The first character must be 0-9 */
    if (!isdigit(token[0])) return PARSER_NOT_OK;
    if (1 == token_len) {
	*is_complete = 1;
	return PARSER_OK;
    }


    /* The 2nd character (optional) must be 0-9 or 'x' */
    if ('x' == token[1]) {
	if ('0' != token[0]) {
	    return PARSER_NOT_OK;
	}
	is_dec = 0;
    } else if (!isdigit(token[1])) {
	return PARSER_NOT_OK;
    }
    if (2 == token_len) {
	*is_complete = is_dec;
	return PARSER_OK;
    }

    /*
     * The rest of characters are either decmial or hex depending on
     * the first 2 characters are equal to '0x' or not 
     */
    if (is_dec) {
	for (n = 2; n < token_len; n++) {
	    if (!isdigit(token[n])) return PARSER_NOT_OK;
	}
    } else {
	for (n = 2; n < token_len; n++) {
	    if (!isxdigit(token[n])) return PARSER_NOT_OK;
	}
    }
    *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_int - Token matching function for 32-bit signed decimals.
 *     Match against /[01]{0,1}[0-9]+/.
 */
parser_result_t
parser_match_int (const char *token, const int token_len, 
		  parser_node_t *node, int *is_complete)
{
    int n;

    assert(token && node && (PARSER_NODE_INT == node->type) && is_complete);
    *is_complete = 0;
    assert(token_len > 0);

    /* 1st digit can be 0-9,-,+ */
    if (!isdigit(token[0]) && ('-' != token[0]) && ('+' != token[0])) 
	return PARSER_NOT_OK;
    if (1 == token_len) {
	if (isdigit(token[0])) *is_complete =  1;
	return PARSER_OK;
    }

    /* All subsequent characters must be digits */
    for (n = 1; n < token_len; n++) {
	if (!isdigit(token[n])) return PARSER_NOT_OK;
    }
    *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_hex - Token matching function for 32-bit hexadecimals.
 */
parser_result_t
parser_match_hex (const char *token, const int token_len, 
		  parser_node_t *node, int *is_complete)
{
    int n;

    assert(token && node && (PARSER_NODE_HEX == node->type) && is_complete);

    *is_complete = 0;
    assert(token_len > 0);
    if ('0' != token[0]) return PARSER_NOT_OK;
    if (1 == token_len) return PARSER_OK;
    if ('x' != token[1]) return PARSER_NOT_OK;
    if (2 == token_len) return PARSER_OK;
    for (n = 2; n < token_len; n++) {
	if (!isxdigit(token[n])) return PARSER_NOT_OK;
    }
    *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_float - Token matching function for double precision floating 
 *     point value.
 */
parser_result_t
parser_match_float (const char *token, const int token_len, 
		    parser_node_t *node, int *is_complete)
{
    int base = 0, has_dec_pt = 0, n;
    assert(token && node && (PARSER_NODE_FLOAT == node->type) && is_complete);
    *is_complete = 0;
    assert(token_len > 0);

    /* Handle '+', '-' separately */
    if (('+' == token[base]) || ('-' == token[base])) {
        if (1 == token_len) {
            return PARSER_OK;
        }
        base++;
    }

    /* Handle a first '.' */
    if ('.' == token[base]) {
        if (2 == token_len) {
            return PARSER_OK;
        }
        has_dec_pt = 1;
    } else if (!isdigit(token[base])) {
        return PARSER_NOT_OK;
    }

    /* Work on the rest of them */
    *is_complete = 1;
    for (n = base; n < token_len; n++) {
        if ('.' == token[n]) {
            if (has_dec_pt) {
                *is_complete = 0;
                return PARSER_NOT_OK;
            }
            has_dec_pt = 1;
            continue;
        }
        if (!isdigit(token[n])) {
            return PARSER_NOT_OK;
        }
    }
    return PARSER_OK;
}

/*
 * parser_match_macaddr - Token matching function for 48-bit MAC address.
 *     Match against /($hex{1,2}:){5,5}$hex/ where $hex = "[0-9a-fA-F]".
 */
parser_result_t
parser_match_macaddr (const char *token, const int token_len, 
		      parser_node_t *node, int *is_complete)
{
    int n, num_digit = 0, num_colon = 0;

    assert(token && node && (PARSER_NODE_MACADDR == node->type));
    *is_complete = 0;
    for (n = 0; n < token_len; n++) {
	if (!num_digit) {
	    if (!isxdigit(token[n])) return PARSER_NOT_OK;
	    num_digit = 1;
	} else if (1 == num_digit) {
	    if (!isxdigit(token[n]) && (':' != token[n])) return PARSER_NOT_OK;
	    num_digit = 2;
	    if (':' == token[n]) {
		num_colon++;
		if (num_colon > 5) return PARSER_NOT_OK;
		num_digit = 0;
	    }
	} else {
	    if (':' != token[n]) return PARSER_NOT_OK;
	    num_colon++;
	    if (num_colon > 5) return PARSER_NOT_OK;
	    num_digit = 0;
	}
    }
    if ((5 == num_colon) && (0 < num_digit)) *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_ipv4addr - Token matching function for IPv4 address.
 *     Match against /([0-9]{1,3}\.){3,3}[0-9]{1,3}/
 */
parser_result_t
parser_match_ipv4addr (const char *token, const int token_len, 
		       parser_node_t *node, int *is_complete)
{
    int n, num_digit = 0, num_dot = 0;

    assert(token && node && (PARSER_NODE_IPV4ADDR == node->type));
    *is_complete = 0;
    for (n = 0; n < token_len; n++) {
	assert((unsigned int)num_digit < 4);
	if (!num_digit) {
	    if (!isdigit(token[n])) return PARSER_NOT_OK;
	    num_digit++;
	} else if (3 == num_digit) {
	    if ('.' != token[n]) return PARSER_NOT_OK;
	    num_dot++;
	    if (num_dot > 3) return PARSER_NOT_OK;
	    num_digit = 0;
	} else {
	    if (!isdigit(token[n]) && ('.' != token[n])) return PARSER_NOT_OK;
	    num_digit++;
	    if ('.' == token[n]) {
		num_dot++;
		if (num_dot > 3) return PARSER_NOT_OK;
		num_digit = 0;
	    }
	}
    }
    if ((3 == num_dot) && (0 < num_digit)) *is_complete = 1;
    return PARSER_OK;
}

/*
 * parser_match_file - Token matching function for a file path. Treat
 *     this like a string.
 */
parser_result_t
parser_match_file (const char *token, const int token_len, 
		   parser_node_t *node, int *is_complete)
{
    assert(token && node && (PARSER_NODE_FILE == node->type));
    *is_complete = 1;
    return PARSER_OK;
}

/***********************************************************************
 * TOKEN COMPLETE FUNCTIONS - These functions are used by 
 *     parser_complete() to provide context-sensitive help on 
 *     parameters.
 ***********************************************************************/
/*
 * parser_complete_file - Token complete function for a file path.
 */
parser_result_t
parser_complete_file (const char *token, const int token_len)
{
    assert(token && token_len);
    /* hack alert - fill in the real thing later */
    return PARSER_OK;
}

/***********************************************************************
 * TOKEN GET FUNCTIONS - These functions are used by glue functions
 *     to extract the parameters and call the action function.
 ***********************************************************************/
/*
 * parser_get_string - Token get function for a string.
 */
parser_result_t
parser_get_string (const char *token, const int token_len, char **val)
{
    assert(token && val);
    if (!token_len) {
        *val = NULL;
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    *val = (char *)token;
    return PARSER_OK;
}

/*
 * parser_get_uint - Token get function for 32-bit unsigned integer.
 */
parser_result_t
parser_get_uint (const char *token, const int token_len, uint32_t *val)
{
    uint32_t new = 0, old, d = 0, n;
    assert(token && val);
    if (!token_len) {
        *val = 0;
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    if ((1 < token_len) && ('x' == token[1])) {
	/* Hexadecmial format use parser_get_hex() */
	return parser_get_hex(token, token_len, val);
    }    

    *val = old = 0;
    for (n = 0; n < token_len; n++) {
	if (('0' <= token[n]) && ('9' >= token[n])) {
	    d = token[n] - '0';
	} else {
	    assert(0); /* not a hex digit! */
	}
	new = (old * 10) + d;
	if (((new - d) / 10) != old) return PARSER_NOT_OK;
	old = new;
    }
    *val = new;
    return PARSER_OK;
}

/*
 * parser_get_int - Token get function for 32-bit integer.
 */
parser_result_t
parser_get_int (const char *token, const int token_len, int32_t *val)
{
    int32_t sign = +1;
    uint32_t tmp, init_pos = 0;

    assert(token && val);
    *val = 0;
    if (!token_len) {
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    assert(token_len > 0);
    if ('-' == token[0]) {
	sign = -1;
	init_pos = 1;
    }
    if ('+' == token[0]) {
	init_pos = 1;
    }
    if (PARSER_OK != parser_get_uint(&token[init_pos], 
				     token_len - init_pos, &tmp)) {
	return PARSER_NOT_OK;
    }
    if (+1 == sign) {
	if (tmp > 0x7fffffff) return PARSER_NOT_OK;
	*val = (int32_t)tmp;
    } else {
	assert(-1 == sign);
	if (tmp > 0x80000000) return PARSER_NOT_OK;
	*val = -((int32_t)tmp);
    }
    return PARSER_OK;
}

/*
 * parser_get_hex - Token get function for 32-bit hexadecimal.
 */
parser_result_t
parser_get_hex (const char *token, const int token_len, uint32_t *val)
{
    int n;
    uint32_t new = 0, old, d = 0;

    assert(token && val);
    if (!token_len) {
        *val = 0;
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    assert((token_len > 2) && ('0' == token[0]) && ('x' == token[1]));
    *val = old = 0;
    for (n = 2; n < token_len; n++) {
	if (('0' <= token[n]) && ('9' >= token[n])) {
	    d = token[n] - '0';
	} else if (('a' <= token[n]) && ('f' >= token[n])) {
	    d = token[n] - 'a' + 10;
	} else if (('A' <= token[n]) && ('F' >= token[n])) {
	    d = token[n] - 'A' + 10;
	} else {
	    assert(0); /* not a hex digit! */
	}
	new = (old << 4) + d;
	if (((new - d) >> 4) != old) return PARSER_NOT_OK;
	old = new;
    }
    *val = new;
    return PARSER_OK;
}

parser_result_t
parser_get_float (const char *token, const int token_len, double *val)
{
    assert(token && val);
    if (!token_len) {
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    if (1 != sscanf(token, "%lf", val)) {
        *val = 0.0;
        return PARSER_NOT_OK;
    }
    return PARSER_OK;
}

/*
 * parser_get_macaddr - Token get function for MAC address.
 */
parser_result_t
parser_get_macaddr (const char *token, const int token_len, parser_macaddr_t *val)
{
    uint32_t a, b, c, d, e, f;
    assert(token && val);
    if (!token_len) {
        memset(val, 0, sizeof(*val));
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    if ((6 != sscanf(token, "%lx:%lx:%lx:%lx:%lx:%lx", (unsigned long *)&a, 
                     (unsigned long *)&b, (unsigned long *)&c, 
                     (unsigned long *)&d, (unsigned long *)&e, 
                     (unsigned long *)&f)) ||
	(a > 255) || (b > 255) || (c > 255) || (d > 255) || (e > 255) || 
	(f > 255)) {
	val->octet[0] = val->octet[1] = val->octet[2] = 
            val->octet[3] = val->octet[4] = val->octet[5] = 0;
	return PARSER_NOT_OK;
    }
    val->octet[0] = (uint8_t)a;
    val->octet[1] = (uint8_t)b;
    val->octet[2] = (uint8_t)c;
    val->octet[3] = (uint8_t)d;
    val->octet[4] = (uint8_t)e;
    val->octet[5] = (uint8_t)f;

    return PARSER_OK;
}

/*
 * parser_get_ipv4addr - Token get function for IPv4 address.
 */
parser_result_t
parser_get_ipv4addr (const char *token, const int token_len, uint32_t *val)
{
    uint32_t a, b, c, d;
    assert(token && val);
    if (!token_len) {
        *val = 0;
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    if ((4 != sscanf(token, "%lu.%lu.%lu.%lu", (unsigned long *)&a, 
                     (unsigned long *)&b, (unsigned long *)&c, 
                     (unsigned long *)&d)) ||
	(a > 255) || (b > 255) || (c > 255) || (d > 255)) {
        *val = 0;
	return PARSER_NOT_OK;
    }
    *val = ((uint32_t)a) << 24;
    *val |= ((uint32_t)b) << 16;
    *val |= ((uint32_t)c) << 8;
    *val |= ((uint32_t)d);
    
    return PARSER_OK;
}

/*
 * parser_get_file - Token get function for file path.
 */
parser_result_t
parser_get_file (const char *token, const int token_len, char **val)
{
    struct stat stat_buf;

    assert(token && val);
    *val = NULL;
    if (!token_len) {
        return PARSER_NOT_OK; /* optional argument wasn't provided */
    }
    if (stat(token, &stat_buf) || !(stat_buf.st_mode & S_IFREG)) {
	return PARSER_NOT_OK;
    }
    *val = (char *)token;
    return PARSER_OK;
}

