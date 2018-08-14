#!/usr/bin/env python
# coding=utf-8

import MySQLdb
import os
import urllib

conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in',charset='utf8')
cursor = conn.cursor()

# with open("./ad3-2.1-cp37-cp37m-win32.whl","rb") as f:
with open("./ad3-2.1-cp37-cp37m-win32.whl","rb") as f:
    data = f.read()
    # data = data.replace('\', '\\')
    data = data.replace("\\","\\\\")
    
sql = 'INSERT INTO bin values("%s")'%(MySQLdb.Binary(data))

try:
    cursor.execute(sql)
    conn.commit()
except Exception,e:
    conn.rollback()
    print e

cursor.close()
conn.close()
