# yet-another-survey (язь)


<p align="center">
  <img src="./preview.gif" alt="yet-another-survey preview" />
</p>

<p align="center">
   <img src="https://img.shields.io/badge/status-active%20development-59c135" alt="status">
  <img src="https://img.shields.io/badge/C%2B%2B-20-3c93d1?" alt="C++20">
  <img src="https://img.shields.io/badge/framework-Drogon-f28c38" alt="Drogon">
  <img src="https://img.shields.io/badge/Qt-6-41cd52" alt="Qt6">
  <img src="https://img.shields.io/badge/database-MongoDB-13aa52" alt="MongoDB">
  <img src="https://img.shields.io/badge/license-MIT-blueviolet" alt="MIT">
</p>


<p align="center">
  <b>Устали от дурацких и лагающих гугл форм?</b><br>
  Есть решение — ЯЗЬ! Blazingly fast, но не Rust — только C++, только хардкор.
</p>


## Что мы делаем?

**Yet another survey** — это ~~очередной~~ сервис для создания, раздачи и прохождения опросов и тестов.

Репозиторий состоит из клиентской (Qt) части и серверной (Drogon + MongoDB). Пользователь может создавать опросы и тесты, получать по ним статистику, проходить опросы и тесты (у тестов отображаются верные и неправильне ответы), а так же просмотреть созданные и пройденные опросы.


## Структура проекта
- `client` — все файлы клиентской части.
- `server` - все файлы сервера.
- `./src` — исходники сервера и клиента.
- `./include` — заголовочные файлы.
- `third_party` — сабмодули (Drogon и nlohmann).

***Стек проекта: C++20, Qt6, Drogon, MongoDB, libcurl, nlohmann.***

## Быстрый старт

### 1) Подтянуть сабмодули
```bash
git submodule update --init --recursive
```

**Важно: нужно проверить, установлен ли Qt и MongoDB**

Для графического интерфейса необходимо установить Qt, а для сервера Mongo, подробнее, как это делается, указано в `DOCUMENTATION.md`

### 2) Сборка и запуск
При всех установленных инструментах, для локального поднятия достаточно в трех разных терминалах прописать:
```bash 
make server
make bot
make web
```
Подробнее о том, как установить все модули, читайте в `DOCUMENTATION.md`.

## Что нужно для опроса?
Зайти в приложение и выбрать `Создать опрос` и интерактивно создать опрос из уже предложенных блоков.


## Документация
Подробнее: `DOCUMENTATION.md`

***made by yepupons, 2026***
