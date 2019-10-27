CXX = g++
CXXFLAGS = -Wall -std=c++11
DBGFLAGS = -g
IFLAGS = -I include
LFLAGS =
TARGET = fren
SRCSDIR = src
OBJSDIR = obj
DBGDIR = dbg
SRCS = $(wildcard $(SRCSDIR)/*.cpp)
OBJS = $(patsubst $(SRCSDIR)/%.cpp,$(OBJSDIR)/%.o,$(SRCS))
DBGS = $(patsubst $(SRCSDIR)/%.cpp,$(DBGDIR)/%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LFLAGS) $(OBJS) -o $(TARGET)

$(OBJSDIR)/%.o : $(SRCSDIR)/%.cpp
	mkdir -p $(OBJSDIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

$(DBGDIR)/%.o : $(SRCSDIR)/%.cpp
	mkdir -p $(DBGDIR)
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: clean debug
clean:
	rm -rf $(OBJSDIR)
	rm -rf $(DBGDIR)
	rm $(TARGET)

debug: $(DBGS)
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(LFLAGS) $(DBGS) -o $(TARGET)
