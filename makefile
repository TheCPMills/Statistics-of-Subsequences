CC = g++
CFLAGS = -O3 -I -pthread

main: ./eigen-3.4.0/ ./Parallel.cpp $(CC) $(CFLAGS) ./eigen-3.4.0/ ./Parallel.cpp -o Parallel

chunk: $(CC) $(CFLAGS) ./eigen-3.4.0/ ./BetterMemNew.cpp -o ParallelChunked

multiP: $(CC) $(CFLAGS) ./eigen-3.4.0/ ./ParallelMulti.cpp -o ParallelMulti

clean: rm -f Parallel ParallelChunked ParallelMulti