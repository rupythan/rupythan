CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = src/main.c src/lexer.c src/ast.c src/parser.c src/eval.c
OUT = rupythan

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)