# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -std=c++11	set compilation version
#  -g    		adds debugging information to the executable file
#  -Wall 		turns on most, but not all, compiler warnings
# -lpthread		compile threads
CFLAGS = -std=c++14 -g -Wall
TFLAGS = -lpthread

# the build target executable:
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).cpp $(TARGET).hpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp $(TFLAGS)

clean:
	$(RM) $(TARGET)
