#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <signal.h>

#define SIZE (1024*10)

// 这个结构体专门存储HTTP请求
typedef struct HttpRequest
{
    char first_line[SIZE];
    char* method;
    char* url;
    char* url_path;
    char* query_string;
    int content_length;
}HttpRequest;

int ReadLine(int sock,char buf[],ssize_t max_size)
{
    // 按行从sock中读取数据
    // 浏览器中的换行符: \n \r \n\r
    // 1. 循环从sock中读取字符,一次读一个
    char c = '\0';
    ssize_t index = 0;
    while(index < max_size)
    {
        ssize_t read_size = recv(sock,&c,1,0);
        if(read_size <= 0)
        {// 如果等于0的话,表示还没有读完一行,读就结束了,所以报文有误
            return -1;
        }
        // 2. 如果读到的是 \r
        if(c == '\r')
        {
            //   a) 尝试从缓冲区读取下一个字符,如果下一个是\n,就把这种情况处理成\n
            recv(sock,&c,1,MSG_PEEK); // MSG_PEEK表示从缓冲区只读一个数据,但是并不把这个数据从缓冲区中删除掉(预读)
            if(c == '\n')
            {// \r\n 情况
                recv(sock,&c,1,0); // 把 \n 从缓冲区中删除掉
            }
            else
            {//   b) 如果下一个字符不是\n,那就把\r修改成\n,为了将\r和\n的情况统一在一起
                c = '\n'; // 为了统一我们的判定标准
            }
        }
        // 3. 如果是其他字符,就将字符放入buf中
        buf[index++] = c;
        // 4. 如果当前字符是 \n,就退出循环,函数结束
        if(c == '\n')
        {// 可能是 \r->\n 或者是读到了 \n
            break;
        }
    }
    buf[index] = '\0';
    return index;
}

// strtok 致命问题!!!
// 当前是多线程程序,而strtok是一个不可重入函数,更是线程不安全的(内部有一个静态变量保存上一次切分的位置)
// 而 strtok_r 是线程安全的字符串切分函数
ssize_t Split(char* first_line,const char* split_char,char* output[])
{
    char* tmp = NULL;
    int output_index = 0;
    char* p = strtok_r(first_line,split_char,&tmp);
    while(p != NULL)
    {
        output[output_index++] = p;
        p = strtok_r(NULL,split_char,&tmp);
    }
    output[output_index] = NULL;
    return output_index;
}

int ParseFirstLine(char* first_line,char** method_ptr,char** url_ptr)
{
    char* tokens[100] = {NULL};
    // Split 切分完毕后就会破坏掉原有的字符串,将原有的分隔符替换为 '\0'
    ssize_t n = Split(first_line," ",tokens);
    if(n != 3)
    {
        printf("first_line split error! n=%ld\n",n);
        return -1;
    }
    // 验证tokens[2]是否包含 HTTP/ 这样的关键字
    *method_ptr = tokens[0];
    *url_ptr = tokens[1];
    return 0;
}

int ParseQueryString(char* url,char** url_path,char** query_string)
{
    // 此处url没有考虑带域名的情况 --- 偷懒了又
    char* p = url;
    *url_path = url;
    while(*p != '\0')
    {
        if(*p == '?')
        {
            *p = '\0';
            *query_string = p+1;
            return 0;
        }
        ++p;
    }
    *query_string = NULL;
    return 0;
}

int HandlerHeader(int sock,int* content_length)
{
    char buf[SIZE] = {0};
    while(1)
    {
        if(ReadLine(sock,buf,sizeof(buf)-1) < 0)
        {
            printf("read line Header faild\n");
            return -1;
        }
        if(strcmp(buf,"\n") == 0)
        {
            return 0;
        }
        const char* content_length_str = "Content-Length: ";
        if(strncmp(buf,content_length_str,strlen(content_length_str)) == 0)
        {
            *content_length = atoi(buf+strlen(content_length_str));
        }
        // 不可以 return
        // 这个函数有两个功能
        // 1. 找到 Content-Length 的值
        // 2. 将接受缓冲区的数据都读取出来,避免粘包问题
    } 
    return 0;
}

