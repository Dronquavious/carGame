CXX = g++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

main: src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o src/main $(LDFLAGS)

clean:
	rm -f src/main