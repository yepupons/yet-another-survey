# Документация yet-another-survey

## Оглавление
- [Документация yet-another-survey](#документация-yet-another-survey)
  - [Оглавление](#оглавление)
  - [Как подтянуть сабмодули?](#как-подтянуть-сабмодули)
  - [Сборка проекта](#сборка-проекта)
    - [macOS: проверить/установить Qt6](#macos-проверитьустановить-qt6)
    - [macOS: установить и запустить mongodb](#macos-установить-и-запустить-mongodb)
    - [macos: установить gnuplot](#macos-установить-gnuplot)
    - [Linux: проверить/установить Qt6](#linux-проверитьустановить-qt6)
    - [Linux: установить и запустить mongodb](#linux-установить-и-запустить-mongodb)
    - [Windows: проверить/установить Qt6](#windows-проверитьустановить-qt6)
    - [Windows: установить и запустить mongodb](#windows-установить-и-запустить-mongodb)
    - [Билд](#билд)
    - [Если запустился из корня cmake .](#если-запустился-из-корня-cmake-)
  - [Что и где запускается?](#что-и-где-запускается)
    - [Сервер](#сервер)
    - [Клиент (Qt)](#клиент-qt)

## Как подтянуть сабмодули?
Надо один раз запустить код ниже, чтобы их всех подтянуть.

ВАЖНО: далее нужно НЕ запускать cmake . в корне, иначе в сабмодуле повятся изменения и придется отчистить его перед коммитом.

Чтобы подтянуть сабмодули:
```bash
git submodule update --init --recursive
```

## Сборка проекта
Проект теперь использует Qt6для клиента, поэтому перед сборкой нужно установить его установить.


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

### Клиент (Qt)

Клиент — Qt-приложение. Запускается отдельно:

```bash
./build/client
```
ИЛИ
```bash
make client
```