int IsDir(char* file_path)
{
    struct stat s;
    stat(file_path,&s);
    if(S_ISDIR(s.st_mode))
    {
        return 1;
    }
    return 0;
}

void HanderFilePath(const char* url_path,char* file_path)
{
    // ./static 这是随便起的名字,此处是指http服务器的根目录
    // 当前服务器需要暴露给客户端的文件全部都放在这个目录下
    sprintf(file_path,"./static%s",url_path);
    // url_path 中的几种特殊情况
    //  a) 如果url中没有写路径,默认是 /,即HTTP服务器的根目录
    //  b) 如果url后写路径了,但是对应的路径是一个目录
    //     那就尝试访问这个目录下的index.html文件,作为默认返回的页面
    if(url_path[strlen(url_path) - 1] == '/')
    {
        strcat(file_path,"index.html");
    }
    // 如果 url_path 最后一个字符不是 /,但是访问的仍然是一个目录,那我们就应该先识别出这个目录
    if(IsDir(file_path))
    {
        strcat(file_path,"/index.html");
    }
    return;
}

size_t GetFileSize(const char* file_path)
{
    size_t filesize = -1;      
    struct stat s;  
    if(stat(file_path, &s) < 0){  
        return filesize;  
    }else{  
        filesize = s.st_size;  
    }  
    return filesize;  
}

int WritestaticFile(int sock,const char* file_path)
{
    // 如果文件打开失败,就证明文件不存在
    int fd = open(file_path,O_RDONLY);
    if(fd < 0)
    {
        perror("open");
        return 404;
    }
    size_t file_size = GetFileSize(file_path);
    const char* first_line = "HTTP/1.1 200 OK\n";
    const char* blank_line = "\n";
    char header[SIZE] = {0};
    sprintf(header,"Content-Length: %lu\n",file_size);

    send(sock,first_line,strlen(first_line),0);
    send(sock,header,strlen(header),0);
    send(sock,blank_line,strlen(blank_line),0);

    // sendfile 可以再一个内核中将数据从一个文件写到另一个文件中
    // 效率比较高,不经过用户态
    sendfile(sock,fd,NULL,file_size);
    close(fd);
    return 200;
}

int HandlerstaticFile(int sock,const HttpRequest* req)
{
    // 1.根据解析出的 url_path,获取到对应的真实文件路径
    char file_path[SIZE] = {0};
    HanderFilePath(req->url_path,file_path);
    // 2.打开文件,把文件中的内容读取出来,并写入socket
    return WritestaticFile(sock,file_path);
}

void HandlerFather(int new_sock,int father_read,int father_write,const HttpRequest* req)
{
    // a) 如果是 POST 就将 body 写入管道
    if(strcasecmp(req->method,"POST") == 0)
    {
        char c = '\0';
        int i = 0;
        for(; i < req->content_length; ++i)
        {
            read(new_sock,&c,1);
            write(father_write,&c,1);
        }
    }
    // b) 构造首行,Header,空行
    const char* first_line = "HTTP/1.1 200 OK\n";
    send(new_sock,first_line,strlen(first_line),0);
    const char* header = "Content-Type: text/html\n";
    send(new_sock, header, strlen(header), 0);
    const char* blank_line = "\n";
    send(new_sock,blank_line,strlen(blank_line),0);
    // c) 从管道中读取子进程返回的 Html 页面,把数据写到 socket 中
    char c = '\0';
    printf("begin father read\n");
    while(read(father_read,&c,1)>0)
    {
        send(new_sock,&c,1,0);
        printf("%c ",c);
    }
    printf("\nend father read\n");
    close(father_read);
    close(father_write);
    // d) 进程等待
    //    这里可以使用waitpid,也可以直接忽略 SIGPIPE 信号
}

