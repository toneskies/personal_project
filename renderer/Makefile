CC = gcc
TARGET = renderer
# CHANGE: Use wildcard to grab all C files in src/
SRC = $(wildcard src/*.c)

INCLUDE_PATHS = -I/opt/homebrew/include
LIBRARY_PATHS = -L/opt/homebrew/lib
LIBRARIES = -lSDL2

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean