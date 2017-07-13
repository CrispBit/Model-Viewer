.PHONY: clean

linux: $(wildcard *.cpp)
	g++ -g -lGLEW -lGL -lsfml-window -lsfml-system -lsfml-graphics -lassimp *.cpp Mesh/*.cpp -o meem

clean:
	rm -f meem
