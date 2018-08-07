.PHONY: all
all:hello select insert

hello:hello.c
	gcc -o $@ $^

select:selectData.c
	gcc -o $@ $^ -L ./lib  -lmysqlclient # -I ./include 

insert:insertData.c
	gcc -o $@ $^ -L ./lib  -lmysqlclient # -I ./include 

.PHONY:clean
clean:
	rm hello select insert
