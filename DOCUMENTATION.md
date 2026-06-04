# Документация yet-another-survey

## Оглавление
- [Документация yet-another-survey](#документация-yet-another-survey)
  - [Оглавление](#оглавление)
  - [Как подтянуть сабмодули?](#как-подтянуть-сабмодули)
  - [Сборка проекта](#сборка-проекта)
    - [macOS: проверить/установить Qt6](#macos-проверитьустановить-qt6)
    - [macOS: установить и запустить mongodb](#macos-установить-и-запустить-mongodb)
    - [macos: установить gnuplot](#macos-установить-gnuplot)
    - [macOS: установить Boost](#macos-установить-boost)
    - [Linux: проверить/установить Qt6](#linux-проверитьустановить-qt6)
    - [Linux: установить и запустить mongodb](#linux-установить-и-запустить-mongodb)
    - [Linux: установить Boost](#linux-установить-boost)
    - [Windows: проверить/установить Qt6](#windows-проверитьустановить-qt6)
    - [Windows: установить и запустить mongodb](#windows-установить-и-запустить-mongodb)
    - [Windows: установить Boost](#windows-установить-boost)
    - [Билд](#билд)
    - [Билд веба](#билд-веба)
    - [Если запустился из корня cmake .](#если-запустился-из-корня-cmake-)
  - [Что и где запускается?](#что-и-где-запускается)
    - [Сервер](#сервер)
    - [Скрипт для демо-опросов](#скрипт-для-демо-опросов)
    - [Клиент (Qt)](#клиент-qt)
    - [Клиент (WebAssembly)](#клиент-webassembly)
    - [Telegram-бот](#telegram-бот)

## Как подтянуть сабмодули?
Надо один раз запустить код ниже, чтобы их всех подтянуть.

ВАЖНО: далее нужно НЕ запускать cmake . в корне, иначе в сабмодуле повятся изменения и придется отчистить его перед коммитом.

Чтобы подтянуть сабмодули:
```bash
git submodule update --init --recursive
```

## Сборка проекта
Проект использует Qt6 для клиента, MongoDB для сервера, gnuplot для экспорта графиков и Boost для Telegram-бота.


### macOS: проверить/установить Qt6

Проверить, установлен ли Qt6 через Homebrew:
```bash
brew list --versions qt@6
```

Если команда ничего не выводит, установить:
```bash
brew install qt@6
```

### macOS: установить и запустить mongodb
```bash
brew tap mongodb/brew
brew install mongodb-community
brew services start mongodb-community
```

### macos: установить gnuplot
```bash
brew install gnuplot
```

### macOS: установить Boost
Boost нужен для сборки Telegram-бота

```bash
brew install boost
```

### Linux: проверить/установить Qt6

Проверить, установлен ли Qt6 (Ubuntu/Debian):
```bash
dpkg -l | rg -i "qt6"
```

Установка (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install -y qt6-base-dev
```

Для Arch:
```bash
sudo pacman -S qt6-base
```

Для Fedora:
```bash
sudo dnf install qt6-qtbase-devel
```

### Linux: установить и запустить mongodb

Если MongoDB Community Server уже установлен из официального репозитория:
```bash
sudo systemctl start mongod
sudo systemctl enable mongod
```

Для Ubuntu (установка `mongodb-org` из официального репозитория):
```bash
sudo apt-get install gnupg curl
curl -fsSL https://pgp.mongodb.com/server-8.0.asc | sudo gpg -o /usr/share/keyrings/mongodb-server-8.0.gpg --dearmor
echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/8.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-8.0.list
sudo apt-get update
sudo apt-get install -y mongodb-org
sudo systemctl start mongod
sudo systemctl enable mongod
```

### Linux: установить Boost
Boost нужен для сборки Telegram-бота

Для Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y libboost-all-dev
```

Для Arch:
```bash
sudo pacman -S boost
```

Для Fedora:
```bash
sudo dnf install boost-devel
```

### Windows: проверить/установить Qt6

Проверить, установлен ли Qt6 через Chocolatey:
```bash
choco list --local-only | findstr /i qt6
```

Установка (Chocolatey):
```bash
choco install qt6
```

### Windows: установить и запустить mongodb

Установи MongoDB Community Server через MSI и включи опцию установки как Windows Service.
После установки запусти сервис через Windows Services (Services.msc), нажав Start у MongoDB.

### Windows: установить Boost
Boost нужен для сборки Telegram-бота

Через Chocolatey:
```bash
choco install boost-msvc-14.3
```

### Билд
Собираем билд, все будет лежать в папочке ./build в корне репозитория.
Можно собирать как ручками, так и использовать заготовленные команды для билда

```bash
cmake -S . -B build
cmake --build build --parallel
```
ИЛИ
```bash
make build
```
***(для дебага)***
```bash
make build-debug
```

### Билд веба
Для веб-сборки нужен Qt for WebAssembly под single thread и emsdk той версии, которую просит сайт Qt для твоей версии Qt.
Обычный Qt той же версии тоже нужен: он передается в `QT_HOST_PATH`.

Необходимо установить с сайта[https://www.qt.io/download-qt-installer-oss] was, в Qt Online Installer / MaintenanceTool надо выбрать Manual download и скачать именно `WebAssembly -> wasm_singlethread`.

Если Qt стоит через Qt Online Installer в `~/Qt`, а emsdk лежит в `~/emsdk`, `~/dev/emsdk`, `~/tools/emsdk` или `/opt/emsdk`, make попробует найти их сам.

Собираем через make:
```bash
make build-wasm
```

Если make сам не нашел Qt или emsdk, можно указать пути руками:
```bash
source /папка/куда/поставил/emsdk/emsdk_env.sh
make build-wasm QT_WASM_PATH=/папка/до/Qt_wasm_singlethread QT_HOST_PATH=/папка/до/Qt_обычного
```

ИЛИ руками через cmake:
```bash
/папка/до/Qt_wasm_singlethread/bin/qt-cmake -S . -B build-wasm -DQT_HOST_PATH=/папка/до/Qt_обычного -DQT_HOST_PATH_CMAKE_DIR=/папка/до/Qt_обычного/lib/cmake/Qt6
cmake --build build-wasm --parallel
```

Чтобы открыть веб-клиент локально:
```bash
make serve-wasm
```

После этого открываем `http://localhost:8054/client.html`.

### Если запустился из корня cmake .

Если все же так случилось, исполняем в терминале код ниже и дальше так стараемся не делать :C

```bash
cd third_party/drogon
git clean -fdx
cd ../..
```

## Что и где запускается?

### Сервер
ВАЖНО: надо запускать сервер перед запуском клиента, иначе клиент поругается и скажет ошибку

```bash
./build/server
```
ИЛИ
```bash
make server
```

### Скрипт для демо-опросов

Скрипт `scripts/seed_surveys.py` добавляет в MongoDB 15 демо-опросов: обычные опросы, тесты и квизы. У каждого опроса уже есть лайки, дизлайки и рейтинг.

Перед запуском MongoDB должна быть запущена. Сервер запускать не обязательно, скрипт пишет напрямую в базу `yas_db`.

Для `pymongo` лучше использовать виртуальное окружение:
```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install pymongo
```

Запуск:
```bash
python3 scripts/seed_surveys.py
```

По умолчанию скрипт создает опросы от пользователя `seed-user` и перед повторным запуском удаляет старые демо-опросы этого пользователя.

Если нужно создать опросы от конкретного пользователя:
```bash
python3 scripts/seed_surveys.py --creator-id USER_ID
```

Если нужно только добавить новые демо-опросы, не удаляя старые:
```bash
python3 scripts/seed_surveys.py --append
```

Если MongoDB запущена не на стандартном адресе:
```bash
python3 scripts/seed_surveys.py --uri mongodb://localhost:27017 --db yas_db
```

### Клиент (Qt)

Клиент — Qt-приложение. Запускается отдельно:

```bash
./build/client
```
ИЛИ
```bash
make client
```

### Клиент (WebAssembly)

Для веб-клиента сначала запускаем сервер, потом раздаем `build-wasm` через локальный http-сервер:

```bash
make server
```

В другом терминале:
```bash
make web
```

Открываем `http://localhost:8054/client.html`.

### Telegram-бот

Бот запускается отдельным процессом. Перед запуском нужно передать токен через переменную окружения `BOT_TOKEN`.

Передать токен только для одной команды:
```bash
BOT_TOKEN="your_bot_token" make bot
```

ИЛИ прописать токен в текущей bash-сессии:
```bash
export BOT_TOKEN="your_bot_token"
make bot
```

Если запускаешь бинарник напрямую:
```bash
export BOT_TOKEN="your_bot_token"
./build/telegram_bot
```

ВАЖНО: сервер должен быть запущен, потому что бот обращается к серверным endpoint-ам.
