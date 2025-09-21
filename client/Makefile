CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I.
LDFLAGS := -lboost_system -lpthread

SRC_DIRS := . core network protocol
SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
OBJ := $(SRC:.cpp=.o)

TARGET := messageu_client

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
