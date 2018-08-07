#!/usr/bin/env python
# coding=utf-8

import MySQLdb
import os

print '<html>'
conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='test_jin')
# 获取游标
cursor = conn.cursor()

method = str(os.getenv('REQUEST_METHOD'))

if method == 'GET':
    data = os.getenv('QUERY_STRING')
else:
    length = int(os.getenv('CONTENT_LENGTH'))
    data = os.read(0,length)

print data
# 形如: id=1&name=zhangsan
s = data.split('&')
stu_id = int(s[0][3:])
stu_name = s[1][5:]

sql = "INSERT INTO student_info values(%d,'%s')"%(stu_id,stu_name)
print sql
cursor.execute(sql)
conn.commit()

cursor.close()
conn.close()
print '</html>'
