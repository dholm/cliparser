#!/usr/bin/python
# $Id: mk_parser.py 55 2009-03-13 08:11:49Z henry $

# Copyright (c) 2008, Henry Kwok
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the project nor the names of its contributors 
#       may be used to endorse or promote products derived from this software 
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY HENRY KWOK ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL HENRY KWOK BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re, sys, glob

print_tree = True
debug = False
end_node = None

# DBG - Debug printf
def DBG(hdr, s):
    if debug:
        sys.stdout.write(hdr)
        print(s)
        
# node - A class of parse tree node
class NODE:
    NODE_TYPES = { 'ROOT'       : None,
                   'END'        : None,
                   'KEYWORD'    : None,
                   'STRING'     : 'char *',
                   'UINT'       : 'uint32_t ',
                   'INT'        : 'int32_t ',
                   'HEX'        : 'uint32_t ',
                   'FLOAT'      : 'double ',
                   'MACADDR'    : 'parser_macaddr_t ',
                   'IPV4ADDR'   : 'uint32_t ',
                   'FILE'       : 'char *'
                   }
    def __init__(self, node_type, param, desc, flags):
        self.type = node_type
        self.param = param
        self.desc = desc
        self.flags = flags
        self.children = []

        # Cannot fill these out until we insert the node to the tree
        self.parent = None
        self.depth = 0
        self.path = ''
        self.next = None
        return

    # add_child - Add a child node to a tree node given its priority
    def add_child(self, child):
        for c in self.children:
            if (c.type == child.type) and (c.param == child.param):
                # The node already exists. Re-use the existing node.
                return c
        # Fill out some information that are tree structure dependent.
        # These information are actually embedded in the tree already
        # However, we compute them and cache them to reduce the
        # processing time later. This approach increases the storage
        # requirement but decreases the processing time.
        child.parent = self
        child.depth = self.depth + 1
        if child.is_param() or child.is_keyword():
            child.path = self.path + '_' + child.param.replace('-','_')
        elif 'END' == child.type:
            child.path = self.path + '_eol'
        elif 'ROOT' == child.type:
            # If we are adding a child ROOT node, the parent ('self' here)
            # must be an END node. For END node, we insert a string
            # 'parser_glue' to the front of self.path. So, we must
            # remove it first before creating the submode root path
            child.path = self.param.replace('parser_glue', '') + '_root'
        if len(self.children) > 0:
            self.children[-1].next = child
        
        # Insert the node into the children list
        self.children.append(child)
        return child

    # Generate indentation based
    def gen_indent(self):
        for n in range(0, self.depth):
            print('  ')

    # is_param - Return True if it is a parameter node; False otherwise.
    def is_param(self):
        if (('ROOT' == self.type) or ('END' == self.type) or ('KEYWORD' == self.type)):
            return False
        return True

    # is_keyword - Return True if it is a keyword node; False otherwise.
    def is_keyword(self):
        if ('KEYWORD' == self.type):
            return True
        return False
    
    # display - Display a summary of the node
    def display(self):
        if 'ROOT' == self.type:
            sys.stdout.write('<ROOT> ')
        elif 'KEYWORD' == self.type:
            sys.stdout.write('<KEYWORD:%s> ' % self.param)
        elif 'END' == self.type:
            sys.stdout.write('<END> ')
        else:
            sys.stdout.write('<%s:%s> ' % (self.type, self.param))

    # walk - Walk the tree
    def walk(self, fn, mode, cookie):
        last = len(self.children)
        if 'pre-order' == mode:
            fn(self, cookie)
            for n in range(0, last):
                self.children[n].walk(fn, mode, cookie)
        if 'post-order' == mode:
            for n in range(0, last):
                self.children[last-1-n].walk(fn, mode, cookie)
            fn(self, cookie)
    
    # gen_c_struct - Generate the C structure name.
    def gen_c_struct(self, fout):
        if self.parent == None:
            fout.write('parser_node_t parser_root = {\n')
        else:
            fout.write('parser_node_t parser_node%s = {\n' % self.path)
        # type
        fout.write('    PARSER_NODE_%s,\n' % self.type)
        # flags
        if len(self.flags) == 0:
            fout.write('    0,\n')
        else:
            fout.write('    %s' % self.flags[0])
            for k in range(1,len(self.flags)):
                fout.write(' | %s' % self.flags[k])
            fout.write(',\n')
        # param
        if 'ROOT' == self.type:  fout.write('    NULL,\n')
        elif 'END' == self.type: fout.write('    %s,\n' % self.param)
        elif 'KEYWORD' == self.type: fout.write('    "%s",\n' % self.param)
        else: fout.write('    "<%s:%s>",\n' % (self.type, self.param))
        # desc
        if self.desc:
            fout.write('    "%s",\n' % self.desc)
        else:
            fout.write('    NULL,\n')
        # sibling
        if self.next:
            fout.write('    &parser_node%s,\n' % self.next.path)
        else:
            fout.write('    NULL,\n')
        # children
        if len(self.children) > 0:
            fout.write('    &parser_node%s\n' % self.children[0].path)
        else:
            fout.write('    NULL\n');
        fout.write('};\n')

    # gen_action_fn - Generate the action function.
    def gen_action_fn(self, fout):
        # Build a list of parse node that forms a path between root
        # to this end node
        p = []
        cur_node = self
        while cur_node.parent:
            p.insert(0, cur_node)
            cur_node = cur_node.parent
            if cur_node.type == 'ROOT':
                break

        # Declare the action function
        fout.write('parser_result_t %s(parser_context_t *context' %
                   self.param.replace('parser_glue', 'parser_cmd'))

        # Declare the variable list
        skip = False
        for k in range(0, len(p)):
            if p[k].is_param():
                skip = True
                val_type = NODE.NODE_TYPES[p[k].type]
                fout.write(',\n')
                fout.write('    %s*%s_ptr' % (val_type, p[k].param))
        fout.write(');\n')

    # gen_glue_fn - Generate the glue function.
    def gen_glue_fn(self, fout):
        # Build a list of parse node that forms a path between root
        # to this end node
        p = []
        cur_node = self
        while cur_node.parent:
            p.insert(0, cur_node)
            cur_node = cur_node.parent
            if cur_node.type == 'ROOT':
                break

        # Build the glue function
        fout.write('parser_result_t\n')
        fout.write('%s (parser_t *parser)\n' % self.param)
        fout.write('{\n')

        # Declare the variable list
        skip = False
        for k in range(0, len(p)):
            if p[k].is_param():
                skip = True
                val_type = NODE.NODE_TYPES[p[k].type]
                fout.write('    %s%s_val;\n' % (val_type, p[k].param))
                fout.write('    %s*%s_ptr = NULL;\n' % (val_type, p[k].param))
        if skip: fout.write('\n')
        
        # Extract the parameters
        for k in range(0, len(p)):
            if p[k].is_param():
                fout.write('    if (PARSER_OK == \n')
                fout.write('        parser_get_%s(parser->tokens[%d].buf,\n' %
                           (p[k].type.lower(), k))
                fout.write('            parser->tokens[%d].token_len, &%s_val)) {\n' %
                           (k, p[k].param))
                fout.write('        %s_ptr = &%s_val;\n' %
                           (p[k].param, p[k].param))
                fout.write('    } else if (%d <= parser->token_tos) {\n' % (k+1))
                fout.write('        printf("ERROR: %s is not a valid %s.\\n");\n' %
                           (p[k].param, p[k].type))
                fout.write('        return PARSER_NOT_OK;\n')
                fout.write('    }\n')

        # Call the user-provided action function
        fout.write('    %s(&parser->context' % self.param.replace('parser_glue', 'parser_cmd'))
        for k in range(0, len(p)):
            if p[k].is_param():
                fout.write(',\n        %s_ptr' % p[k].param)
        fout.write(');\n')
        fout.write('    return PARSER_OK;\n')
        fout.write('}\n\n')

