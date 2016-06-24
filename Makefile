F77 = g77
F77FLAGS = -fdefault-integer-8
CC = gcc
CFLAGS = -g -Wall

CXX = g++
CXXFLAGS = -g -Wall -pedantic

F77_TARGETS = tbm2cos
CXX_TARGETS = tbmconv bk

CXX_OBJS = cdc.o tbm.o

#TARGETS = $(F77_TARGETS)
TARGETS = $(CXX_TARGETS)

all: $(TARGETS)

tbm2cos: t.f
	$(F77) $(FFLAGS) $< -o $@

$(CXX_OBJS): %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

$(CXX_TARGETS): % : %.cpp $(CXX_OBJS)
	$(CXX) $(CXXFLAGS) $(CXX_OBJS) $< -o $@

$(C_TARGETS): % : %.c
	$(CC) $(CFLAGS) $< -o $@
