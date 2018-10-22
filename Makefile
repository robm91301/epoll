ex : main.c 
	gcc main.c -lpthread -pthread  -o server 

clean:
	rm server 

format:
	indent -kr main.c
	rm main.c~
package:
	tar cfvz verizon.tar.gz main.c test.py Makefile server README
