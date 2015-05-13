CXX=g++
CXXFLAGS=-O1 -Wall
LIBS=-lgecodeflatzinc -lgecodedriver -lgecodegist -lgecodesearch -lgecodeminimodel \
	-lgecodeset -lgecodefloat -lgecodeint -lgecodekernel -lgecodesupport
RM=rm
SRCS0=no-overlap.cpp
#SRCS1=MatrixQueen.cpp
OBJS0=$(subst .cpp,.o,$(SRCS0))
#OBJS1=$(subst .cpp,.o,$(SRCS1))
EXEC=no-overlap

all: $(EXEC)

no-overlap: $(OBJS0)
	$(CXX) $(CXXFLAGS) -o no-overlap $(OBJS0) $(LIBS)

no-overlap.o: no-overlap.cpp

matrix_queen: $(OBJS1)
	$(CXX) $(CXXFLAGS) -o matrix_queen $(OBJS1) $(LIBS)

MatrixQueen.o: MatrixQueen.cpp

clean:
	$(RM) $(OBJS0)

dist-clean: clean
	$(RM) $(EXEC)
