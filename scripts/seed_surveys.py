#!/usr/bin/env python3

# this script was written in order to test our system for correctnes of showing all types of surveys
# as soon as it's python, the code below isn't to be reviewed 

import argparse
import copy
import random
import uuid
from typing import Any

try:
    from pymongo import MongoClient
except ImportError as exc:
    raise SystemExit(
        "pymongo is required. Install it with: python3 -m pip install pymongo"
    ) from exc


SURVEY_TOPICS = [
    {
        "title": "Учебная продуктивность",
        "description": "Короткий опрос про планирование, концентрацию и учебные привычки.",
        "questions": [
            {
                "type": "single",
                "text": "Когда вы обычно планируете свой день?",
                "required": True,
                "options": ["Утром", "Вечером накануне", "По ходу дня"],
            },
            {
                "type": "multiple",
                "text": "Какие инструменты помогают вам не отвлекаться?",
                "required": False,
                "options": ["Календарь", "Таймер", "Доска задач", "Заметки"],
            },
            {
                "type": "text",
                "text": "Какая привычка сильнее всего улучшила вашу продуктивность?",
                "required": False,
            },
        ],
    },
    {
        "title": "Питание в университете",
        "description": "Опрос про обеды, кафе и качество еды рядом с учебой.",
        "questions": [
            {
                "type": "single",
                "text": "Как часто вы едите в университете или рядом с ним?",
                "required": True,
                "options": ["Каждый день", "Несколько раз в неделю", "Редко"],
            },
            {
                "type": "multiple",
                "text": "Что кафе стоит улучшить в первую очередь?",
                "required": False,
                "options": ["Цены", "Скорость очереди", "Вегетарианские блюда", "Кофе"],
            },
        ],
    },
    {
        "title": "Рабочее место дома",
        "description": "Опрос про комфорт удаленной работы и организацию рабочего места.",
        "questions": [
            {
                "type": "single",
                "text": "Насколько удобно ваше текущее рабочее место?",
                "required": True,
                "options": ["Очень удобно", "Терпимо", "Нужно улучшать"],
            },
            {
                "type": "text",
                "text": "Что бы вы улучшили в своем рабочем месте в первую очередь?",
                "required": False,
            },
        ],
    },
    {
        "title": "Обратная связь по мобильному приложению",
        "description": "Опрос про удобство интерфейса, дизайн и недостающие функции.",
        "questions": [
            {
                "type": "single",
                "text": "Насколько легко ориентироваться в приложении?",
                "required": True,
                "options": ["Легко", "Иногда непонятно", "Сложно"],
            },
            {
                "type": "multiple",
                "text": "Какие части приложения требуют доработки?",
                "required": False,
                "options": ["Вход", "Создание опросов", "Статистика", "Скорость работы"],
            },
        ],
    },
    {
        "title": "Планирование поездок",
        "description": "Опрос про выбор направлений, бюджет и формат путешествий.",
        "questions": [
            {
                "type": "single",
                "text": "Что важнее всего при выборе направления?",
                "required": True,
                "options": ["Бюджет", "Погода", "Культура", "Природа"],
            },
            {
                "type": "text",
                "text": "Опишите идеальную поездку на выходные.",
                "required": False,
            },
        ],
    },
]