# add_cli - Add one line of CLI to the parse tree
def add_cli(root, line, line_num, comment):
    global end_node
    nodes = []
    flags = []
    num_opt_start = 0

    # Convert a line into a token list
    line = line.replace('\n','')
    DBG('\nDBG:add_cli:1>> ', line)
    tokens = line.split(' ')

    # Delete all token that is ''
    cnt = tokens.count('')
    for k in range(0, cnt):
        tokens.remove('')
    if len(tokens) == 0:
        return root # this is a blank line. quit

    # Check each token to make sure that it is valid
    for t in tokens:
        m = re.search('\<(.+):(.+)>\>', t)
        if m:
            # A parameter. Check type and variable name
            if not (m.group(1) in NODE.NODE_TYPES.keys()):
                print('line %d: Unknown parameter type "%s".' %
                      (line_num, m.group(1)))
                sys.exit(-1)
            if not re.search('[a-zA-Z]([a-zA-Z0-9_]*)', m.group(2)):
                print('line %d: Invalid parameter name "%s".' %
                      (line_num, m.group(2)))
                sys.exit(-1)
        elif (t == '{') or (t == '}'):
            continue
        else:
            if not re.search('[a-zA-Z0-9_-]+', t):
                print 'line %d: Invalid keyword "%s".' % (line_num, t)
                sys.exit(-1)

    for t in tokens:
        # Parse each token
        # Look for '{'
        if '{' == t:
            flags.append('PARSER_NODE_FLAGS_OPT_START')
            num_opt_start = num_opt_start + 1
            continue
        # Look for '}'
        if '}' == t:
            nodes[len(nodes)-1].flags.append('PARSER_NODE_FLAGS_OPT_END')
            continue
        flags = []
        # Look for a parameter 
        m1 = re.search('\<(.+):(.+)\>', t)
        m2 = re.search('\<(.+):(.+):(.+)\>', t)
        if m1 or m2:
            if m2:
                node_type = m2.group(1)
                param = m2.group(2)
                desc = m2.group(3)
            else:
                desc = None
                node_type = m1.group(1)
                param = m1.group(2)
            nodes.append(NODE(node_type, param, desc, flags))
            continue
        # Look for a keyword
        m = re.search('(.+)', t)
        if m:
            nodes.append(NODE('KEYWORD', m.group(1), None, flags))

    # hack alert - Check that if there are optional parameters, the format is ok
    DBG('DBG:add_cli:2>> ', tokens)

    if debug:
        DBG('DBG:add_cli:3>> ', '')
        for nn in nodes:
            nn.display()
        sys.stdout.write('\n')

    # We need to create the glue function name. Since the path of the node
    # is set up when the node is inserted, we don't have it here. So,
    # we manually walk all the nodes and generate the glue function name
    glue_fn = 'parser_glue' + root.param
    for n in nodes:
        glue_fn = glue_fn + '_' + n.param.replace('-','_')
    
    # Insert them into the parse tree
    for k in range(0, num_opt_start+1):
        DBG('DBG:add_cli:4>> ', k)
        num_braces = 0
        drop = False
        cur_node = root
        if num_opt_start == k:
            end_node = NODE('END', glue_fn, comment, [])
        else:
            end_node = NODE('END', glue_fn, None, ['PARSER_NODE_FLAGS_OPT_PARTIAL'])
        for n in nodes:
            if drop:
                continue
            if (n.flags.count('PARSER_NODE_FLAGS_OPT_START') +
                n.flags.count('PARSER_NODE_FLAGS_OPT_END') > 0):
                if num_braces == k:
                    drop = True
                num_braces = num_braces + 1
            if debug:
                cur_node.display()
                sys.stdout.write('-> ')
                n.display()
                sys.stdout.write('\n')
            cur_node = cur_node.add_child(n)
        if debug:
            cur_node.display()
            sys.stdout.write('-> ')
            end_node.display()
            sys.stdout.write('\n')
        cur_node.add_child(end_node)
    return root

