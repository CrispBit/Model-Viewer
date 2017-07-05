.PHONY: clean

linux: $(wildcard *.cpp)
	g++ -lGLEW -lX11 -lGL -lsfml-window -lsfml-system -lsfml-graphics -lassimp *.cpp -o meem

clean:
	rm -f meem
