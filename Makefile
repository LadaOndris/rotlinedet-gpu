CXX=nvc++
CXXFLAGS=-fast -acc=gpu -Minfo=accel -gpu=managed

.PHONY: clean

linedet: main.o linedet.o
	nvc++ $(CXXFLAGS) $^ -o $@

%.o: %.cpp %.hpp
	nvc++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f linedet *.o core *.lst *.ptx *.pgprof *.cubin *.s *.x *.mod *.nvprof

