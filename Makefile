CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall

# Defines path to source files
SRC_DIR = src

# Targets
TARGET = zombie_game
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp
OBJS = $(SRCS:.cpp=.o)
HDRS = $(SRC_DIR)/game.h $(SRC_DIR)/config.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .cpp files in src/ to .o files in src/
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

rebuild: clean all