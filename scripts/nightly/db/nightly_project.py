#!/usr/bin/env python

##
# @file      nightly_project.py
# @brief     Create, delete projects.
# @version   @verbatim $Id: nightly_project.py 15 2009-03-18 03:16:21Z henry $ @endverbatim
#

import sys, getopt, nightly_mysql

def usage():
    print '\nAdd, list, or delete projects.\n'
    print 'USAGE: nightly_target.py [OPTION] PROJ1 PROJ2 ...\n'
    print '--user [-u] USER    : Specify the database user name.'
    print '--create [-c]       : Create a set of projects in the database.'
    print '                      This results in a set of databases created.'
    print '--delete [-d]       : Delet a list of projects. All tables in their'
    print '                      corresponding databases are removed.'
    print '--help [-h]         : Display this help message.\n'
    print '--create, --delete are mutual exclusive and one must be selected.'

def main():
    try:
        opts, projects = getopt.getopt(sys.argv[1:], 'u:cdh',
                                       ['user=', 'create', 'delete', 'help',])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        sys.exit(-1)

    mode = None
    db_user = 'root'
    for (ostr, oarg) in opts:
        if (ostr == '--help') or (ostr == '-h'):
            usage()
            sys.exit(0)
        elif (ostr == '--user') or (ostr == '-u'):
            db_user = oarg
        elif (ostr == '--project') or (ostr == '-p'):
            db_name = oarg
        elif (ostr == '--create') or (ostr == '-c'):
            mode = 'create'
        elif (ostr == '--delete') or (ostr == '-d'):
            mode = 'delete'

    if not mode:
        print 'No operation mode selected.\n'
        usage()
        sys.exit(-1)
    else:
        if mode == 'create':
            # Create a list of project
            for p in projects:
                print 'Adding %s...' % p
                db = nightly_mysql.NightlyMySQL(user=db_user)
                db.init_db(p)
                db.disconnect()
        elif mode == 'delete':
            for p in projects:
                print 'Deleting %s...' % p
                db = nightly_mysql.NightlyMySQL(user=db_user)
                db.delete_db(p)
                db.disconnect()
            
main()
