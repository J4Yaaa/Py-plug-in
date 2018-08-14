#!/usr/bin/env python
# coding=utf-8

import MySQLdb
import os

conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in')
cursor = conn.cursor()

with open('./aggdraw-1.3.4-cp27-cp27m-win32.whl','rb') as f:
    data = f.read()

table_name = 'A'
name = 'aggdraw'
v = 'aggdraw-1.3.4-cp27-cp27m-win32.whl'

sql = "INSERT INTO %s values('%s','%s','%s')"%(table_name,name,v,(MySQLdb.Binary(data)))
# print sql
# '%s','%s',%b)"%(table_name,name,v,data)
# try:
cursor.execute(sql)
conn.commit()
# except Exception,e:
#   db.rollback()
#   print str(e)
            
cursor.close()
conn.close()
