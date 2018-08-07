#!/usr/bin/env python
# coding=utf-8

str = 'id=1&name=zhangsan'
s = str.split('&')

id = int(s[0][3:])
name = s[1][5:]

print id,name

value = [id,name]
sql = 'INSERT INTO student_info values(%d,"%s")',value)
print value
print sql
