#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql/mysql.h>

void select_data()
{
    MYSQL* mysql_fd = mysql_init(NULL);
    if(!mysql_real_connect(mysql_fd,"localhost","root","qweasd","py_plug_in",0,NULL,0))
    {
        perror("connect faild\n");
        return;
    }
    mysql_query(mysql_fd, "select * A");
    MYSQL_RES* res = mysql_store_result(mysql_fd);
    int row = mysql_num_rows(res);
    int col = mysql_num_fields(res);
    MYSQL_FIELD* field = mysql_fetch_fields(res);
    int i = 0;
    for(; i < col; ++i)
    {
        printf("%s\t",field[i].name);
    }
    printf("\n");
    printf("<table border=\"1\">");
    for(i = 0; i < row; ++i)
    {
        MYSQL_ROW rowData = mysql_fetch_row(res);
        int j = 0;
        printf("<tr>");
        for(; j < col-1; ++j)
        {
            printf("<td>%s</td>",rowData[j]);
        }
        printf("</tr>");
    }
    printf("</table>");
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
                read(0,data+i,1);
            }
            data[i] = '\0';
        }
    }
    select_data();
    return 0;
}
