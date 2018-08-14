#!/usr/bin/env python
# coding=utf-8

import MySQLdb
import os
import signal

signal.signal(signal.SIGINT,signal.SIG_IGN)

ev = os.environ
method = ev.get('REQUEST_METHOD').upper()
if method == 'GET':
    query_string = ev.get('QUERY_STRING')
    table_name = query_string[5].upper()
    version = query_string[5:]
else:
    content_length = ev.get('CONTENT_LENGTH')
    body = os.read(0,int(content_length))
    table_name = body[5].upper()
    version = body[5:]

# 表示当前为可下载文件
# print "Content-Type: text/html"
print "Content-Type: application/octet-stream"
# 产生固定的文件名
print "Content-Disposition: attachment;filename=%s" % version
# 空行
print ""

path = "static/cgi-bin/plugin/"+version

with open(path,"r") as f:
    data = f.read()
    print data