TEST_TOPICS = [
    {
        "title": "Базовый тест по C++",
        "description": "Небольшой тест по синтаксису C++ и стандартным контейнерам.",
        "questions": [
            {
                "type": "single",
                "text": "В каком заголовке объявлен std::vector?",
                "required": True,
                "options": ["<array>", "<vector>", "<map>", "<list>"],
                "answer": 2,
            },
            {
                "type": "multiple",
                "text": "Какие типы являются стандартными последовательными контейнерами?",
                "required": True,
                "options": ["std::vector", "std::deque", "std::map", "std::list"],
                "answer": [1, 2, 4],
            },
        ],
    },
    {
        "title": "Основы HTTP и API",
        "description": "Тест на базовое понимание HTTP-методов и кодов ответа.",
        "questions": [
            {
                "type": "single",
                "text": "Какой HTTP-метод чаще всего используют для создания ресурса?",
                "required": True,
                "options": ["GET", "POST", "HEAD", "OPTIONS"],
                "answer": 2,
            },
            {
                "type": "text",
                "text": "Какой код ответа обычно означает, что пользователь не авторизован?",
                "required": True,
                "answer": ["401", "401 Unauthorized", "401 Не авторизован"],
            },
        ],
    },
    {
        "title": "Основы баз данных",
        "description": "Короткий тест про идентификаторы, индексы и документы.",
        "questions": [
            {
                "type": "single",
                "text": "Для чего в первую очередь нужен индекс в базе данных?",
                "required": True,
                "options": [
                    "Хранить пароли",
                    "Ускорять поиск",
                    "Сжимать изображения",
                    "Шифровать коллекции",
                ],
                "answer": 2,
            },
            {
                "type": "multiple",
                "text": "Какие поля полезны для аудита изменений?",
                "required": True,
                "options": ["created_at", "updated_at", "owner_id", "random_color"],
                "answer": [1, 2, 3],
            },
        ],
    },
    {
        "title": "Тест по безопасности",
        "description": "Проверка базовых понятий безопасности веб-приложений.",
        "questions": [
            {
                "type": "single",
                "text": "Что лучше хранить в БД вместо сырого access token?",
                "required": True,
                "options": ["Хэш токена", "Скриншот токена", "Токен в открытом виде"],
                "answer": 1,
            },
            {
                "type": "text",
                "text": "Какой HTTP-заголовок обычно используют для bearer auth?",
                "required": True,
                "answer": ["Authorization", "authorization"],
            },
        ],
    },
    {
        "title": "Основы Qt",
        "description": "Короткий тест по виджетам Qt и сигналам.",
        "questions": [
            {
                "type": "single",
                "text": "Какой механизм связывает события и обработчики в Qt?",
                "required": True,
                "options": ["Сигналы и слоты", "SQL-триггеры", "CSS-селекторы"],
                "answer": 1,
            },
            {
                "type": "multiple",
                "text": "Какие классы являются Qt-виджетами?",
                "required": True,
                "options": ["QWidget", "QLabel", "std::vector", "QPushButton"],
                "answer": [1, 2, 4],
            },
        ],
    },
]

QUIZ_TOPICS = [
    {
        "title": "Какая роль в проекте вам ближе?",
        "description": "Квиз сопоставляет предпочтения с ролями в команде.",
        "outcomes": ["Фронтенд", "Бэкенд", "Продукт"],
        "questions": [
            {
                "type": "single",
                "text": "Какая задача звучит для вас интереснее?",
                "required": True,
                "options": [
                    "Доводить интерфейс до идеала",
                    "Проектировать API",
                    "Расставлять приоритеты фич",
                ],
                "scores": ["Фронтенд", "Бэкенд", "Продукт"],
            },
            {
                "type": "single",
                "text": "Что вы первым замечаете в продукте?",
                "required": True,
                "options": ["Визуальный сценарий", "Модель данных", "Пользу для людей"],
                "scores": ["Фронтенд", "Бэкенд", "Продукт"],
            },
        ],
    },
    {
        "title": "Ваш стиль обучения",
        "description": "Квиз про то, как вам удобнее изучать новый материал.",
        "outcomes": ["Практика", "Теория", "Обсуждение"],
        "questions": [
            {
                "type": "single",
                "text": "С чего вы начинаете изучать новую тему?",
                "required": True,
                "options": ["Собираю пример", "Читаю документацию", "Спрашиваю у людей"],
                "scores": ["Практика", "Теория", "Обсуждение"],
            },
            {
                "type": "single",
                "text": "Какой формат помогает лучше запомнить материал?",
                "required": True,
                "options": ["Упражнения", "Структурные конспекты", "Разбор в группе"],
                "scores": ["Практика", "Теория", "Обсуждение"],
            },
        ],
    },
    {
        "title": "Идеальные выходные",
        "description": "Квиз про отдых, хобби и уровень энергии.",
        "outcomes": ["Активный отдых", "Творчество", "Спокойствие"],
        "questions": [
            {
                "type": "single",
                "text": "Что лучше всего звучит после тяжелой недели?",
                "required": True,
                "options": ["Долгая прогулка", "Сделать что-то руками", "Почитать дома"],
                "scores": ["Активный отдых", "Творчество", "Спокойствие"],
            },
            {
                "type": "single",
                "text": "Какой план вам ближе?",
                "required": True,
                "options": ["Активный", "Гибкий", "Спокойный"],
                "scores": ["Активный отдых", "Творчество", "Спокойствие"],
            },
        ],
    },
    {
        "title": "Стиль принятия решений",
        "description": "Квиз описывает, как вы обычно принимаете решения.",
        "outcomes": ["Аналитик", "Быстрый старт", "Командный подход"],
        "questions": [
            {
                "type": "single",
                "text": "Что вы делаете перед сложным решением?",
                "required": True,
                "options": ["Сравниваю данные", "Доверяю интуиции", "Советуюсь с командой"],
                "scores": ["Аналитик", "Быстрый старт", "Командный подход"],
            },
            {
                "type": "single",
                "text": "Что делает решение убедительным?",
                "required": True,
                "options": ["Факты", "Темп", "Согласие участников"],
                "scores": ["Аналитик", "Быстрый старт", "Командный подход"],
            },
        ],
    },
    {
        "title": "Какой опрос вам стоит создать?",
        "description": "Квиз помогает выбрать тему следующего опроса.",
        "outcomes": ["Образование", "Продукт", "Образ жизни"],
        "questions": [
            {
                "type": "single",
                "text": "Кого вам интереснее опросить?",
                "required": True,
                "options": ["Студентов", "Пользователей приложения", "Друзей"],
                "scores": ["Образование", "Продукт", "Образ жизни"],
            },
            {
                "type": "single",
                "text": "Какой результат для вас полезнее?",
                "required": True,
                "options": ["Пробелы в знаниях", "Приоритеты фич", "Предпочтения людей"],
                "scores": ["Образование", "Продукт", "Образ жизни"],
            },
        ],
    },
]


