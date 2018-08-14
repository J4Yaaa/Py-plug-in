#!/usr/bin/env python
# coding=utf-8

import MySQLdb
import os
import signal

signal.signal(signal.SIGINT,signal.SIG_IGN)

print "Content-Type: text/html\n"
print "\n"

# 连接数据库
conn = MySQLdb.connect(host='localhost',port=0,user='root',passwd='qweasd',db='py_plug_in')
# 获取游标
cursor = conn.cursor()
# 构造查询语句

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
    table_name = query_string[5].upper()
    name = query_string[5:]
else:
    '''
    POST方法根据Content-Length来读取数据
    '''
    content_length = ev.get('CONTENT_LENGTH')
    body = os.read(0,int(content_length))
    table_name = body[5].upper()
    name = body[5:]

# 将页面的样式写入管道
print '''
<!doctype html>
<head>
<meta charset="utf-8">
<title>Py-plug-in</title>
<meta name="description" content="">
<meta name="author" content="">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<!-- combined & minified reset, bootstrap, bootstrap-responsive & bootstrap docs stylesheets -->
<link rel="stylesheet" href="css/reset-bootstrap-responsive-docs.css">
<link rel="stylesheet" href="css/fontawesome.css">
<link rel="stylesheet" href="css/prettyphoto.css">
<link rel="stylesheet" href="style.css">
<link rel="stylesheet" href="css/skin/default.css"> 
</head>

<body>
<style type="text/css">
input[type="submit"]{
	width:500px;
	height:30px;
	border:0px;
	background-color:white;
	backgrount-opacity:0.2;
	color:#FF6347;
	font-size:18px;
        float:left
}
.outer-nav {
    background-color: #F1584A;
    border-bottom: 2px solid #F53A26;
}
a,.colored,
.underline a:hover,
.single-item a:hover, 
.page-stats li span.stat-title {
    color: #F53A26;
}
html, body {
    width:100%;
    padding:0;
    margin:0;
}
.exclamation h1, 
.exclamation h2, 
.exclamation h3, 
.exclamation h4, 
.exclamation h5, 
.exclamation h6,
.filter.btn-group .btn {
    color:#555;
    text-shadow: 1px 1px 0 #F2F2F2, 1px 2px 0 #B1B1B2;
}
.exclamation h1 {
    font-size: 40px;
    line-height: 45px;
}
ul {
    margin:0;
    padding:0;
    list-style-image:none;
    list-style:none outside none;
}
.span4{
    width:100%
}
h1,h2,h3,h4,h5,h6{margin:0;font-family:inherit;font-weight:bold;color:inherit;text-rendering:optimizelegibility}
.page-header{padding-bottom:17px;margin:18px 0;border-bottom:1px solid #eee}
</style>
'''

# with open("./header.txt","r") as f:
#     for line in f.read():  
#         print line

print '</header>' 
print '<div class="exclamation title margin-bottom">'
print '<div class="page-header">'
print '<h1>&nbsp &nbsp %s </h1>'%name
print '</div></div>'

print '<div class="span4">'
print '<p class="lead">'
print '<form action="./download.py" method="GET">'

sql = "SELECT version FROM %s where name='%s'"%(table_name,name)
cursor.execute(sql)
results = cursor.fetchall()
for r in results:
    print '''<input type="submit" name="name" value="'''+r[0]+'''">'''

# 关闭游标连接
cursor.close()
# 关闭数据库连接
conn.close()

print '</form></p></div></div></body></html>'