void HandlerChild(int new_sock,int child_read,int child_write,const HttpRequest* req)
{
    (void)new_sock;

    // a) 设置环境变量 REQUEST_METHOD,QUERY_STRING,CONTENT_LENGTH
    //    (注意:这里的环境变量命名必须严格按照CGI标准来命名)
    /* printf("url_path: %s\n",req->url_path); */
    char request_method[SIZE] = {0};
    sprintf(request_method,"REQUEST_METHOD=%s",req->method);
    putenv(request_method);

    if(strcasecmp(req->method,"GET") == 0)
    {
        char query_string[SIZE] = {0};
        sprintf(query_string,"QUERY_STRING=%s",req->query_string);
        putenv(query_string);
    }
    else if(strcasecmp(req->method,"POST") == 0)
    {
        char content_length[SIZE] = {0};
        sprintf(content_length,"CONTENT_LENGTH=%d",req->content_length);
        putenv(content_length);
    }

    // b) 将标准输入输出重定向到管道中
    dup2(child_read,0);
    dup2(child_write,1);

    // c) 子进程进行程序替换
    char URLPATH[SIZE] = {0};
    sprintf(URLPATH,"static%s",req->url_path);
    execl(URLPATH,URLPATH,NULL);
    exit(1);
}

int HandlerCGI(int sock,const HttpRequest* req)
{
    int err_code = 200;
    // 1.创建一对匿名管道
    //   这里的output和input都是站在子进程的角度来说的
    int output[2]; // 子进程写 , 父进程读
    int input[2]; // 子进程读 , 父进程写
    if(pipe(output)<0)
    {
        perror("pipe fd1");
        return 404;
    }
    if(pipe(input)<0)
    {
        perror("pipe fd2");
        return 404;
    }

    // 2.fork创建子进程
    pid_t pid = fork();
    if(pid == 0)
    {// child
     // 4.子进程逻辑
        close(output[0]);
        close(input[1]);
        HandlerChild(sock,input[0],output[1],req);
    }
    else if(pid > 0)
    {// father
     // 3.父进程逻辑
        close(output[1]);
        close(input[0]);
        HandlerFather(sock,output[0],input[1],req);
    }
    else
    {
        err_code = 404;
    }
    return err_code;
}

void Handler404(int sock)
{
    // 构建一个HTTP的404页面返回
    const char* err_page = "./static/err.html";
    int fd = open(err_page,O_RDONLY);
    size_t file_size = GetFileSize(err_page);
    const char* first_line = "HTTP/1.1 200 OK\n";
    const char* blank_line = "\n";
    char header[SIZE] = {0};
    sprintf(header,"Content-Length: %lu\n",file_size);

    send(sock,first_line,strlen(first_line),0);
    send(sock,header,strlen(header),0);
    send(sock,blank_line,strlen(blank_line),0);

    // sendfile 可以再一个内核中将数据从一个文件写到另一个文件中
    // 效率比较高,不经过用户态
    sendfile(sock,fd,NULL,file_size);
    close(fd);
}

