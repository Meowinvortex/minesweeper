# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = 

# SFML and TGUI paths (modify these if you installed to custom locations)
SFML_INCLUDE = /usr/local/include
SFML_LIB = /usr/local/lib
TGUI_INCLUDE = /usr/local/include
TGUI_LIB = /usr/local/lib

# SFML and TGUI libraries to link
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network
TGUI_LIBS = -ltgui

# Add include paths
INCLUDES = -I$(SFML_INCLUDE) -I$(TGUI_INCLUDE)

# Add library paths
LIB_PATHS = -L$(SFML_LIB) -L$(TGUI_LIB)

# Source files
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = main

# Build rules
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXEC) $(LIB_PATHS) $(SFML_LIBS) $(TGUI_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

c:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean