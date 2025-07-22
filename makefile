stage1exe: lexer.o parser.o driver.o
	gcc -o stage1exe lexer.o parser.o driver.o

lexer.o: lexer.c lexerDef.h lexer.h
	gcc -c lexer.c

parser.o: parser.c parserDef.h parser.h lexer.h
	gcc -c parser.c

driver.o: driver.c lexer.h parser.h
	gcc -c driver.c

clean:
	rm -f program lexer.o parser.o driver.o