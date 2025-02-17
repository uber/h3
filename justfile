default: purge
	mkdir build
	cd build; cmake ..; make

purge:
	rm -rf build

test:
	cd build; make test

fast:
	cd build; make test-fast
