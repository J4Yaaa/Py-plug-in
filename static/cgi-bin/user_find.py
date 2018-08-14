#!/usr/bin/env python
# coding=utf-8

# 如果用户找不到需要的插件,可以给留下姓名,邮箱,所要找的插件信息,我们查找到后会发送邮件给用户
# 主要保存用户名,用户邮箱,用户所要找的插件
# user_find(name varchar(64),email varchar(64),find varchar(128), submit_time datetime, state tinyint(1));

import MySQLdb
import os
import urllib
import signal

signal.signal(signal.SIGINT,signal.SIG_IGN)

print "Content-Type: text/html\n"
print "\n"

print '<html>'
print '<head><meta http-equiv="refresh" content="1,url=../index.html" charset="utf8"></head>'

# 连接数据库
conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in',charset='utf8')
# 获取游标
cursor = conn.cursor()

# 获取环境变量,这里获取的是 用户姓名,邮箱,所查找的插件
# username=J4Ya_&useremail=123%40qq.com&userfind=hahah
ev = os.environ
method = ev.get('REQUEST_METHOD').upper()
if method == 'GET':
    '''
    GET请求只有query_string
    取出第一个字符,表示要查找的表名(转为大写)
    table_name 表示当前所要查找的表名
    name 表示在这个表中查找name为这里name值的version
    '''
    query_string = ev.get('QUERY_STRING')
    query_string = urllib.unquote(query_string)
    msg = query_string.split('&')
    username = msg[0][9:]
    useremail = msg[1][10:]
    userfind = msg[2][9:]
elif method == 'POST':
    '''
    POST方法根据Content-Length来读取数据
    '''
    content_length = ev.get('CONTENT_LENGTH')
    body = os.read(0,int(content_length))
    body = urllib.unquote(body)
    msg = body.split('&')
    username = msg[0][9:]
    useremail = msg[1][10:]
    userfind = msg[2][9:]

sql = "INSERT INTO user_find values('%s','%s','%s',now(),0)"%(username,useremail,userfind)
try:
    cursor.execute(sql)
    conn.commit()
except Exception,e:
    db.rollback()
    print str(e)

cursor.close()
conn.close()

print '我们已经收到您的反馈,正在处理了呢,稍等一下下嘛~~~'

print '</html>'
