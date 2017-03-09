# CC and CFLAGS are varilables
CC = g++
CFLAGS = -c
AR = ar
ARFLAGS = rcv
# -c option ask g++ to compile the source files, but do not link.
# -g option is for debugging version
# -O2 option is for optimized version
DBGFLAGS = -g -D_DEBUG_ON_
OPTFLAGS = -O2

all	: FMPartition
	@echo -n ""

# optimized version
FMPartition	: fm_partition.o main.o
			$(CC) $(OPTFLAGS) fm_partition.o main.o -o FMPartition
main.o 	   	: src/main.cpp
			$(CC) $(CFLAGS) $< -o $@
fm_partition.o	: src/fm_partition.cpp src/fm_partition.h
			$(CC) $(CFLAGS) $(OPTFLAGS) $< -o $@


# clean all the .o and executable files
clean:
		rm -rf *.o lib/*.a lib/*.o bin/*

