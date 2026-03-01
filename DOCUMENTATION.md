# Документация yet-another survey

## Как подтянуть сабмодули (drogon и его зависимость)?

Надо один раз запустить код ниже, чтобы их всех подтянуть.
ВАЖНО: далее нужно НЕ запускать cmake . в корне, иначе в сабмодуле повятся изменения и придется отчистить его перед коммитом.

Чтобы подтянуть сабмодули:
```bash
git submodule update --init --recursive
```

## Сборка проекта (Qt6)

Проект теперь использует Qt6 (Widgets) для клиента, поэтому перед сборкой нужно установить Qt6.

Минимальные требования:
- CMake >= 3.16
- C++20 компилятор
- Qt6 (Widgets)

### macOS: проверить/установить Qt6

Проверить, установлен ли Qt6 через Homebrew:
```bash
brew list --versions qt@6
```

Если команда ничего не выводит, установить:
```bash
brew install qt@6
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

### Windows: проверить/установить Qt6

Проверить, установлен ли Qt6 через Chocolatey:
```bash
choco list --local-only | findstr /i qt6
```

Установка (Chocolatey):
```bash
choco install qt6
```

### Билд
Собираем билд, все будет лежать в папочке ./build в корне репозитория

```bash
cmake -S . -B build
cmake --build build --parallel
```

### Если запустился из корня cmake .

Если все же так случилось, исполняем в терминале код ниже и дальше так стараемся не делать :C

```bash
cd third_party/drogon
git clean -fdx
cd ../..
```

## Что и где запускается?

### Drogon
Сервер слушает 8080 порт на 127.0.0.1, на /file отдает файл с опросом через переданный параметр `id=`
ВАЖНО: запускать сервер перед запуском клиента, иначе клиент может завершиться/крашнуться

```bash
./build/server
```

### Клиент (Qt)

Клиент — Qt-приложение. Запускается отдельно:

```bash
./build/client
```