# process_cli_file - Process one .cli file. This includes handling
#     all preprocessors directive.
def process_cli_file(filename, root, mode, labels):
    num_disable = 0
    label_stack = []
    deplist = []
    comment = None
    line = ''
    last_cli_root = None
    last_cli_end = None
    submode = False
    try:
        fin = open(filename, 'r')
        line_num = 0
        for line in fin:
            line_num = line_num + 1

            # Process the file line by line. The orderof processing
            # These different directives is extremely important. And the
            # order below is not arbitrary.
            #
            # We must process #endif not matter what. Otherwise, once
            # we start a #ifdef block that is omitted, we'll never be able
            # to terminate it. Then, we omit every other type of line as long
            # as there is at least one disable #ifdef left. Afterward, we
            # check for illegal directives. A normal command line is handled
            # last. Illegal tokens are checked inside add_cli().
            
            # #endif
            m = re.search('^#endif', line)
            if m:
                if len(label_stack) == 0:
                    print('line %d: Unmatched #ifdef/#ifndef' % line_num)
                    sys.exit(-1)
                num_disable = num_disable - label_stack.pop(0)[1]
                continue
            # Skip the rest of processing because some #ifdef/#ifndef is
            # keeping this line from being processed.
            if (num_disable > 0):
                continue
            # Check for illegal preprocessor directives
            if (re.search('^#', line) and
                (not re.search('^#ifdef(\S*\/\/.*)*', line) and
                 not re.search('^#submode(\S*\/\/.*)*', line) and
                 not re.search('^#endsubmode(\S*\/\/.*)*', line) and
                 not re.search('^#include(\S*\/\/.*)*', line))):
                print('line %d: Unknown preprocessor directive.' % line_num)
                sys.exit(-1)
            # Comment
            m = re.search('^\s*\/\/\s*(.*)', line)
            if m:
                if 'compile' == mode:
                    comment = m.group(1)
                if 'preprocess' == mode:
                    sys.stdout.write(line)
                continue
            # #ifdef
            m = re.search('^#ifdef (.+)', line)
            if m:
                l = m.group(1)
                val = 0
                if not labels.has_key(l):
                    val = 1
                    num_disable = num_disable + 1
                label_stack.insert(0, [l, val])
                continue
            # #ifndef
            m = re.search('^#ifndef (.+)', line)
            if m:
                l = m.group(1)
                val = 0
                if labels.has_key(m.group(1)):
                    val = 1
                    num_disable = num_disable + 1
                label_stack.insert(0, [l, val])
                continue
            # #include
            m = re.search('^#include "(.+)"', line)
            if m:
                if len(glob.glob(m.group(1))) == 0:
                    print('line %d: file %s does not exist.' %
                          (line_num, m.group(1)))
                    sys.exit(-1)
                if ('compile' == mode):
                    process_cli_file(m.group(1), root, mode, labels)
                elif ('mkdep' == mode):
                    deplist.append(m.group(1))
                elif ('preprocess' == mode):
                    process_cli_file(m.group(1), root, mode, labels)
                else:
                    print('line %d: unknown mode %s' % (line_num, mode))
                continue
            # #submode
            m = re.search('^#submode "(.+)"', line)
            if m:
                if submode:
                    print('line %d: nested submode is invalid.' % line_num)
                else:
                    submode = True
                    last_cli_root = NODE('ROOT', '_' + m.group(1),
                                         'Root of submode %s' % m.group(1), [])
                    last_cli_end.add_child(last_cli_root)
                continue
            # #endsubmode
            m = re.search('^#endsubmode', line)
            if m:
                if not submode:
                    print('line %d: #endsubmode without a #submode.' %
                          line_num)
                    sys.exit(-1)
                else:
                    submode = False
                    last_cli_root = None
                    last_cli_end = None
                continue
            # What survive must be either an empty line or a command
            if ('compile' == mode):
                if not submode:
                    root = add_cli(root, line, line_num, comment)
                    last_cli_end = end_node
                else:
                    last_cli_root = add_cli(last_cli_root, line, line_num, comment)
            elif ('preprocess' == mode):
                sys.stdout.write(line)
            comment = None
        if 'compile' == mode:
            if print_tree:
                root.walk(walker_gen_dbg, 'pre-order', sys.stdout)
        elif 'mkdep' == mode:
            sys.stdout.write('%s:' % filename)
            for d in deplist: sys.stdout.write(' %s' % d)
            sys.stdout.write('\n')
    finally:
        fin.close()
    return root