EXTRA_SURVEY_QUESTIONS = [
    {
        "type": "single",
        "text": "Насколько эта тема актуальна для вас сейчас?",
        "required": True,
        "options": ["Очень актуальна", "Скорее актуальна", "Не очень актуальна"],
    },
    {
        "type": "multiple",
        "text": "Что влияет на ваше мнение по этой теме?",
        "required": False,
        "options": ["Личный опыт", "Советы знакомых", "Отзывы", "Цена", "Удобство"],
    },
    {
        "type": "single",
        "text": "Как часто вы сталкиваетесь с этой ситуацией?",
        "required": True,
        "options": ["Почти каждый день", "Несколько раз в неделю", "Редко"],
    },
    {
        "type": "text",
        "text": "Что бы вы предложили улучшить в первую очередь?",
        "required": False,
    },
    {
        "type": "multiple",
        "text": "Какие критерии для вас самые важные?",
        "required": False,
        "options": ["Качество", "Скорость", "Простота", "Надежность", "Поддержка"],
    },
]

EXTRA_TEST_QUESTIONS = [
    {
        "type": "single",
        "text": "Что обычно означает код ответа 404?",
        "required": True,
        "options": ["Успешный запрос", "Ресурс не найден", "Ошибка авторизации"],
        "answer": 2,
    },
    {
        "type": "multiple",
        "text": "Какие элементы помогают писать поддерживаемый код?",
        "required": True,
        "options": ["Понятные имена", "Тесты", "Дублирование логики", "Малые функции"],
        "answer": [1, 2, 4],
    },
    {
        "type": "text",
        "text": "Как называется формат обмена данными с фигурными скобками и массивами?",
        "required": True,
        "answer": ["JSON", "json"],
    },
    {
        "type": "single",
        "text": "Что лучше использовать для уникального публичного ID?",
        "required": True,
        "options": ["UUID", "Пароль пользователя", "Случайную букву"],
        "answer": 1,
    },
    {
        "type": "multiple",
        "text": "Какие проверки стоит делать на сервере?",
        "required": True,
        "options": ["Типы полей", "Диапазоны значений", "Авторизацию", "Цвет кнопок"],
        "answer": [1, 2, 3],
    },
]


def quiz_extra_questions(outcomes: list[str]) -> list[dict[str, Any]]:
    return [
        {
            "type": "single",
            "text": "Какой подход вам ближе в неопределенной ситуации?",
            "required": True,
            "options": [
                "Сначала разобраться в деталях",
                "Быстро попробовать вариант",
                "Обсудить с другими",
            ],
            "scores": outcomes,
        },
        {
            "type": "single",
            "text": "Что сильнее мотивирует вас продолжать?",
            "required": True,
            "options": [
                "Понятный план",
                "Быстрый прогресс",
                "Обратная связь",
            ],
            "scores": outcomes,
        },
        {
            "type": "single",
            "text": "Как вы предпочитаете оценивать результат?",
            "required": True,
            "options": [
                "По фактам и метрикам",
                "По ощущению движения вперед",
                "По реакции людей",
            ],
            "scores": outcomes,
        },
    ]


