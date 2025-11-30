# Compilador e flags
CC = g++
FLAGS = -std=c++17 -pthread -Wall -Wextra

# Diretórios e nomes de arquivos
SRC_DIR = src
BUILD_DIR = build
TARGET_NAME = zombie_game

ifeq ($(OS),Windows_NT)
	TARGET = $(TARGET_NAME).exe
	MKDIR_CMD = powershell -Command "New-Item -ItemType Directory -Force $(BUILD_DIR) | Out-Null"
	CLEAN_CMD = powershell -Command "Remove-Item -Recurse -Force $(BUILD_DIR), $(TARGET) -ErrorAction SilentlyContinue"
else
	TARGET = $(TARGET_NAME)
	MKDIR_CMD = mkdir -p $(BUILD_DIR)
	CLEAN_CMD = rm -rf $(BUILD_DIR) $(TARGET)
endif

# Arquivos
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))
HDRS = $(wildcard $(SRC_DIR)/*.h)

# Regra principal
all: setup_build $(TARGET)

setup_build:
	$(MKDIR_CMD)

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

rebuild: clean all