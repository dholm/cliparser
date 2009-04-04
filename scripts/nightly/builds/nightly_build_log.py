#!/usr/bin/python
import sys, cgi, nightly_mysql, time, re

def process_one_day(db_name, this_day, targets):
    if len(this_day) == 0: return
    print '    <TD ROWSPAN=%d>%d</TD>' % (len(this_day), this_day[0][0].day)
    for row in this_day:
        print '    <TD>%s</TD>' % row[0].strftime('%H:%M:%S')
        ii = 0
        for tgt in row[1:]:
            if tgt > 0:
                print('      <TD><A HREF="nightly_build_log.py?db=%s&target=%s&date=%s"><FONT COLOR="green">OK</FONT></A></TD>' %
                      (db_name, targets[ii], row[0].strftime('%Y%m%d%H%M%S')))
            elif tgt == 0:
                print('      <TD><A HREF="nightly_build_log.py?db=%s&target=%s&date=%s"><FONT COLOR="red">FAILED</FONT></A></TD>' %
                      (db_name, targets[ii], row[0].strftime('%Y%m%d%H%M%S')))
            else:
                print '      <TD></TD>'
            ii = ii + 1
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
                print('    <TD>%s</TD><TD COLSPAN=%d><CENTER><A HREF="nightly_build_log.py?db=%s&year=%d&month=%d">[Expand]</A></CENTER></TD></TR>' %
                      (this_month[0][0].strftime('%b'), 2+len(targets), db_name, cur_year, cur_month))
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
            print('    <TD>%s</TD><TD COLSPAN=%d><CENTER><A HREF="nightly_build_log.py?db=%s&year=%d&month=%d">[Expand]</A></CENTER></TD></TR>' %
                  (this_month[0][0].strftime('%b'), 2+len(targets), db_name, cur_year, cur_month))
        else:
            process_one_month(db_name, this_month, targets)
    return

def show_summary(db_name, year, month):
    # Query the Target table to get all build targets.
    db = nightly_mysql.NightlyMySQL(user='root', db=db_name)
    targets = db.get_targets()
    targets_len = len(targets)
    if targets_len == 0:
        # There is no target.
        print 'No target is registered in database.'
        sys.exit(0)
    
    # For each target, get all build records
    builds = []
    for t in targets:
        results = db.get_build_records(t)
        for n in range(len(results)):
            results[n] = results[n] + (t,)
        builds = builds + results

    # Sort them by date
    builds.sort(lambda x,y: cmp(x[0], y[0]))

    if len(builds) == 0:
        print 'No build records for any targets.'
        sys.exit(0)

    # Make a lookup table that maps target to an index
    lookup = {}
    index = 1
    for t in targets:
        lookup[t] = index
        index = index + 1
    
    # Make a final result table
    cur_date = builds[0][0]
    table = [[cur_date,] + [-1,] * targets_len]
    
    for b in builds:
        if b[0] != cur_date:
            # A different date. Create a new row.
            cur_date = b[0]
            table.append([cur_date,] + [-1,] * targets_len)
        table[-1][lookup[b[2]]] = b[1]

    # Generate HTML
    print 'Content-type: text/html\n'
    print '<HTML>'
    print '<H2>BUILD LOG SUMMARY - %s</H2>' % db_name
    print '<TABLE BORDER=1 CELLPADDING=2 RULES=ALL">'

    print '<TR><TH>Year</TH><TH>Month</TH><TH>Date</TH><TH>Time</TH>'
    for t in targets:
        print '<TH>%s</TH>' % t
    print '</TR>\n'

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

def show_log(db_name, target, date):
    print 'Content-type: text/html\n'
    datestr = '%s-%s-%s %s:%s:%s' % (date[0:4], date[4:6], date[6:8],
                                     date[8:10], date[10:12], date[12:14])
    db = nightly_mysql.NightlyMySQL(user='root', db=db_name)
    log = db.execute('SELECT Log FROM Build_%s WHERE Date="%s"' % (target, date))
    print '<HTML>'
    print '<P><TABLE BORDER=1 CELLPADDING=2 RULES=ALL>'
    print '<TR><TD><B>Database</B></TD><TD>%s</TD></TR>' % db_name
    print '<TR><TD><B>Target</B></TD><TD>%s</TD></TR>' % target
    print '<TR><TD><B>Date</B></TD><TD>%s</TD></TR>' % datestr
    print '</TABLE></P>'
    print '<PRE>'
    if len(log) > 0:
        build_log = log[0][0].replace('<', '&lt;')
        build_log = build_log.replace('>', '&gt;')
        print build_log
    print '</PRE>'
    return

def main():
    form = cgi.FieldStorage()
    if not form.has_key('db'):
        print 'Content-type: text/plain\n'
        print '!!! ERROR: NO DATABASE SPECIFIED !!!\n'
        return
    db_name = form['db'].value
    
    if form.has_key('target') and form.has_key('date'):
        show_log(db_name, form['target'].value, form['date'].value)
    else:
        year = int(time.strftime('%Y'))
        month = int(time.strftime('%m'))
        #year = -1
        #month = -1
        if form.has_key('year'):
            year = int(form['year'].value)
        if form.has_key('month'):
            month = int(form['month'].value)
        show_summary(db_name, year, month)

    print '<HR>'
    print '<I>Generated by Nightly on %s</I>' % time.strftime('%Y-%b-%d %H:%M:%S')
    print '</HTML>'

main()
