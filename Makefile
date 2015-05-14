CXX=g++
CXXFLAGS=-O1 -Wall
LIBS=-lgecodeflatzinc -lgecodedriver -lgecodegist -lgecodesearch -lgecodeminimodel \
	-lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport
RM=rm
#SRCS0=no-overlap.cpp
SRCS1=square.cpp
#OBJS0=$(subst .cpp,.o,$(SRCS0))
OBJS1=$(subst .cpp,.o,$(SRCS1))
EXEC=square

all: $(EXEC)

no-overlap: $(OBJS0)
	$(CXX) $(CXXFLAGS) -o no-overlap $(OBJS0) $(LIBS)

no-overlap.o: no-overlap.cpp

square: $(OBJS1)
	$(CXX) $(CXXFLAGS) -o square $(OBJS1) $(LIBS)

square.o: square.cpp

clean:
	$(RM) $(OBJS0) $(OBJS1)

dist-clean: clean
	$(RM) $(EXEC)
