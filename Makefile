QT_ROOT ?= $(HOME)/Qt
QT_WASM_PATH ?= $(firstword $(wildcard $(QT_ROOT)/*/wasm_singlethread) $(wildcard $(QT_ROOT)/*/wasm_32))
QT_HOST_PATH ?= $(firstword $(wildcard $(dir $(QT_WASM_PATH))macos) $(wildcard $(dir $(QT_WASM_PATH))gcc_64) $(wildcard $(dir $(QT_WASM_PATH))clang_64) $(wildcard $(dir $(QT_WASM_PATH))msvc*))
EMSDK_ENV ?= $(firstword $(wildcard $(HOME)/emsdk/emsdk_env.sh) $(wildcard $(HOME)/dev/emsdk/emsdk_env.sh) $(wildcard $(HOME)/tools/emsdk/emsdk_env.sh) $(wildcard /opt/emsdk/emsdk_env.sh))
EM_CACHE ?= $(CURDIR)/build-wasm/.emscripten_cache
CCACHE_DIR ?= $(CURDIR)/build-wasm/.ccache
WASM_ENV = EM_CACHE="$(EM_CACHE)" CCACHE_DIR="$(CCACHE_DIR)"
EMSDK_SOURCE = $(if $(EMSDK_ENV),EMSDK_QUIET=1 . "$(EMSDK_ENV)" >/dev/null && ,)$(WASM_ENV)

.PHONY: configure configure-wasm check-wasm-env build build-wasm build-debug server client bot serve-wasm web format clean

configure:
	cmake -S . -B build

check-wasm-env:
	@test -n "$(QT_WASM_PATH)" || (echo "Qt for WebAssembly not found. Set QT_WASM_PATH=/path/to/Qt/wasm_singlethread or QT_ROOT=/path/to/Qt."; exit 1)
	@test -x "$(QT_WASM_PATH)/bin/qt-cmake" || (echo "qt-cmake not found at $(QT_WASM_PATH)/bin/qt-cmake"; exit 1)
	@test -n "$(QT_HOST_PATH)" || (echo "Qt host not found near $(QT_WASM_PATH). Set QT_HOST_PATH=/path/to/desktop/Qt."; exit 1)
	@test -d "$(QT_HOST_PATH)/lib/cmake/Qt6" || (echo "Qt host cmake dir not found at $(QT_HOST_PATH)/lib/cmake/Qt6"; exit 1)
	@if [ -n "$(EMSDK_ENV)" ]; then EMSDK_QUIET=1 . "$(EMSDK_ENV)" >/dev/null; fi; $(WASM_ENV) emcc --version >/dev/null || (echo "emsdk is not active. Run: source /path/to/emsdk/emsdk_env.sh"; exit 1)

configure-wasm: check-wasm-env
	$(EMSDK_SOURCE) "$(QT_WASM_PATH)/bin/qt-cmake" -S . -B build-wasm -DQT_HOST_PATH="$(QT_HOST_PATH)" -DQT_HOST_PATH_CMAKE_DIR="$(QT_HOST_PATH)/lib/cmake/Qt6"

build: configure
	cmake --build build --parallel

build-wasm: configure-wasm
	$(EMSDK_SOURCE) cmake --build build-wasm --parallel

server:
	./build/server

client:
	./build/client

bot:
	BOT_TOKEN=$(BOT_TOKEN)
	./build/telegram_bot

web:
	test -d build-wasm
	python3 -m http.server 8054 --directory build-wasm
	
format:
	clang-format -i client/*/*.*pp server/*/*.*pp

clean:
	rm -rf build build-wasm
