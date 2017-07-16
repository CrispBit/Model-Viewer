.PHONY: clean

linux: $(wildcard *.cpp)
	g++ -g -lGLEW -lGL -lboost_system -lboost_filesystem -lsfml-window -lsfml-system -lsfml-graphics -lassimp *.cpp Mesh/*.cpp -o meem

clean:
	rm -f meem
