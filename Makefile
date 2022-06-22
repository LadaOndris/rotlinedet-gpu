CXX=nvc++
CXXFLAGS=-fast -acc=gpu -Minfo=all

.PHONY: clean

rotlinedet: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o core *.lst *.ptx *.pgprof *.cubin *.s *.x *.mod *.nvprof
