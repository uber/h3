default: purge
	mkdir build
	just build

all:
	just default
	just fast

build:
	cd build; cmake ..; make

purge:
	rm -rf build

test:
	cd build; make test

fast:
	cd build; make test-fast

setup:
	brew install llvm@14
	# echo 'export PATH="/usr/local/opt/llvm@14/bin:$PATH"' >> ~/.zshrc  # didn't work
