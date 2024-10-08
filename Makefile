CXX = g++

ifeq ($(OS),Windows_NT)
    #mingw
    BUILD_CMD = g++ -I src/include -L src/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2
else
    #linux
    BUILD_CMD = g++ -o main main.cpp -lSDL2main -lSDL2
endif

build:
	$(BUILD_CMD)

clean:
	rm -f main
