.PHONY: clean run

cppneighbours: src/cppneighbours.cpp
	gcc src/cppneighbours.cpp -lstdc++ -o cppneighbours

clean:
	rm -f cppneighbours

run: cppneighbours
	./cppneighbours
