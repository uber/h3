default: purge
	mkdir build
	just build

all:
	just default
	just fast
	just bench

build:
	cd build; cmake -DCMAKE_BUILD_TYPE=Release ..; make

purge:
	rm -rf build

test:
	cd build; make test

fast:
	cd build; make test-fast

bench:
	./build/bin/benchmarkIsValidCell

setup:
	# echo 'export PATH="/usr/local/opt/llvm@14/bin:$PATH"' >> ~/.zshrc  # didn't work
	brew install llvm@14

convert name="old":
	python tocsv.py {{name}}

loop name N:
	for i in $(seq 1 {{N}}); do echo "Iteration $i"; ./build/bin/benchmarkIsValidCell >> {{name}}.log; done
