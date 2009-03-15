/**
 * \file     test_token.c
 * \brief    Test program for verifying token parsing routines.
 * \version  \verbatim $Id: test_token.c 51 2009-03-12 22:33:20Z henry $ \endverbatim
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
#include "parser.h"
#include "parser_priv.h"
#include "parser_token.h"

#define NELEM(a) (sizeof(a)/sizeof(a[0]))

int main (int argc, char *argv[])
{
    struct {
        char               *name;
        char               *str;
        parser_node_type_t type;
        char               *param;
        parser_result_t    result;
        int                is_complete;
    } match_testcases[] = {
        /* Keyword */
        { "KW01", "s", PARSER_NODE_KEYWORD, "show", PARSER_OK, 0 },
        { "KW02", "sh", PARSER_NODE_KEYWORD, "show", PARSER_OK, 0 },
        { "KW03", "sho", PARSER_NODE_KEYWORD, "show", PARSER_OK, 0 },
        { "KW04", "show", PARSER_NODE_KEYWORD, "show", PARSER_OK, 1 },
        { "KW05", "shows", PARSER_NODE_KEYWORD, "show", PARSER_NOT_OK, 0 },
        { "KW06", "how", PARSER_NODE_KEYWORD, "show", PARSER_NOT_OK, 0 },
        /* String */
        { "STR01", "abc", PARSER_NODE_STRING, "s", PARSER_OK, 1 },
        { "STR02", "1234", PARSER_NODE_STRING, "s", PARSER_OK, 1 },
        /* Unsigned integer */
        { "UINT01", "1xabc", PARSER_NODE_UINT, "a", PARSER_NOT_OK, 0 },
        { "UINT02", "0xabc", PARSER_NODE_UINT, "a", PARSER_OK, 1 },
        { "UINT03", "01234", PARSER_NODE_UINT, "a", PARSER_OK, 1 },
        { "UINT04", "012abc", PARSER_NODE_UINT, "a", PARSER_NOT_OK, 0 },
        { "UINT05", "0x", PARSER_NODE_UINT, "a", PARSER_OK, 0 },
        { "UINT06", "1x", PARSER_NODE_UINT, "a", PARSER_NOT_OK, 0 },
        /* Signed integer */
        { "INT01", "123", PARSER_NODE_INT, "a", PARSER_OK, 1 },
        { "INT02", "9876543210", PARSER_NODE_INT, "a", PARSER_OK, 1},
        { "INT03", "abc", PARSER_NODE_INT, "a", PARSER_NOT_OK, 0 },
        { "INT04", "-980", PARSER_NODE_INT, "a", PARSER_OK, 1 },
        { "INT05", "-", PARSER_NODE_INT, "a", PARSER_OK, 0 },
        { "INT06", "+", PARSER_NODE_INT, "a", PARSER_OK, 0 },
        { "INT07", "+5678", PARSER_NODE_INT, "a", PARSER_OK, 1 },
        { "INT08", "1324-80", PARSER_NODE_INT, "a", PARSER_NOT_OK, 0 },
        /* Hexadecimal */
        { "HEX01", "0x78abcdef", PARSER_NODE_HEX, "a", PARSER_OK, 1 },
        { "HEX02", "0x0123456", PARSER_NODE_HEX, "a", PARSER_OK, 1 },
        { "HEX03", "1x1234", PARSER_NODE_HEX, "a", PARSER_NOT_OK, 0 },
        { "HEX04", "0x", PARSER_NODE_HEX, "a", PARSER_OK, 0 },
        { "HEX05", "0xyz", PARSER_NODE_HEX, "a", PARSER_NOT_OK, 0 },
        /* Float */
        /* MAC address */
        { "MAC01", "0", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC02", "00", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC03", "00:", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC04", "00:1", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC05", "00:11", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC06", "00:11:", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC07", "00:11:9", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC08", "00:11:99", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC09", "00:11:99:", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC10", "00:11:99:a", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC11", "00:11:99:aa", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC12", "00:11:99:aa:", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC13", "00:11:99:aa:C", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC14", "00:11:99:aa:CC", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC15", "00:11:99:aa:CC:", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 0 },
        { "MAC16", "00:11:99:aa:CC:F", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 1 },
        { "MAC17", "00:11:99:aa:CC:Ff", PARSER_NODE_MACADDR, "macaddr", PARSER_OK, 1 },
        { "MAC18", "xx:yy:zz:pp:qq:rr", PARSER_NODE_MACADDR, "macaddr", PARSER_NOT_OK, 0 },
        /* IP address */
        { "IP4_01", "1", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 0 },
        { "IP4_02", "10.", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 0 },
        { "IP4_03", "10.123", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 0 },
        { "IP4_04", "10.123.0.", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 0 },
        { "IP4_05", "10.123.0.1", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 1 },
        { "IP4_06", "10.123.0.13", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_OK, 1 },
        { "IP4_07", "1234.1.1.1", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_NOT_OK, 0 },
        { "IP4_08", "1.2.3.3.4.5", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_NOT_OK, 0 },
        { "IP4_09", "a.b.c.d", PARSER_NODE_IPV4ADDR, "ipv4", PARSER_NOT_OK, 0 },
        /* File path */
        { "FILE01", "test.txt", PARSER_NODE_FILE, "fname", PARSER_OK, 1 },
        { "FILE02", "/usr/include/stdio.h", PARSER_NODE_FILE, "fname", PARSER_OK, 1 },
    };
    int n, is_complete, num_tests = 0, num_passes = 0;
    int total_tests = 0, total_passes = 0;
    parser_result_t result;
    parser_node_t node;

    for (n = 0; n < NELEM(match_testcases); n++) {
        node.type = match_testcases[n].type;
        node.param = match_testcases[n].param;
        is_complete = 0xff;
        assert(parser_match_fn_tbl[node.type]);
        result = parser_match_fn_tbl[node.type](match_testcases[n].str, 
                                                strlen(match_testcases[n].str), 
                                                &node, &is_complete);
        num_tests++;

        if ((result == match_testcases[n].result) && 
            (is_complete == match_testcases[n].is_complete)) {
            printf("PASS: %s: ", match_testcases[n].name);
            num_passes++;
        } else {
            printf("FAIL: %s: ", match_testcases[n].name);
        }
        printf("'%s' -> (%d, %s) -> (%s, %s)\n", match_testcases[n].str,
               match_testcases[n].type, match_testcases[n].param, 
               (PARSER_OK == result ? "OK" : "NOT OK"),
               (is_complete ? "COMPLETE" : "INCOMPLETE"));
    }
    printf("Match function tests: %d total. %d passed.\n\n\n", 
           num_tests, num_passes);
    total_tests = num_tests;
    total_passes = num_passes;

    /* 
     * Test the get functions. Due to the output argument of each 
     * function being different. There is no table to simplify this.
     */
    num_tests = num_passes = 0;

    /* String */
    char *str = "hello, world", *str_val;
    result = parser_get_string(str, strlen(str), &str_val);
    num_tests++;
    if ((PARSER_OK != result) || (str_val != str)) {
        printf("FAIL: STR01: %s -> %s\n", str, str_val);
    } else {
        printf("PASS: STR01: %s -> %s\n", str, str_val);
        num_passes++;
    }

    /* Unsigned integer */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
        uint32_t         val;
    } get_uint_testcases[] = {
        { "UINT01", "0x1234abCD", PARSER_OK, 0x1234abcd },
        { "UINT02", "12345678", PARSER_OK, 12345678 },
        { "UINT03", "0xabcdef123", PARSER_NOT_OK, 0 },
        { "UINT04", "9876543210", PARSER_NOT_OK, 0 },
        { "UINT05", "0x0000001234", PARSER_OK, 0x1234 },
        { "UINT06", "00000000000001234", PARSER_OK, 1234 }
    };
    uint32_t uint_val;
    for (n = 0; n < NELEM(get_uint_testcases); n++) {
        result = parser_get_uint(get_uint_testcases[n].str,
                                 strlen(get_uint_testcases[n].str),
                                 &uint_val);
        num_tests++;
        if ((result != get_uint_testcases[n].result) || 
            (uint_val != get_uint_testcases[n].val)) {
            printf("FAIL: %s: ", get_uint_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_uint_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %lu (%lX)\n", get_uint_testcases[n].str, 
               (unsigned long)uint_val, (unsigned long)uint_val);
    }

    /* Integer */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
        int32_t          val;
    } get_int_testcases[] = {
        { "INT01", "123", PARSER_OK, 123 },
        { "INT02", "12345678", PARSER_OK, 12345678 },
        { "INT03", "12345678900", PARSER_NOT_OK, 0 },
        { "INT04", "9876543210", PARSER_NOT_OK, 0 },
        { "INT05", "00000000000001234", PARSER_OK, 1234 },
        { "INT06", "+000777777", PARSER_OK, 777777 },
        { "INT07", "-000777777", PARSER_OK, -777777 },
        { "INT08", "+2147483647", PARSER_OK, 2147483647L },
        { "INT09", "+2147483648", PARSER_NOT_OK, 0 },
        { "INT10", "-2147483648", PARSER_OK, -2147483648LL },
        { "INT11", "-2147483649", PARSER_NOT_OK, 0 },
    };
    int32_t int_val;
    for (n = 0; n < NELEM(get_int_testcases); n++) {
        result = parser_get_int(get_int_testcases[n].str,
                                 strlen(get_int_testcases[n].str),
                                 &int_val);
        num_tests++;
        if ((result != get_int_testcases[n].result) || 
            (int_val != get_int_testcases[n].val)) {
            printf("FAIL: %s: ", get_int_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_int_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %ld\n", get_int_testcases[n].str, (long int)int_val);
    }

    /* Hexadecimal */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
        uint32_t         val;
    } get_hex_testcases[] = {
        { "HEX01", "0x1234abCD", PARSER_OK, 0x1234abcd },
        { "HEX02", "0xabcdef123", PARSER_NOT_OK, 0 },
        { "HEX03", "0x0000001234", PARSER_OK, 0x1234 },
    };
    for (n = 0; n < NELEM(get_hex_testcases); n++) {
        result = parser_get_hex(get_hex_testcases[n].str,
                                strlen(get_hex_testcases[n].str),
                                &uint_val);
        num_tests++;
        if ((result != get_hex_testcases[n].result) || 
            (uint_val != get_hex_testcases[n].val)) {
            printf("FAIL: %s: ", get_hex_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_hex_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %lX\n", get_hex_testcases[n].str, 
               (unsigned long)uint_val);
    }

    /* Float */

    /* MAC address */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
        uint8_t          macaddr[6];
    } get_macaddr_testcases[] = {
        { "MAC01", "12:45:89:ab:CD:Ef", PARSER_OK, 
          { 0x12, 0x45, 0x89, 0xab, 0xcd, 0xef } },
        { "MAC02", "1:4:9:a:C:f", PARSER_OK, 
          { 0x01, 0x04, 0x09, 0x0a, 0x0c, 0x0f } },
    };
    parser_macaddr_t macaddr;
    for (n = 0; n < NELEM(get_macaddr_testcases); n++) {
        result = parser_get_macaddr(get_macaddr_testcases[n].str,
                                    strlen(get_macaddr_testcases[n].str),
                                    &macaddr);
        num_tests++;
        if ((result != get_macaddr_testcases[n].result) || 
            (memcmp(&macaddr, get_macaddr_testcases[n].macaddr, 6))) {
            printf("FAIL: %s: ", get_macaddr_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_macaddr_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %02X:%02X:%02X:%02X:%02X:%02X\n", 
               get_macaddr_testcases[n].str, macaddr.octet[0], 
               macaddr.octet[1], macaddr.octet[2], macaddr.octet[3], 
               macaddr.octet[4], macaddr.octet[5]);
    }

    /* IPv4 address */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
        uint32_t         ipv4addr;
    } get_ipv4addr_testcases[] = {
        { "IP4_01", "192.168.1.1", PARSER_OK, 0xc0a80101 },
        { "IP4_02", "333.1.1.1", PARSER_NOT_OK, 0x0 },
    };
    uint32_t ipv4addr;
    for (n = 0; n < NELEM(get_ipv4addr_testcases); n++) {
        result = parser_get_ipv4addr(get_ipv4addr_testcases[n].str,
                                     strlen(get_ipv4addr_testcases[n].str),
                                     &ipv4addr);
        num_tests++;
        if ((result != get_ipv4addr_testcases[n].result) || 
            (memcmp(&ipv4addr, &get_ipv4addr_testcases[n].ipv4addr, 4))) {
            printf("FAIL: %s: ", get_ipv4addr_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_ipv4addr_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %d.%d.%d.%d\n", get_ipv4addr_testcases[n].str, 
               (int)(ipv4addr >> 24) & 0xff, (int)(ipv4addr >> 16) & 0xff, 
               (int)(ipv4addr >> 8) & 0xff, (int)ipv4addr & 0xff);
    }

    /* File path */
    struct {
        char             *name;
        char             *str;
        parser_result_t  result;
    } get_file_testcases[] = {
        { "FILE01", "/usr/include/stdlib.h", PARSER_OK },
        { "FILE02", "/tmp/xyz.parser", PARSER_NOT_OK },
        { "FILE03", "/usr", PARSER_NOT_OK }
    };
    char *file;
    for (n = 0; n < NELEM(get_file_testcases); n++) {
        result = parser_get_file(get_file_testcases[n].str,
                                 strlen(get_file_testcases[n].str),
                                 &file);
        num_tests++;
        if ((result != get_file_testcases[n].result) || 
            ((PARSER_OK == result) && 
             strcmp(file, get_file_testcases[n].str))) {
            printf("FAIL: %s: ", get_file_testcases[n].name);
        } else {
            printf("PASS: %s: ", get_file_testcases[n].name);
            num_passes++;
        }
        printf("%s -> %s\n", get_file_testcases[n].str, 
               (PARSER_OK == result ? "OK" : "NOT OK"));
    }
    printf("Get function tests: %d total. %d passed.\n\n", 
           num_tests, num_passes);

    total_tests += num_tests;
    total_passes += num_passes;
    printf("Total=%d  Passed=%d  Failed=%d\n", total_tests,
           total_passes, total_tests - total_passes);
    return 0;
}
