CC=g++ -std=c++11
CFLAGS= -o
LDFLAGS=
SOURCES= NetNinny.cc ServerFunctions.cc ClientFunctions.cc 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Nattinatti

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@
