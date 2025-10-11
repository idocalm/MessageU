CXX := g++ -g -O0 -fsanitize=address -fno-omit-frame-pointer
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
	@mkdir -p tests
	@mkdir -p tests/client1 tests/client2
	@cp $(TARGET) tests/client1/
	@cp $(TARGET) tests/client2/
	@cp server.info tests/client1/ 
	@cp server.info tests/client2/

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
