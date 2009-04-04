#!/usr/bin/python
import sys, cgi, nightly_mysql, time, re

def process_one_day(db_name, this_day, targets):
    if len(this_day) == 0: return
    print '    <TD ROWSPAN=%d>%d</TD>' % (len(this_day), this_day[0][0].day)
    for row in this_day:
        print '    <TD>%s</TD>' % row[0].strftime('%H:%M:%S')
        ii = 0
        for tgt in row[1:]:
            if (tgt[0] == 0) and (tgt[1] == 0) and (tgt[2] == 0):
                print('      <TD></TD><TD></TD><TD></TD>')
                ii += 1
                continue
            if tgt[0] > 0:
                color = 'green'
            else:
                color = 'gray'
            print('      <TD><A HREF="nightly_test_log.py?db=%s&target=%s&date=%s"><FONT COLOR="%s">%d</FONT></A></TD>' %
                  (db_name, targets[ii], row[0].strftime('%Y%m%d%H%M%S'), color, tgt[0]))
            if tgt[1] > 0:
                color = 'red'
            else:
                color = 'gray'
            print('      <TD><A HREF="nightly_test_log.py?db=%s&target=%s&date=%s"><FONT COLOR="%s">%d</FONT></A></TD>' %
                  (db_name, targets[ii], row[0].strftime('%Y%m%d%H%M%S'), color, tgt[1]))
            if tgt[2] > 0:
                color = 'red'
            else:
                color = 'gray'
            print('      <TD><A HREF="nightly_test_log.py?db=%s&target=%s&date=%s"><FONT COLOR="%s">%d</FONT></A></TD>' %
                  (db_name, targets[ii], row[0].strftime('%Y%m%d%H%M%S'), color, tgt[2]))
            ii += 1
        sys.stdout.write('</TR>\n')
    return

def process_one_month(db_name, this_month, targets):
    if len(this_month) == 0: return
    print '  <TD ROWSPAN=%d>%s</TD>' % (len(this_month), this_month[0][0].strftime('%b'))
    cur_day = this_month[0][0].day
    this_day = []
    for row in this_month:
        if row[0].day != cur_day:
            # Parse the list day by day
            process_one_day(db_name, this_day, targets)
            cur_day = row[0].day
            this_day = [row,]
        else:
            this_day.append(row)
    # Handle the last day list
    if len(this_day) > 0:
        process_one_day(db_name, this_day, targets)
    return

def selected(selected_year, selected_month, year, month):
    if (((selected_year > 0) and (year != selected_year)) or
        ((selected_month > 0) and (month != selected_month))):
        return False
    return True

def process_one_year(db_name, this_year, targets, selected_year, selected_month):
    if len(this_year) == 0: return
    cur_year = this_year[0][0].year

    # Count the number of row to span. Note that we need to count months
    # that are not selected only once even if they have many
    colspan = 0
    increment = 1
    cur_month = this_year[0][0].month
    for row in this_year:
        if row[0].month != cur_month:
            increment = 1
            cur_month = row[0].month
        colspan = colspan + increment
        if not selected(selected_year, selected_month, row[0].year, row[0].month):
            increment = 0
    print '<TR><TD ROWSPAN=%d>%d</TD>' % (colspan, cur_year)
    cur_month = this_year[0][0].month
    this_month = []
    for row in this_year:
        if row[0].month != cur_month:
            # Parse the list month by month
            if len(this_month) == 0: continue
            if not selected(selected_year, selected_month, cur_year, cur_month):
                # This is not the selected (year, month). Just display
                # the collapsed link.
                print('    <TD>%s</TD><TD COLSPAN=%d><CENTER><A HREF="nightly_test_log.py?db=%s&year=%d&month=%d">[Expand]</A></CENTER></TD></TR>' %
                      (this_month[0][0].strftime('%b'), 2+3*len(targets), db_name, cur_year, cur_month))
            else:
                process_one_month(db_name, this_month, targets)
            cur_month = row[0].month
            this_month = [row,]
        else:
            this_month.append(row)

    # Handle the last month list
    if len(this_month) > 0:
        if not selected(selected_year, selected_month, cur_year, cur_month):
            # This is not the selected (year, month). Just display
            # the collapsed link.
            print('    <TD>%s</TD><TD COLSPAN=%d><CENTER><A HREF="nightly_test_log.py?db=%s&year=%d&month=%d">[Expand]</A></CENTER></TD></TR>' %
                  (this_month[0][0].strftime('%b'), 2+3*len(targets), db_name, cur_year, cur_month))
        else:
            process_one_month(db_name, this_month, targets)
    return

