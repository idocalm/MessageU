CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I. -mrdrnd
LDFLAGS := -lboost_system -lpthread -lcryptopp
OBJDIR := build

SRC_DIRS := . core network protocol crypto
SRC := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

TARGET := messageu_client

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