void HandlerRequet(int new_sock)
{
    // 1.读取请求并解析
    //   a) 从 socket 中读取HTTP请求的首行
    int err_code = 200;
    HttpRequest req;
    memset(&req,0x00,sizeof(req)); // 不初始化的时候值可能是栈上残存的数据 - 未定义行为
    if(ReadLine(new_sock,req.first_line,sizeof(req.first_line)-1) < 0)
    {
        printf("ReadLine first_line faild!\n");
        err_code = 404;
        goto END;
    }
    printf("first_line: %s\n",req.first_line);
    //   b) 解析首行,获取到方法,url,版本号(不用)
    if(ParseFirstLine(req.first_line,&req.method,&req.url) < 0)
    {
        printf("ParseFirstLine faild! first_line: %s\n",req.first_line);
        err_code = 404;
        goto END;
    }
    //   c) 对 url 进行解析,解析出其中的url_path,query_string
    if(ParseQueryString(req.url,&req.url_path,&req.query_string) < 0)
    {
        printf("ParseQueryString faild! url: %s\n",req.url);
        err_code = 404;
        goto END;
    }
    //   d) 读取并解析header部分(这里只保留 content_length,偷懒...)
    if(HandlerHeader(new_sock,&req.content_length) < 0)
    {
        printf("HandlerHeader faile!\n");
        err_code = 404;
        goto END;
    }

    // 2.根据请求的详细情况决定是执行静态页面逻辑还是动态页面逻辑
    //   a) 如果是 GET请求,并且没有query_string - 静态页面
    //   b) 如果是 GET请求,并且有query_string - 动态请求,根据query_string参数内容来动态计算生成页面
    //   c) POST请求一定是动态页面
    if(strcasecmp(req.method,"GET") == 0 && req.query_string == NULL)
    {// strcasecmp 是忽略大小写的比较
     // 生成静态页面
        err_code = HandlerstaticFile(new_sock,&req);
    }
    else if(strcasecmp(req.method,"GET") == 0 && req.query_string != NULL)
    {
        err_code = HandlerCGI(new_sock,&req);
    }
    else if(strcasecmp(req.method,"POST") == 0)
    {
        err_code = HandlerCGI(new_sock,&req);
    }
    else
    {
        printf("method is not supposed! method: %s\n",req.method);
        err_code = 404;
        goto END;
    }

END:
    // 对请求处理结束的收尾工作
    if(err_code != 200)
    {
        Handler404(new_sock);
    }
    // 由于是服务器主动断开连接,于是服务器上可能同一时间内有大量的time_wait状态
    // 这样不仅占用连接,也占用端口号
    // 所以需要 REUSEADDR
    close(new_sock);
}

void* ThreadEntry(void* arg)
{
    int new_sock = (uint64_t)arg;
    // 负责这一次的请求完整过程
    HandlerRequet(new_sock);
    return NULL;
}

void HttpServerStart(short port)
{
    // 1.创建 tcp socket
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0)
    {
        perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    // 2.绑定端口号
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))){
        perror("bind");
        return;
    }
    // 3.监听socket
    if(listen(fd,5)){
        perror("listen");
        return;
    }
    printf("HttpServerStart OK!\n");
    // 4.进入循环,处理客户端的链接
    while(1)
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        uint64_t new_sock = accept(fd,(struct sockaddr*)&peer,&len);
        if(new_sock < 0)
        {
            perror("accept");
            continue;
        }

        // 使用多线程处理多个连接并行
        
        // 注意这个new_sock的生命周期,以下的代码是错误的  
        // pthread_create(&tid,NULL,ThreadEntry,&new_sock);
        // 生命周期是这个while(1)循环,这个循环是转瞬即逝的,new_sock很快就被释放了
        // 在线程入口函数中new_sock已经被释放了,所以是访问无效地址
        //
        // 解决方法1: 静态全局变量 - NONONO
        // 因为这是一个多线程的服务器,大家都在使用,线程不安全,很可能出错
        //
        // 解决方法2: 堆上的变量 - OK 
        //   int* new_sock = (int*)malloc(sizeof(int));
        //   *new_sock = accept(...);
        //   记得在循环外部进行释放
        //
        // 解决方法3:
        // 将 new_sock 转换为 指针 传进去

        pthread_t tid; 
        pthread_create(&tid,NULL,ThreadEntry,(void*)new_sock);
        pthread_detach(tid);
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s [port]\n",argv[0]);
        return 1;
    }
    signal(SIGPIPE,SIG_IGN);
    HttpServerStart(atoi(argv[1]));
    return 0;
}