def show_summary(db_name, year, month):
    # Query the Target table to get all unit test targets.
    db = nightly_mysql.NightlyMySQL(user='root', db=db_name)
    targets = db.get_targets()
    targets_len = len(targets)
    if targets_len == 0:
        # There is no target.
        print 'No target is registered in database.'
        sys.exit(0)
    
    # For each target, get all unit test records
    tests = []
    for t in targets:
        results = db.get_unit_test_records(t)
        for n in range(len(results)):
            results[n] = results[n] + (t,)
        tests = tests + results

    # Sort them by date
    tests.sort(lambda x,y: cmp(x[0], y[0]))

    if len(tests) == 0:
        print 'No unit test records for any targets.'
        sys.exit(0)

    # Make a lookup table that maps target to an index
    lookup = {}
    index = 1
    for t in targets:
        lookup[t] = index
        index = index + 1
    
    # Make a final result table
    cur_date = tests[0][0]
    table = [[cur_date,]]
    for n in range(targets_len): table[-1].append([0, 0, 0])

    for b in tests:
        if b[0] != cur_date:
            # A different date. Create a new row.
            cur_date = b[0]
            table.append([cur_date,])
            for n in range(targets_len): table[-1].append([0, 0, 0])
        table[-1][lookup[b[-1]]][0] += b[2]
        table[-1][lookup[b[-1]]][1] += b[3]
        table[-1][lookup[b[-1]]][2] += b[4]

    # Generate HTML
    print 'Content-type: text/html\n'
    print '<HTML>'
    print '<H2>UNIT TEST SUMMARY - %s</H2>' % db_name
    print '<TABLE BORDER=1 CELLPADDING=2 RULES=ALL">'

    print '<TR><TH ROWSPAN=2>Year</TH><TH ROWSPAN=2>Month</TH><TH ROWSPAN=2>Date</TH><TH ROWSPAN=2>Time</TH>'
    for t in targets:
        print '<TH COLSPAN=3>%s</TH>' % t
    print '</TR>\n'
    print '</TR>'
    for t in targets:
        print '<TH>Passed</TH><TH>Failed</TH><TH>Crashed</TH>'
    print '</TR>'

    cur_year = table[0][0].year
    this_year = []
    for row in table:
        if row[0].year != cur_year:
            # Parse the list month by month
            process_one_year(db_name, this_year, targets, year, month)
            cur_year = row[0].year
            this_year = [row,]
        else:
            this_year.append(row)

    # Handle the last year list
    if len(this_year) > 0:
        process_one_year(db_name, this_year, targets, year, month)

    print '</TABLE>'
    return

def show_log(db_name, target, date, test):
    datestr = '%s-%s-%s %s:%s:%s' % (date[0:4], date[4:6], date[6:8],
                                     date[8:10], date[10:12], date[12:14])
    db = nightly_mysql.NightlyMySQL(user='root', db=db_name)

    print 'Content-type: text/html\n'
    print '<HTML>'
    print '<H2>UNIT TEST LOG</H2>'
    print '<P><TABLE BORDER=1 CELLPADDING=2 RULES=ALL>'
    print '<TR><TD><B>Database</B></TD><TD>%s</TD></TR>' % db_name
    print '<TR><TD><B>Target</B></TD><TD>%s</TD></TR>' % target
    print '<TR><TD><B>Date</B></TD><TD>%s</TD></TR>' % datestr
    print '</TABLE></P>'

    if None == test:
        result = db.get_unit_test_records(target, datestr)

        print '<TABLE BORDER=1 RULES=ALL CELLPADDING=2>'
        print '<TR><TH>Test Name</TH><TH>Passed</TH><TH>Failed</TH><TH>Crashed</TH></TR>'
        for row in result:
            print('<TR><TD><A HREF="nightly_test_log.py?db=%s&target=%s&date=%s&test=%s">%s</A></TD>' %
                  (db_name, target, date, row[1], row[1]))
            if row[2] > 0:
                color = 'green'
            else:
                color = 'gray'
            print '<TD><FONT COLOR="%s">%d</FONT></TD>' % (color, row[2])
            if row[3] > 0:
                color = 'red'
            else:
                color = 'gray'
            print '<TD><FONT COLOR="%s">%d</FONT></TD>' % (color, row[3])
            if row[4] > 0:
                color = 'red'
            else:
                color = 'gray'
            print '<TD><FONT COLOR="%s">%d</FONT></TD></TR>' % (color, row[4])
        print '</TABLE>'
    else:
        log = db.execute('SELECT Log FROM Unit_Test_%s WHERE Date="%s" AND Name="%s"' %
                         (target, datestr, test))
        print '<P><PRE>'
        if len(log) > 0:
            print log[0][0]
        print '</PRE></P>'
    return

def main():
    form = cgi.FieldStorage()
    if not form.has_key('db'):
        print 'Content-type: text/plain\n'
        print '!!! ERROR: NO DATABASE SPECIFIED !!!\n'
        return
    db_name = form['db'].value

    unit_test = None
    if form.has_key('test'):
        unit_test = form['test'].value
        
    if form.has_key('target') and form.has_key('date'):
        show_log(db_name, form['target'].value, form['date'].value, unit_test)
    else:
        year = int(time.strftime('%Y'))
        month = int(time.strftime('%m'))
        if form.has_key('year'):
            year = int(form['year'].value)
        if form.has_key('month'):
            month = int(form['month'].value)
        show_summary(db_name, year, month)

    print '<HR>'
    print '<I>Generated by Nightly on %s</I>' % time.strftime('%Y-%b-%d %H:%M:%S')
    print '</HTML>'

main()