def expanded_questions(
    topic: dict[str, Any],
    survey_type: str,
    rng: random.Random,
    target_count: int = 4,
) -> list[dict[str, Any]]:
    questions = copy.deepcopy(topic["questions"])
    if survey_type == "survey":
        extras = copy.deepcopy(EXTRA_SURVEY_QUESTIONS)
    elif survey_type == "test":
        extras = copy.deepcopy(EXTRA_TEST_QUESTIONS)
    else:
        extras = quiz_extra_questions(topic["outcomes"])

    rng.shuffle(extras)
    for question in extras:
        if len(questions) >= target_count:
            break
        questions.append(question)
    return questions


def split_sections(questions: list[dict[str, Any]], rng: random.Random) -> list[dict[str, Any]]:
    if len(questions) <= 2 or rng.random() < 0.45:
        return [
            {
                "title": "Основной раздел",
                "questions": questions,
                "next_section_id": -1,
            }
        ]

    pivot = max(1, len(questions) // 2)
    return [
        {
            "title": "Базовые вопросы",
            "questions": questions[:pivot],
            "next_section_id": 1,
        },
        {
            "title": "Подробности",
            "questions": questions[pivot:],
            "next_section_id": -1,
        },
    ]


def make_survey(topic: dict[str, Any], survey_type: str, creator_id: str, rng: random.Random) -> dict[str, Any]:
    likes = rng.randint(5, 35)
    dislikes = rng.randint(0, 15)
    is_public = rng.random() >= 0.25
    survey_id = str(uuid.uuid4())
    questions = expanded_questions(topic, survey_type, rng)
    survey = {
        "data": {
            "id": survey_id,
            "creator_id": creator_id,
            "type": survey_type,
            "is_public": is_public,
            "preview_image": f"default:{rng.randrange(6)}",
            "likes_count": likes,
            "dislikes_count": dislikes,
            "ratings_count": likes + dislikes,
            "rating_score": likes - dislikes,
        },
        "title": topic["title"],
        "description": topic["description"],
        "sections": split_sections(questions, rng),
    }
    if survey_type == "quiz":
        survey["outcomes"] = topic["outcomes"]
    return survey


def seed(uri: str, database: str, creator_id: str, append: bool) -> None:
    rng = random.Random(42)
    client = MongoClient(uri)
    db = client[database]

    if not append:
        survey_ids = [
            doc["data"]["id"]
            for doc in db.surveys.find(
                {"data.creator_id": creator_id},
                {"data.id": 1, "_id": 0},
            )
            if "data" in doc and "id" in doc["data"]
        ]
        if survey_ids:
            db.surveys.delete_many({"data.id": {"$in": survey_ids}})
            db.answers.delete_many({"data.survey_id": {"$in": survey_ids}})
            db.survey_rates.delete_many({"survey_id": {"$in": survey_ids}})
        db.users.delete_one({"id": creator_id})

    surveys: list[dict[str, Any]] = []
    for topic in SURVEY_TOPICS:
        surveys.append(make_survey(topic, "survey", creator_id, rng))
    for topic in TEST_TOPICS:
        surveys.append(make_survey(topic, "test", creator_id, rng))
    for topic in QUIZ_TOPICS:
        surveys.append(make_survey(topic, "quiz", creator_id, rng))

    db.surveys.insert_many(surveys)
    db.users.update_one(
        {"id": creator_id},
        {
            "$setOnInsert": {
                "id": creator_id,
                "given_answers": [],
            },
            "$set": {
                "created_surveys": [survey["data"]["id"] for survey in surveys],
            },
        },
        upsert=True,
    )

    type_counts = {
        "survey": sum(1 for survey in surveys if survey["data"]["type"] == "survey"),
        "test": sum(1 for survey in surveys if survey["data"]["type"] == "test"),
        "quiz": sum(1 for survey in surveys if survey["data"]["type"] == "quiz"),
    }
    print(f"Добавлено опросов: {len(surveys)} в {database}.surveys")
    print(f"Опросы: {type_counts['survey']}")
    print(f"Тесты: {type_counts['test']}")
    print(f"Квизы: {type_counts['quiz']}")
    print(f"ID создателя: {creator_id}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Добавить демо-опросы в MongoDB.")
    parser.add_argument("--uri", default="mongodb://localhost:27017")
    parser.add_argument("--db", default="yas_db")
    parser.add_argument("--creator-id", default="seed-user")
    parser.add_argument(
        "--append",
        action="store_true",
        help="Не удалять предыдущие опросы этого creator-id.",
    )
    args = parser.parse_args()
    seed(args.uri, args.db, args.creator_id, args.append)


if __name__ == "__main__":
    main()
