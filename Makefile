default:
	g++ main.cpp -o build/main

run:
	cd build && ./main

clean:
	rm -rf build/main
