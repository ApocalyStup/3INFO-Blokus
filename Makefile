CC=g++
BIN=bin
LIBS=-lsfml-graphics -lsfml-window -lsfml-system
INCLUDE=-I /SFML-2.5.1/include
CFLAGS=-g -Wfatal-errors -fopenmp -ffast-math -O3 -c -Wall -std=c++17 $(INCLUDE)
LDFLAGS=-fopenmp -std=c++14 #-lprofiler -Wl,-no_pie
SOURCES=blokus.cpp omp_util.cpp fast_log.cpp statistics.cpp allocator.cpp main.cpp shape.cpp
OBJECTS=$(addprefix $(BIN)/, $(SOURCES:.cpp=.o))
EXECUTABLE=$(BIN)/main
vpath %.cpp src

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

-include $(BIN)/$(OBJECTS:.o=.d)

$(BIN)/%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $(BIN)/$*.o
	$(CC) -MM $(CFLAGS) $< > $(BIN)/$*.d
	@mv -f $(BIN)/$*.d $(BIN)/$*.d.tmp
	@sed -e 's|.*:|$(BIN)/$*.o:|' < $(BIN)/$*.d.tmp > $(BIN)/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(BIN)/$*.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> $(BIN)/$*.d
	@rm -f $(BIN)/$*.d.tmp

clean:
	rm -f $(BIN)/*.o $(BIN)/*.d $(EXECUTABLE)
