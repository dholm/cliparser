/**
 * \file     cparser_token.h
 * \brief    Parser token parsing and completion routine prototypes.
 * \version  \verbatim $Id: cparser_token.h 81 2009-03-20 10:10:22Z henry $ \endverbatim
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

#ifndef __CPARSER_TOKEN_H__
#define __CPARSER_TOKEN_H__

typedef enum {
    CPARSER_NODE_ROOT = 0,
    CPARSER_NODE_END,
    CPARSER_NODE_KEYWORD,
    CPARSER_NODE_STRING,
    CPARSER_NODE_UINT,
    CPARSER_NODE_INT,
    CPARSER_NODE_HEX,
    CPARSER_NODE_FLOAT,
    CPARSER_NODE_MACADDR,
    CPARSER_NODE_IPV4ADDR,
    CPARSER_NODE_FILE,
    CPARSER_MAX_NODES
} cparser_node_type_t;

/**
 * 48-bit MAC address.
 */
typedef struct {
    uint8_t octet[6];
} cparser_macaddr_t;

typedef cparser_result_t (*cparser_match_fn)(const char *token, 
                                             const int token_len, 
                                             cparser_node_t *node, 
                                             int *is_complete);
typedef cparser_result_t (*cparser_complete_fn)(const char *token,
                                                const int token_len);

extern cparser_match_fn cparser_match_fn_tbl[];
extern cparser_complete_fn cparser_complete_fn_tbl[];

/********** Token match functions **********/
cparser_result_t cparser_match_root(const char *token, const int token_len,
                                    cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_end(const char *token, const int token_len,
                                   cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_keyword(const char *token, const int token_len,
                                       cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_string(const char *token, const int token_len,
                                      cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_uint(const char *token, const int token_len,
                                    cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_int(const char *token, const int token_len,
                                   cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_hex(const char *token, const int token_len,
                                   cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_float(const char *token, const int token_len,
                                     cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_macaddr(const char *token, const int token_len,
                                       cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_ipv4addr(const char *token, const int token_len,
                                        cparser_node_t *node, int *is_complete);
cparser_result_t cparser_match_file(const char *token, const int token_len,
                                    cparser_node_t *node, int *is_complete);

/********** Token complete functions **********/
cparser_result_t cparser_complete_file(const char *token, const int token_len);

/********** Token get functions **********/
cparser_result_t cparser_get_string(const char *token, const int token_len, char **val);
cparser_result_t cparser_get_uint(const char *token, const int token_len, uint32_t *val);
cparser_result_t cparser_get_int(const char *token, const int token_len, int32_t *val);
cparser_result_t cparser_get_hex(const char *token, const int token_len, uint32_t *val);
cparser_result_t cparser_get_float(const char *token, const int token_len, double *val);
cparser_result_t cparser_get_macaddr(const char *token, const int token_len, 
                                     cparser_macaddr_t *val);
cparser_result_t cparser_get_ipv4addr(const char *token, const int token_len, 
                                      uint32_t *val);
cparser_result_t cparser_get_file(const char *token, const int token_len, char **val);

#endif /* __CPARSER_MATCH_H__ */
