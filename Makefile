main: source/main.c
	cc -Wextra -Wall -I./include source/main.c \
	-L./lib -lraylib -lm -ldl -lX11 -o main

run:
	./main