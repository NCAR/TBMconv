
# Copyright (c) 2016, University Corporation for Atmospheric Research
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
F77 = g77
F77FLAGS = -fdefault-integer-8
CC = gcc
CFLAGS = -g -Wall

CXX = g++
CXXFLAGS = -g -Wall -pedantic

F77_TARGETS = tbm2cos
CXX_TARGETS = tbmconv tbmexplore

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
