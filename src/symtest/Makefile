CXX = g++
IFLAGS = -I/usr/local/include/FSMlib -I/usr/local/include/bumblelib -I/usr/local/include/stlport
OFLAGS = -g
WFLAGS = -Wall

LFLAGS = -L/usr/local/lib -lm -lFSMSymSpec

TARGETS = symtest ittest

all: $(TARGETS)

.cc.o:
	$(CXX) $(WFLAGS) $(IFLAGS) $(OFLAGS) -c -o $@ $<

ittest: ittest.o
	$(CXX) $(WFLAGS) $(IFLAGS) $(OFLAGS) -o $@ $< -lstlport_gcc -lpthread

symtest: symtest.o
	$(CXX) $(WFLAGS) $(OFLAGS) -o $@ $^ $(LFLAGS)

clean:
	rm -f *~ *.o $(TARGETS)
