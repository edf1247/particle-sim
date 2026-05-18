all:
	gcc -o sim main.c -lraylib -lm -lX11
clean:
	rm sim
