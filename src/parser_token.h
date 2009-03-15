/**
 * \file     parser_token.h
 * \brief    Parser token parsing and completion routine prototypes.
 * \version  \verbatim $Id: parser_token.h 51 2009-03-12 22:33:20Z henry $ \endverbatim
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

#ifndef __PARSER_TOKEN_H__
#define __PARSER_TOKEN_H__

typedef enum {
    PARSER_NODE_ROOT = 0,
    PARSER_NODE_END,
    PARSER_NODE_KEYWORD,
    PARSER_NODE_STRING,
    PARSER_NODE_UINT,
    PARSER_NODE_INT,
    PARSER_NODE_HEX,
    PARSER_NODE_FLOAT,
    PARSER_NODE_MACADDR,
    PARSER_NODE_IPV4ADDR,
    PARSER_NODE_FILE,
    PARSER_MAX_NODES
} parser_node_type_t;

typedef struct {
    uint8_t octet[6];
} parser_macaddr_t;

typedef parser_result_t (*parser_match_fn)(const char *token, 
					   const int token_len, 
					   parser_node_t *node, 
					   int *is_complete);
typedef parser_result_t (*parser_complete_fn)(const char *token,
					      const int token_len);

extern parser_match_fn parser_match_fn_tbl[];
extern parser_complete_fn parser_complete_fn_tbl[];

/********** Token match functions **********/
parser_result_t parser_match_root(const char *token, const int token_len,
				  parser_node_t *node, int *is_complete);
parser_result_t parser_match_end(const char *token, const int token_len,
                                 parser_node_t *node, int *is_complete);
parser_result_t parser_match_keyword(const char *token, const int token_len,
				     parser_node_t *node, int *is_complete);
parser_result_t parser_match_string(const char *token, const int token_len,
				    parser_node_t *node, int *is_complete);
parser_result_t parser_match_uint(const char *token, const int token_len,
				  parser_node_t *node, int *is_complete);
parser_result_t parser_match_int(const char *token, const int token_len,
				 parser_node_t *node, int *is_complete);
parser_result_t parser_match_hex(const char *token, const int token_len,
				 parser_node_t *node, int *is_complete);
parser_result_t parser_match_float(const char *token, const int token_len,
				   parser_node_t *node, int *is_complete);
parser_result_t parser_match_macaddr(const char *token, const int token_len,
				     parser_node_t *node, int *is_complete);
parser_result_t parser_match_ipv4addr(const char *token, const int token_len,
				      parser_node_t *node, int *is_complete);
parser_result_t parser_match_file(const char *token, const int token_len,
				  parser_node_t *node, int *is_complete);

/********** Token complete functions **********/
parser_result_t parser_complete_file(const char *token, const int token_len);

/********** Token get functions **********/
parser_result_t parser_get_string(const char *token, const int token_len, char **val);
parser_result_t parser_get_uint(const char *token, const int token_len, uint32_t *val);
parser_result_t parser_get_int(const char *token, const int token_len, int32_t *val);
parser_result_t parser_get_hex(const char *token, const int token_len, uint32_t *val);
parser_result_t parser_get_float(const char *token, const int token_len, double *val);
parser_result_t parser_get_macaddr(const char *token, const int token_len, 
                                   parser_macaddr_t *val);
parser_result_t parser_get_ipv4addr(const char *token, const int token_len, 
				    uint32_t *val);
parser_result_t parser_get_file(const char *token, const int token_len, char **val);

#endif /* __PARSER_MATCH_H__ */
