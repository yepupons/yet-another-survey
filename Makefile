.PHONY: configure build build-server build-client server client format clean

configure:
	cmake -S . -B build

build: configure
	cmake --build build --parallel

build-debug:
	cmake -S . -B build -DYAZ_DEBUG=ON
	cmake --build build --parallel

server:
	./build/server

client:
	./build/client

format:
	clang-format -i client/*/*.*pp server/*/*.*pp

clean:
	rm -rf build
