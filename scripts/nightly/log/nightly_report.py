#!/usr/bin/env python

##
# @file      nightly_report.py
# @brief     Make nightly reports
# @version   @verbatim $Id$ @endverbatim
#
# Copyright 2009, Henry Kwok. All rights reserved.
#

import time, os, sys, getopt, re

def usage():
    print 'Generate a report of activities of a SVN repository.\n'
    print 'USAGE: nightly_report.py [OPTIONS] repo1 repo2 ...\n'
    print '--output [-o] DIR    : Output directory.'
    print '--date [-d] DATE     : The ending date of the query.'
    

class ReportRcFile:
    def __init__(self, out_dir='.'):
        self.fname = out_dir + '/.nightly_report'
        self.last_rev  = 1
        self.num_dir   = 0
        self.num_files = 0
        
    def read(self):
        try:
            cfg_file = open(self.fname, 'r')
            self.last_rev  = int(cfg_file.readline())
            self.num_dir   = int(cfg_file.readline())
            self.num_files = int(cfg_file.readline())
            cfg_file.close()
            retval = True
        except:
            retval = False
        return retval

    def write(self):
        cfg_file = open(self.fname, 'w')
        cfg_file.write('%d\n' % self.last_rev)
        cfg_file.write('%d\n' % self.num_dir)
        cfg_file.write('%d\n' % self.num_files)
        cfg_file.close()

class Revision:
    def __init__(self):
        self.rev  = None
        self.user = None
        self.date = None
        self.time = None
        self.comment  = ''
        self.add_list = []
        self.del_list = []
        self.mov_list = []
        self.mod_list = []

    def html(self):
        msg = '<TR VALIGN="TOP">'
        msg += '<TD>%s</TD><TD>%s</TD>' % (self.date, self.time)
        msg += '<TD>%d</TD><TD>%s</TD>' % (self.rev, self.user)
        msg += '<TD>%s</TD>' % self.comment
        msg += '<TD>'
        if self.mov_list:
            for f in self.mov_list:
                msg += f + '<BR>'
        if self.del_list:
            for f in self.del_list:
                msg += '- ' + f + '<BR>'
        if self.add_list:
            for f in self.add_list:
                msg += '+ ' + f + '<BR>'
        if self.mod_list:
            for f in self.mod_list:
                msg += f + '<BR>'
                
        msg += '</TD></TR>'
        return msg

    def __repr__(self):
        msg = '%d: %s %s by %s\n' % (self.rev, self.date, self.time, self.user)
        msg += '%s\n\n' % self.comment
        return msg

class RevisionSvn(Revision):
    def __init__(self, log):
        Revision.__init__(self)
        assert (len(log) > 0)

        # First line contains a lot of headers
        m = re.search('^r([0-9]+) \| (.+) \| (\S+) ([0-9:]+)', log[0])
        assert m
        self.rev = int(m.group(1))
        self.user = m.group(2)
        self.date = m.group(3)
        self.time = m.group(4)

        # Second line is a fixed string
        assert log[1] == 'Changed paths:\n'

        k = 2
        while True:
            m = re.search('^   ([A|M|D]) (\S+)', log[k])
            if not m: break
            if m.group(1) == 'D':
                self.del_list.append(m.group(2))
            elif m.group(1) == 'M':
                self.mod_list.append(m.group(2))
            elif m.group(1) == 'A':
                mm = re.search('from (\S+):([0-9]+)', log[k])
                if not mm:
                    self.add_list.append(m.group(2))
                else:
                    try:
                        self.del_list.remove(mm.group(1))
                    except:
                        pass
                    self.mov_list.append(mm.group(1) + ' -> ' + m.group(2))
            k = k + 1

        self.comment = '<BR>'.join(log[(k+1):])

def process_one_repo(in_dir, out_dir, date=None):
    if not out_dir:
        out_dir = in_dir

    print 'Processing %s -> %s' % (in_dir, out_dir)
    
    # Initialize a .rc file and read it
    rcfile = ReportRcFile(out_dir)
    rcfile.read()
        
    # Use the current date if none is given
    if not date:
        date = time.strftime('%Y-%m-%d')

    # Get a log from SVN.
    if os.system('svn log -v %s > %s/svn.log' % (in_dir, out_dir)):
        print 'Fail to get SVN log.'
        sys.exit(-1)

    # Parse it
    log_file = open(out_dir + '/svn.log', 'r')
    rev_list = []
    rev_log = []
    for line in log_file:
        if line == '------------------------------------------------------------------------\n':
            if rev_log:
               rev_list.append(RevisionSvn(rev_log))
            rev_log = []
        else:
            rev_log.append(line)
    log_file.close()

    # Generate the HTML
    html_file = open(out_dir + '/svn_log.html', 'w')
    html_file.write('<HTML>\n')
    html_file.write('<TABLE BORDER=1 CELLPADDING=2 RULES="ALL">\n');
    html_file.write('<TR><TH WIDTH>Date</TH><TH WIDTH>Time</TH><TH>Rev</TH>' +
                    '<TH>User</TH><TH>Comments</TH><TH WIDTH=350>Files</TH></TR>')
    for rev in rev_list:
        html_file.write(rev.html() + '\n')
    html_file.write('</TABLE>\n');
    html_file.write('</HTML>\n')
    html_file.close()

def main():
    # Parse input arguments
    try:
        opts, repos = getopt.getopt(sys.argv[1:], 'o:',
                                    ['output=',])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(-1)

    out_dir = None
    for (ostr, oarg) in opts:
        if (ostr == '--output') or (ostr == '-o'):
            out_dir = oarg

    # Process each repo workspace
    for r in repos:
        process_one_repo(r, out_dir)
            

main()
