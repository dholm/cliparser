#!/usr/bin/env python

##
# USAGE: run_tests.py [bin_dir] [output_dir]
#
# Copyright 2008, Henry Kwok. All rights reserved.
#
# $Id: nightly_run.py 24 2009-03-24 01:04:40Z henry $
#
import sys, os, re, time, getopt

def usage():
    print '\nRun a set of test programs and log the result.\n'
    print 'USAGE: nightly_run.py [OPTS] BIN1 BIN2 ...\n'
    print '--user [-u] USER    : Specify the database user name.'
    print '--project [-p] PROJ : Specify the project (created by nightly_project.py).'
    print '--bin-dir [-b] DIR  : Binary directory'
    print '--help [-h]         : Display this help message.\n'
    
#-----------------------------------------------------------------------
# run_tests - Run each test program and log the output
def run_tests(bin_dir, files):
    # Run each test and log the result
    results = {}
    test_files = []
    for f in files:
        if not re.search('test_(.+)', f):
            continue
        test_files.append(f)
        print('[ %s ]' % f)
        output_file = '/tmp/unit_test.log'
        rc = os.system('%s/%s > %s' % (bin_dir, f, output_file))

        # Read the log
        f_handle = open(output_file, 'r')
        log = f_handle.read()
        f_handle.close()
        
        if 0 != rc:
            print('!!!!! %s crashed !!!!!' % f)
            results[f] = { 'passed' : 0,
                           'failed' : 0,
                           'crashed' : 1,
                           'log' : log }
        else:
            m = re.search('Passed=([0-9]+)\s+Failed=([0-9]+)', log)
            results[f] = { 'passed' : int(m.group(1)),
                           'failed' : int(m.group(2)),
                           'crashed' : 0,
                           'log' : log }
    return results

#-----------------------------------------------------------------------
# gen_test_results - Parse all test program output and generate a test
#     result webpage.
def gen_test_results(target, datestr, results, files, db_user=None, db_name=None):
    # Print the unit test result webpage
    num_passed = 0
    num_failed = 0
    num_crashed = 0
    if db_user and db_name:
        import nightly_mysql
        db = nightly_mysql.NightlyMySQL(user=db_user, db=db_name)
    for f in files:
        num_passed = num_passed + results[f]['passed']
        num_failed = num_failed + results[f]['failed']
        num_crashed = num_crashed + results[f]['crashed']
        if db_user and db_name:
            db.add_unit_test_record(target, datestr, f,
                                    (results[f]['passed'], results[f]['failed'],
                                     results[f]['crashed']),
                                    results[f]['log'])
        else:
            s = '%d/%d/%d' % (results[f]['passed'], results[f]['failed'],
                              results[f]['crashed'])
            print ('%10s: %s' % (s, f))

    if db_user and db_name:
        db.disconnect()
    return (num_passed, num_failed, num_crashed)


#-----------------------------------------------------------------------
# main - Main program
def main():
    datestr = time.strftime('%Y-%m-%d %H:%M:%S')
    bin_dir = os.getcwd()
    target = None

    # Parse all input arguments
    try:
        opts, files = getopt.getopt(sys.argv[1:], 'u:p:t:b:h',
                                   ['user=', 'project=', 'target=',
                                    'bin-dir=', 'help'])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(-1)

    db_name = None
    db_user = None
    target = None
    for (ostr, oarg) in opts:
        if (ostr == '--help') or (ostr == '-h'):
            usage()
            sys.exit(0)
        elif (ostr == '--user') or (ostr == '-u'):
            db_user = oarg
        elif (ostr == '--project') or (ostr == '-p'):
            db_name = oarg
        elif (ostr == '--target') or (ostr == '-t'):
            target = oarg
        elif (ostr == '--bin-dir') or (ostr == '-b'):
            bin_dir = oarg
    
    results = run_tests(bin_dir, files)
    (num_passed, num_failed, num_crashed) = gen_test_results(target, datestr,
                                                             results, files,
                                                             db_user, db_name)
    
    print('Passed=%d  Failed=%d  Crashed=%d' %
          (num_passed, num_failed, num_crashed))
    return

#-----------------------------------------------------------------------
# Entry point of the program
main()
