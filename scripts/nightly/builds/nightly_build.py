#!/usr/bin/env python

##
# @file      nightly_build.py
# @brief     Make nightly builds.
# @version   @verbatim $Id: nightly_build.py 22 2009-03-19 18:50:30Z henry $ @endverbatim
#
# Copyright 2008, Henry Kwok. All rights reserved.
#

import time, os, sys, getopt, re

def usage():
    print 'Build a set of makefile targets and collect build log.\n'
    print 'USAGE: nightly_build.py [OPTIONS] target1 target2 ...\n'
    print '--input [-i] DIR     : Root directory of all targets '
    print '--output [-o] DIR    : Output directory of the build log.'
    print '--makeopts [-m] OPTS : Options passed to make.'
    print '--user [-u] USER     : Database user name.'
    print '--project [-p] PROJ  : Project name.'
    print '--help [-h]          : Display this help message.'
    
def update_log(target, timestr, result, db_user=None, db_name=None):
    """
    Update the build log database
    """
    if db_user and db_name:
        import nightly_mysql
        print 'Logging %s...' % target
        db = nightly_mysql.NightlyMySQL(user=db_user, db=db_name)
        db.add_build_record(target, timestr, result['result'], result['log'])
        db.disconnect()
    return
    
def make_target(target, options, root_dir, output_dir):
    """
    Build a target and return the build log and result.
    """
    sys.stdout.write('Building %s...' % target)
    sys.stdout.flush()
    retval = os.system('cd %s; make %s %s >& %s/build.log' %
                       (root_dir, options, target, output_dir))

    # Read the build log
    f = open('%s/build.log' % output_dir, 'r')
    log = f.read()
    f.close()
    
    if 0 != retval:
        print('fail')
        return { 'result' : False, 'log' : log } # build fails
    print('ok')
    return { 'result' : True, 'log' : log } # build is okay

def main():
    root_dir = os.getcwd()
    output_dir = os.getcwd()
    options = '-s'
    db_user = None
    db_name = None
    
    # Parse input argument
    try:
        opts, targets = getopt.getopt(sys.argv[1:], 'i:o:m:u:p:h',
                                   ['input=', 'output=', 'makeopts=',
                                    'user=', 'project=', 'help'])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(2)

    if ('--help', '') in opts:
        usage()
        sys.exit(0)

    for (ostr, oarg) in opts:
        if (ostr == '--input') or (ostr == '-i'):
            root_dir = oarg
        elif (ostr == '--output') or (ostr == '-o'):
            output_dir = oarg
        elif (ostr == '--user') or (ostr == '-u'):
            db_user = oarg
        elif (ostr == '--project') or (ostr == '-p'):
            db_name = oarg
        elif (ostr == '--makeopts') or (ostr == '-m'):
            options = oarg

    # Make time string
    timestr = time.strftime('%Y-%m-%d %H:%M:%S')

    # Make each target
    for t in targets:
        result = make_target(t, options, root_dir, output_dir)
        if ('clean' == t) or re.search('_tests', t):
            continue # skip if it is clean or a unit test target
        update_log(t, timestr, result, db_user, db_name)
    return

#=== Entry point of the script ===
main()

