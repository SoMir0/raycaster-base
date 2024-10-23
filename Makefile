run: game
	./game

game: main.c
	cc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o game

clean:
	rm game
