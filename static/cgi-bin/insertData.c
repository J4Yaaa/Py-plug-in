#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>

void splitData(char* data,int* id, char* name)
{
    strtok(data,"=&");
    *id = atoi(strtok(NULL,"=&"));
    strtok(NULL,"=&");
    strcpy(name,strtok(NULL,"=&"));
}

void insertData(char* data)
{
    printf("data: %s<br>",data);
    int id;
    char name[32] = {0};
    splitData(data,&id,name);
    MYSQL* mysql_fd = mysql_init(NULL);
    if(mysql_real_connect(mysql_fd,"localhost","root","qweasd","test_jin",0,NULL,0) == 0)
    {
        perror("mysql_real_connect");
        return;
    }
    char insert[1024] = {0};
    sprintf(insert,"insert into student_info values(%d,\"%s\")",id,name);
    if(mysql_query(mysql_fd,insert) < 0)
    {
        perror("mysql_query");
        return;
    }
    mysql_close(mysql_fd);
}

int main()
{
    char data[1024] = {0};
    if(getenv("REQUEST_METHOD"))
    {
        if(strcasecmp("GET",getenv("REQUEST_METHOD")) == 0)
        {
            strcpy(data,getenv("QUERY_STRING"));
        }
        else
        {
            int content_length = atoi(getenv("CONTENT_LENGTH"));
            int i = 0;
            for(; i < content_length; ++i)
            {
                read(0,&data[i],1);
            }
            data[i] = '\0';
        }
    }
    insertData(data);
    return 0;
}
