CC=gcc
DEPS= lex.yy
OBJ=$(DEPS:%=obj/%.o)
CFLAGS=-Wall -ly -lfl -pedantic -g 
EXEC=tpcc
PARSER=parser
LEXER=lexer

all : makedirs bin/$(EXEC)  

makedirs:
	@touch ./test/feedback.txt
	@touch _anonymous.asm
	@mkdir -p bin
	@mkdir -p obj

bin/$(EXEC): obj/lex.yy.o src/$(PARSER).tab.c src/*.c
	$(CC) -o $@ $^ 

src/lex.yy.c: src/$(LEXER).lex src/tree.h src/$(PARSER).tab.h
	flex -o $@ src/$(LEXER).lex

src/$(PARSER).tab.c src/$(PARSER).tab.h: src/$(PARSER).y
	bison -d src/$(PARSER).y
	@mv $(PARSER).tab.c src/
	@mv $(PARSER).tab.h src/

obj/lex.yy.o: src/lex.yy.c
	$(CC) -c src/lex.yy.c -o obj/lex.yy.o $(CFLAGS)


obj/%.o: src/%.c src/%.h
	$(CC) -c $< -o $@ $(CFLAGS) 

clean:
	rm -f src/lex.yy.* src/$(PARSER).tab.* obj/* bin/* ./_anonymous.* ./test/feedback.txt 

mrproper: clean
	rm -f ./output.txt ./out