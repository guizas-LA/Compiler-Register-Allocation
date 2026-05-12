CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -Iinclude -I.

TARGET = myProg
SOURCES = src/ui/main.cpp src/ui/BatchMode.cpp src/ui/InteractiveMode.cpp src/ui/Menu.cpp src/parser/Parser.cpp src/parser/parserTools.cpp data_structures/Heap.cpp data_structures/UFDS.cpp

.PHONY: all clean run docs

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

docs:
	doxygen Doxyfile

clean:
	rm -f $(TARGET)
