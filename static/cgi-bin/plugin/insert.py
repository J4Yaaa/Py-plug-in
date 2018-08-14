#!/usr/bin/env python
# coding=utf-8

# import MySQLdb
# import os

import MySQLdb
import os
import urllib

conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in',charset='utf8')
cursor = conn.cursor()

for root,dirs,files in os.walk("./"):
    file_name = files

for item in file_name:
    if item[-4:] == '.whl':
        str = item.split('-')
        name = str[0]
        table_name = str[0][0].upper()
        sql = '''INSERT INTO %s values('%s','%s')'''%(table_name,name,item)
        try:
            cursor.execute(sql)
            conn.commit()
        except Exception,e:
            conn.rollback()
            print e

cursor.close()
conn.close()
