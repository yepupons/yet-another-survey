#include <tgbot/tgbot.h>

#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main() {
    const char *token_env = std::getenv("BOT_TOKEN");

    if (token_env == nullptr) {
        std::cerr << "BOT_TOKEN is not set\n";
        return 1;
    }

    TgBot::Bot bot(token_env);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        const std::string text = message->text;
        std::string login_token;

        const std::string prefix = "/start ";

        if (text.rfind(prefix, 0) == 0) {
            login_token = text.substr(prefix.size());
        }

        if (login_token.empty()) {
            bot.getApi().sendMessage(
                message->chat->id,
                "Привет! Для авторизации открой вход через приложение."
            );
            return;
        }

        const std::int64_t telegram_id = message->from->id;
        const std::string username = message->from->username;
        const std::string first_name = message->from->firstName;

        std::string answer =
            "Авторизация получена.\n\n"
            "Login token: " + login_token + "\n"
            "Telegram ID: " + std::to_string(telegram_id) + "\n"
            "Username: @" + username + "\n"
            "First name: " + first_name + "\n\n"
            "Позже эти данные будут отправляться на сервер приложения.";

        bot.getApi().sendMessage(message->chat->id, answer);
    });

    bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(
            message->chat->id,
            "Нажми кнопку входа в приложении, чтобы авторизоваться через Telegram."
        );
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        if (
            message->text == "/help" ||
            message->text == "/start" ||
            message->text.rfind("/start ", 0) == 0
        ) {
            return;
        }

        bot.getApi().sendMessage(
            message->chat->id,
            "Для входа нажми кнопку Log in with Telegram в приложении."
        );
    });

    try {
        std::cout << "Bot username: " << bot.getApi().getMe()->username << '\n';

        TgBot::TgLongPoll long_poll(bot);

        while (true) {
            long_poll.start();
        }
    } catch (const std::exception &error) {
        std::cerr << "Bot error: " << error.what() << '\n';
    }

    return 0;
}
