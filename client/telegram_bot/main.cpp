#include <tgbot/tgbot.h>
#include <curl/curl.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <thread>

static size_t write_cb(char *ptr, size_t size, size_t nmemb, std::string *out) {
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string http_get(const std::string &url) {
    std::string body;
    CURL *curl = curl_easy_init();
    if (!curl) return "";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return body;
}

void http_post(const std::string &url, const std::string &body) {
    CURL *curl = curl_easy_init();
    if (!curl) return;
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void http_delete(const std::string &url) {
    CURL *curl = curl_easy_init();
    if (!curl) return;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

int main() {
    const char *token_env = std::getenv("BOT_TOKEN");

    if (token_env == nullptr) {
        std::cerr << "BOT_TOKEN is not set\n";
        return 1;
    }

    TgBot::Bot bot(token_env);
    std::map<std::int64_t, std::string> sessions; // telegram_id → login_token

    bot.getEvents().onCommand("start", [&bot, &sessions](TgBot::Message::Ptr message) {
        const std::string text = message->text;
        std::string login_token;
        const std::string prefix = "/start ";
        if (text.rfind(prefix, 0) == 0) {
            login_token = text.substr(prefix.size());
        }

        if (login_token.empty()) {
            bot.getApi().sendMessage(
                message->chat->id,
                "Hello! To authorize, open the login through the application.\n"
                "For more info use /help."
            );
            return;
        }

        const std::int64_t telegram_id = message->from->id;
        const std::string name = message->from->username;
        const std::string username = name.empty() ? "(не задан)" : "@" + name;
        const std::string first_name = message->from->firstName;

        sessions[telegram_id] = login_token;

        const std::string link_url =
            "http://localhost:8080/account/telegram?session-id=" + login_token;
        http_post(link_url, "{\"telegram_id\": " + std::to_string(telegram_id) + "}");

        std::string answer =
            "Authorization granted.\n\n"
            "Login token: " + login_token + "\n"
            "Telegram ID: " + std::to_string(telegram_id) + "\n"
            "Username: " + username + "\n"
            "First name: " + first_name + "\n";
        bot.getApi().sendMessage(message->chat->id, answer);
    });

    bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(
            message->chat->id,
            "commands:\n"
            "/start - start bot\n"
            "/account - account you logged in with\n"
            "/unlink - unlink Telegram from your account\n"
        );
    });

    bot.getEvents().onCommand("account", [&bot, &sessions](TgBot::Message::Ptr message) {
        const std::int64_t telegram_id = message->from->id;
        auto it = sessions.find(telegram_id);
        if (it == sessions.end()) {
            bot.getApi().sendMessage(message->chat->id, "You are not logged in. Use /start to authorize.");
            return;
        }

        const std::string url =
            "http://localhost:8080/account?session-id=" + it->second;
        const std::string response = http_get(url);

        if (response.empty()) {
            bot.getApi().sendMessage(message->chat->id, "Server error.");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, "Your account data:\n" + response);
    });

    bot.getEvents().onCommand("unlink", [&bot, &sessions](TgBot::Message::Ptr message) {
        const std::int64_t telegram_id = message->from->id;
        auto it = sessions.find(telegram_id);
        if (it == sessions.end()) {
            bot.getApi().sendMessage(message->chat->id, "You are not logged in. Use /start to authorize.");
            return;
        }

        std::string url = "http://localhost:8080/account/telegram?session-id=" + it->second;
        http_delete(url);
        sessions.erase(it);

        bot.getApi().sendMessage(message->chat->id, "Telegram unlinked from your account.");
    });

    bot.getEvents().onNonCommandMessage([&bot](TgBot::Message::Ptr msg) {
        bot.getApi().sendMessage(msg->chat->id, "To authorize, open the login through the application.");
    });

    TgBot::TgLongPoll long_poll(bot);
    while (true) {
        try {
            long_poll.start();
        } catch (const std::exception &e) {
            std::cerr << "Poll error: " << e.what() << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    return 0;
}
