#!/usr/bin/env python
# coding=utf-8

# import MySQLdb
# import os

# conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in')
# cursor = conn.cursor()

# # 查找出当前目录下所有的文件
# for root,dirs,files in os.walk("./"):
#     # print root
#     # print dirs
#     # print files
#     file_name = files

# # "aggdraw-1.3.4-cp27-cp27m-win32.whl"
# for item in file_name:
#     # 我们需要插入的是所有的以 .whl 为后缀的文件
#     if item[-4:] == '.whl':
#         str = item.split('-')
#         # 以上述为例, 取出 aggdraw 
#         name = str[0]
#         # 取出 a,转化为对应的大写字母(数据库表名为大写)
#         table_name = str[0][0].upper()
#         version = item
#         # print table_name
#         # print name
#         # print version
#         with open(item,'rb') as f:
#             data = f.read()
#         # with open('aaaaa','a+') as f:
#             # f.write(data)
            
#         sql = "INSERT INTO %s values('%s','%s','%s')"%(table_name,name,version,MySQLdb.Binary(data))
#         try:
#             cursor.execute(sql)
#             conn.commit()
#         except Exception,e:
#             db.rollback()
#             print str(e)
            
# cursor.close()
# conn.close()

import MySQLdb
import os
import urllib

conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in',charset='utf8')
cursor = conn.cursor()

for root,dirs,files in os.walk("./"):
    file_name = files

# "aggdraw-1.3.4-cp27-cp27m-win32.whl"
for item in file_name:
    if item[-4:] == '.whl':
        str = item.split('-')
        name = str[0]
        print type(name)
        table_name = str[0][0].upper()
        print type(table_name)
        v = item
        print type(v)
        with open(item,'rb') as f:
            data = f.read()
        # sql = '''INSERT INTO %s values('%s','%s',%s)'''%(table_name,name,v,MySQLdb.Binary(data))
        sql = '''INSERT INTO %s values('%s','%s',%b)'''%(table_name,name,v,data)
        break
try:
    cursor.execute(sql)
    conn.commit()
except Exception,e:
    conn.rollback()
    print e

cursor.close()
conn.close()
