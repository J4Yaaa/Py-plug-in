#!/usr/bin/env python
# coding=utf-8

# str = 'id=1&name=zhangsan&hahah=enenen'
# s = str.split('&')
# print s

# id = int(s[0][3:])
# name = s[1][5:]

# print id,name

# value = [id,name]
# sql = 'INSERT INTO student_info values(%d,"%s")',value)
# print value
# print sql

# with open("./header.txt") as f:
#     for line in f.read():  
#         print line

# 读取当前文件下的所有子文件
# import os
# for root,dirs,files in os.walk("/home/jin/py-plug-in/HTTP/static/cgi-bin"):
#     # print root
#     # print dirs
#     # print files
#     file_name = files

# for item in file_name:
#     if item[-4:] == '.whl':
#         print item

# 尝试使用python忽略信号
import signal
signal.signal(signal.SIGINT,signal.SIG_IGN)
while True:
    pass
