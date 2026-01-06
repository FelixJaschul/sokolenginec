# Project settings
PROJECT_NAME = GAME
BUILD_DIR = build
SRC_DIR = src
LIB_DIR = lib

# Compiler settings
CC = cc
CXX = c++
CFLAGS = -std=c11 -O2 -g -arch arm64
CXXFLAGS = -std=c++17 -O2 -g -arch arm64
INCLUDES = -I$(LIB_DIR)/sokol \
           -I$(LIB_DIR)/cimgui \
           -I$(LIB_DIR)/cimgui/imgui \
           -I$(LIB_DIR)/SDL/include \
           -I$(BUILD_DIR)/sdl_include

# Linker settings
LDFLAGS = -arch arm64
FRAMEWORKS = -framework OpenGL \
             -framework Cocoa \
             -framework IOKit \
             -framework CoreVideo \
             -framework CoreAudio \
             -framework AudioToolbox \
             -framework ForceFeedback \
             -framework Carbon \
             -framework Metal \
             -framework GameController

# Define SOKOL backend
DEFINES = -DSOKOL_GLCORE

# Source files
MAIN_SRC = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/game.c

# CImGui source files
CIMGUI_DIR = $(LIB_DIR)/cimgui
CIMGUI_SRCS = $(CIMGUI_DIR)/cimgui.cpp \
              $(CIMGUI_DIR)/imgui/imgui.cpp \
              $(CIMGUI_DIR)/imgui/imgui_demo.cpp \
              $(CIMGUI_DIR)/imgui/imgui_draw.cpp \
              $(CIMGUI_DIR)/imgui/imgui_tables.cpp \
              $(CIMGUI_DIR)/imgui/imgui_widgets.cpp

CIMGUI_OBJS = $(patsubst $(CIMGUI_DIR)/%.cpp,$(BUILD_DIR)/cimgui/%.o,$(CIMGUI_SRCS))

# SDL3 - we'll use the system or build from source
SDL_BUILD_DIR = $(BUILD_DIR)/sdl_build
SDL_LIB = $(SDL_BUILD_DIR)/libSDL3.dylib

# Object files
MAIN_OBJ = $(BUILD_DIR)/main.o

# Targets
.PHONY: all clean run setup sdl cimgui

all: setup sdl cimgui $(BUILD_DIR)/$(PROJECT_NAME)

setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/cimgui
	@mkdir -p $(BUILD_DIR)/cimgui/imgui
	@mkdir -p $(SDL_BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/sdl_include

# Build SDL3 from source
sdl: $(SDL_LIB)

$(SDL_LIB):
	@cd $(LIB_DIR)/SDL && \
	cmake -S . -B ../../$(SDL_BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_OSX_ARCHITECTURES=arm64 \
		-DSDL_SHARED=ON \
		-DSDL_STATIC=OFF \
		-DSDL_TEST=OFF \
		-DSDL_TESTS=OFF \
		2>&1 | grep -v "^--" || true
	@cmake --build $(SDL_BUILD_DIR) --target SDL3-shared -j8 2>&1 | grep -E "(Building|Linking|Built)" || true
	@cp -r $(SDL_BUILD_DIR)/include-revision/* $(BUILD_DIR)/sdl_include/ 2>/dev/null || true

# Build CImGui
cimgui: $(BUILD_DIR)/libcimgui.a

$(BUILD_DIR)/cimgui/%.o: $(CIMGUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1 -c $< -o $@

$(BUILD_DIR)/libcimgui.a: $(CIMGUI_OBJS)
	@ar rcs $@ $^

# Build main executable
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/$(PROJECT_NAME): $(MAIN_OBJ) $(BUILD_DIR)/libcimgui.a $(SDL_LIB)
	@$(CC) $(LDFLAGS) -o $@ $(MAIN_OBJ) \
		$(BUILD_DIR)/libcimgui.a \
		-L$(SDL_BUILD_DIR) -lSDL3 \
		$(FRAMEWORKS)

run: all
	@DYLD_LIBRARY_PATH=$(SDL_BUILD_DIR) ./$(BUILD_DIR)/$(PROJECT_NAME)

clean:
	@rm -rf $(BUILD_DIR)

# Install SDL3 library to system (optional)
install-sdl:
	@sudo cp $(SDL_LIB) /usr/local/lib/
	@sudo install_name_tool -id /usr/local/lib/libSDL3.dylib /usr/local/lib/libSDL3.dylib

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build everything (default)"
	@echo "  run          - Build and run the application"
	@echo "  clean        - Remove all build files"
	@echo "  sdl          - Build SDL3 only"
	@echo "  cimgui       - Build CImGui only"
	@echo "  install-sdl  - Install SDL3 to system (requires sudo)"
	@echo "  help         - Show this help message"
	@echo "  push         - Push current dir to remote"
	@echo "  push m=Hello - Sets commit message to Hello when pushing"

# Git Helpers

m=.

push: add commit
	git push

add: 
	git add Makefile lib src README.md 

commit:
	git commit -a -m "$(m)"