# walker_gen_dbg - Display each node
def walker_gen_dbg(node, fout):
    for n in range(0, node.depth):
        fout.write('  ')
    node.display()
    fout.write('\n')
    return

# walker_gen_glue - Generate glue functions
def walker_gen_glue(node, fout):
    if (('END' != node.type) or
        (node.flags.count('PARSER_NODE_FLAGS_OPT_PARTIAL') > 0)):
        return
    node.gen_glue_fn(fout)
    return

# walker_gen_action - Generate action functions
def walker_gen_action(node, fout):
    if (('END' != node.type) or
        (node.flags.count('PARSER_NODE_FLAGS_OPT_PARTIAL') > 0)):
        return
    node.gen_action_fn(fout)
    
# walker_gen_struct - Generate C structure for parse tree ndoes
def walker_gen_struct(node, fout):
    node.gen_c_struct(fout)
    return

# main - Process the command-line argument
def main():
    filelist = []
    labels = {}
    mode = 'compile'
    out_dir = '.'
    # Parse input arguments
    sys.argv.pop(0) # remove mk_parser.py itself
    while (len(sys.argv) > 0):
        item = sys.argv.pop(0)
        if '-MM' == item:
            mode = 'mkdep'
        elif '-P' == item:
            mode = 'preprocess'
        elif '-D' == item:
            l = sys.argv.pop(0)
            labels[l] = 0;
        elif '-o' == item:
            out_dir = sys.argv.pop(0)
        else:
            filelist.append(item)

    # Process each file
    root = NODE('ROOT', '', 'Root node of the parser tree', [])
    for f in filelist:
        if ('mkdep' != mode):
            print('Processing %s...' % f)
        root = process_cli_file(f, root, mode, labels)

    # Generate .c file that contains glue functions and parse tree
    fout = open('%s/parser_tree.c' % out_dir, 'w')
    fout.write('/*----------------------------------------------------------------------\n')
    fout.write(' * This file is generated by mk_parser.py.\n')
    fout.write(' *----------------------------------------------------------------------*/\n')
    fout.write('#include <stdint.h>\n')
    fout.write('#include <stdio.h>\n')
    fout.write('#include "parser.h"\n')
    fout.write('#include "parser_priv.h"\n')
    fout.write('#include "parser_token.h"\n')
    fout.write('#include "parser_tree.h"\n\n')
    
    root.walk(walker_gen_glue, 'pre-order', fout)
    root.walk(walker_gen_struct, 'post-order', fout)
    fout.close()

    fout = open('%s/parser_tree.h' % out_dir, 'w')
    fout.write('/*----------------------------------------------------------------------\n')
    fout.write(' * This file is generated by mk_parser.py.\n')
    fout.write(' *----------------------------------------------------------------------*/\n')
    fout.write('#ifndef __PARSER_TREE_H__\n')
    fout.write('#define __PARSER_TREE_H__\n\n')
    fout.write('extern parser_node_t parser_root;\n\n')
    root.walk(walker_gen_action, 'pre-order', fout)
    fout.write('\n#endif /* __PARSER_TREE_H__ */\n')
    fout.close()

    return

# Entry point of the script
main()

