#!/usr/bin/env python

##
# @file      nightly_target.py
# @brief     Add, list, or delete targets of a project (database).
# @version   @verbatim $Id: nightly_target.py 15 2009-03-18 03:16:21Z henry $ @endverbatim
#

import sys, getopt, nightly_mysql

def usage():
    print '\nAdd, list, or delete targets of a project.\n'
    print 'USAGE: nightly_target.py [OPTION] [ARGS] ...\n'
    print '--user [-u] USER    : Specify the database user name.'
    print '--project [-p] PROJ : Specify the project (created by nightly_project.py).'
    print '--add [-a]          : Add a list of targets to a project.'
    print '                      ARGS should be pair (target name, description).'
    print '--list [-l]         : List all targets in a project. ARGS should be empty.'
    print '--delete [-d]       : Delet a list of targets from a project. ARGS should '
    print '                      be a list of targets in the project.'
    print '--help [-h]         : Display this help message.\n'
    print '--add, --list, --delete are mutual exclusive and one must be selected.'

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'u:p:aldh',
                                   ['user=', 'project=', 'add', 'list',
                                    'delete', 'help',])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(-1)

    mode = None
    db_name = None
    db_user = None
    for (ostr, oarg) in opts:
        if (ostr == '--help') or (ostr == '-h'):
            usage()
            sys.exit(0)
        elif (ostr == '--user') or (ostr == '-u'):
            db_user = oarg
        elif (ostr == '--project') or (ostr == '-p'):
            db_name = oarg
        elif (ostr == '--add') or (ostr == '-a'):
            mode = 'add'
        elif (ostr == '--list') or (ostr == '-l'):
            mode = 'list'
        elif (ostr == '--delete') or (ostr == '-d'):
            mode = 'delete'

    if not mode:
        print 'No operation mode selected.\n'
        usage()
        sys.exit(-1)
    elif not db_name:
        print 'No project specified.\n'
        usage()
        sys.exit(-1)
    else:
        db = nightly_mysql.NightlyMySQL(user=db_user, db=db_name)
        if mode == 'add':
            # Add a list of (target, desc) pair.
            n = 0
            while (n+1) < len(args):
                print 'Adding %s...' % args[n]
                db.add_target(args[n], args[n+1])
                n = n + 2
        elif mode == 'list':
            # List all targets
            tgts = db.get_targets()
            print '\n%s:' % db_name
            n = 1
            for t in tgts:
                print '  %2d. %s' % (n, t)
                n = n + 1
        elif mode == 'delete':
            for t in args:
                print 'Deleting %s...' % t
                db.delete_target(t)
        db.disconnect()
            
main()
