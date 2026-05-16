.PHONY: configure build server client bot format clean

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

bot: configure
	cmake --build build --target telegram_bot
	BOT_TOKEN=$(BOT_TOKEN) ./build/telegram_bot
	
format:
	clang-format -i client/*/*.*pp server/*/*.*pp

clean:
	rm -rf build
