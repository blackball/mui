CC=gcc
CXX=g++
LIBS+=`pkg-config --libs opencv x11`

demo:
	$(CXX) -c demo.cpp -O3 -march=native 
	$(CXX) -o $@ demo.o $(LIBS)

demo_sui:
	$(CC) -c sui.c -O3 -march=native
	$(CXX) -c -DUSE_SUI demo.cpp -O3 -march=native 
	$(CXX) -o $@ demo.o sui.o $(LIBS)

clean:
	rm -rf *.o demo demo_sui
