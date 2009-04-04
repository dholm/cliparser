##
# @file      nightly_mysql.py
# @brief     SQL driver for MySQL
# @version   @verbatim $Id: nightly_mysql.py 19 2009-03-18 04:10:47Z henry $ @endverbatim
#

# You need to install MySQL for Python package. All scripts in the
# Nightly were tested using version 1.2.2 of this package. You can
# find this in SourceForget:
#
# http://sourceforge.net/projects/mysql-python
#
import MySQLdb, re

class NightlyMySQL:
    DEFAULT_PORT = 3306
    DEBUG = False
    
    def __init__(self, host=None, port=None, user=None, passwd=None, db=None):
        if None == host:
            host = 'localhost'
        if None == port:
            port = NightlyMySQL.DEFAULT_PORT
        if db == None:
            db = 'test'
        if (None != user):
            self.con = MySQLdb.connect(host=host, port=port, user=user, db=db)
            self.cursor = self.con.cursor()
        else:
            self.con = None
            self.cursor = None
        return

    def DBG(self, flag, msg):
        if False == flag: return
        if (None == flag) and (False == NightlyMySQL.DEBUG): return
        print msg
        return
    
    def connect(self, host, port, user, passwd, db):
        """
        Connect to a MySQL database.

        connect(host, port, user, passwd, db) connects to a MySQL database
        at (host, port) using a (MySQL) user id and password.

        host   - Name string of the MySQL server.
        port   - Server port number. If omitted 3306 is used.
        user   - MySQL user id.
        passwd - Password.
        db     - Database to connect to.
        """
        if None == host:
            host = 'localhost'
        if None == port:
            port = NightMySQL.DEFAULT_PORT
        self.con = MySQLdb.connect(host=host, port=port, user=user, db=db)
        self.cursor = self.con
        return

    def disconnect(self):
        """
        Disconnect from a MySQL database.

        disconnect() disconnects from a MySQL database.
        """
        self.con.close()
        return
    
    def execute(self, cmd):
        self.DBG(None, cmd)
        self.cursor.execute(cmd)
        return self.cursor.fetchall()

    def create_db(self, db_name):
        """
        Create a database for a project. Each project should use its own
        database.
        """
        return
    
    def init_db(self, db_name):
        """
        Create a database for a project. Each project should use its own
        database. Initialize a target table in the database.

        init_db() creates a database and a 'Targets' table in the database
        which holds all targets to be built and regression tested.
        """
        self.execute('CREATE DATABASE %s' % db_name)
        self.execute('USE %s' % db_name)
        self.execute('CREATE TABLE Targets(Name TINYTEXT, Description TEXT)')
        return

    def delete_db(self, db_name):
        """
        Delete a database from a project.
        """
        self.execute('DROP DATABASE %s' % db_name)
        return

    def add_target(self, target, desc):
        """
        Add a target to the target table in the database.

        add_target(target, desc) adds a new target to the Target table
        and creates two new tables. The first table is called
        'Build_[target]' which records results of (nightly) build. The
        second table is called 'Unit_Test_[target]' which contains
        results of unit test regression results.

        target - Target name string.
        desc   - Description of the target.
        """

        self.execute('INSERT INTO Targets VALUE ("%s", "%s")' % (target, desc))
        self.execute('CREATE TABLE Build_%s(Date DATETIME, Result TINYINT, ' % target +
                     'Log MEDIUMTEXT)')
        self.execute('CREATE TABLE Unit_Test_%s(Date DATETIME, ' % target + 
                     'Name TINYTEXT, Passed SMALLINT UNSIGNED, ' +
                     'Failed SMALLINT UNSIGNED, Crashed SMALLINT UNSIGNED, ' +
                     'Log MEDIUMTEXT)')
        return

    def get_targets(self):
        """
        Get all targets in the Targets table.
        """
        results = self.execute('SELECT Name FROM Targets')
        targets = []
        for r in results:
            targets.append(r[0])
        return targets
    
    def delete_target(self, target):
        """
        Remove a target from the target table in the database.

        delete_target(target) removes an existing target from the Target
        table.
        
        target - Target name string.
        """

        self.execute('DROP TABLE Build_%s' % target)
        self.execute('DROP TABLE Unit_Test_%s' % target)
        self.execute('DELETE FROM Targets WHERE Name="%s"' % target)
        return

    def add_build_record(self, target, datetime, result, log):
        """
        Add a build record of a target to 'Build_[target]' table.

        add_build_record(target, datetime, result, log) inserts a row to
        the 'Build_[target]' table. The row contains the date, the
        result and a build log.

        target - Target name string.
        result - True if build is successful; False otherwise.
        log    - A string that is the build log itself.
        """

        if result:
            pass_or_fail = 1
        else:
            pass_or_fail = 0
        # We need to replace all " with \" so that MySQL will handle
        # them correctly. 
        log = re.sub('"', '\\"', log)
        self.execute('INSERT INTO Build_%s VALUES ("%s", %d, "%s")' %
                     (target, datetime, pass_or_fail, log))
        return

    def get_build_records(self, target):
        """
        Get all build record of a target.

        get_build_record(target) returns all build records of a target.
        Build records are returned as a list of (datetime, pass_or_fail)
        where datetime is a datetime object and pass_or_fail is integer
        (0 for build failure; 1 for build success).
        """
        result = self.execute('SELECT Date,Result FROM Build_%s' % target)
        return list(result)
        
    def delete_build_record(self, target, datetime):
        """
        Delete a build record from a target's build table.

        target   - Target name string.
        datetime - Date/time string in format of 'YYYY-MM-DD HH:MM:SS'.
        """
        self.execute('DELETE FROM Build_%s WHERE Date="%"' % (target, datetime))
        return

    def add_unit_test_record(self, target, datetime, test, result, log):
        """
        Add an unit test record to the 'Unit_Test_[target]' table.

        This methods inserts a row to the 'Unit_Test_[target]' table.
        Each row contains the date, the test name, result, and
        a log of the test.

        target - Target name string.
        test   - Test name string.
        result - A 3-tuple that holds the number of tests passed,
                 failed and crashed.
        log    - A string that is the test log itself.
        """
        
        # We need to replace all " with \" so that MySQL will handle
        # them correctly.
        log = re.sub('"', '\\"', log)
        self.execute('INSERT INTO Unit_Test_%s VALUES ("%s", "%s", %d, %d, %d, "%s")' %
                     (target, datetime, test, result[0], result[1], result[2], log))
        return

    def get_unit_test_records(self, target, datetime=None):
        """
        Get all unit test records for a target on a given date/time.

        get_unit_test_records(target, datetime) queries all unit test
        records from Unit_test_[target] and returns an array of
        tuples (datetime, name, passed, failed, crashed).

        target   - Target name string.
        datetime - A date/time string of format 'YYYY-MM-DD HH:MM:SS'.
                   If it is not given, all records are queried.
        """
        if None == datetime:
            results = self.execute('SELECT Date,Name,Passed,Failed,Crashed FROM Unit_Test_%s' %
                                   target)
        else:
            results = self.execute('SELECT Date,Name,Passed,Failed,Crashed FROM Unit_Test_%s WHERE Date="%s"' %
                                   (target, datetime))
        return list(results)
