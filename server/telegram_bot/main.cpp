#include <curl/curl.h>
#include <tgbot/tgbot.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

static size_t write_cb(char *ptr, size_t size, size_t nmemb, std::string *out) {
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string http_get(const std::string &url, const std::string &body) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return "";
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return body;
}

std::string http_post(const std::string &url, const std::string &body) {
    std::string response;
    CURL *curl = curl_easy_init();
    if (!curl) {
        return "";
    }
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

void http_delete(const std::string &url) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return;
    }
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
                "Привет! Я — бот сервиса ЯЗЬ (yet-another-survey). Чтобы войти "
                "в сервис под своим аккаунтом, откройте приложение и выберите "
                "Login with telegram.\n"
                "Чтобы узнать о всех командах, пропишите /help."
            );
            return;
        }

        const std::int64_t telegram_id = message->from->id;
        const std::string name = message->from->username;
        const std::string username = name.empty() ? "(не задан)" : "@" + name;
        const std::string first_name = message->from->firstName;

        const std::string link_url =
            "http://127.0.0.1:8080/api/auth/telegram/confirm";
        nlohmann::json auth_data = {};
        auth_data["token"] = login_token;
        auth_data["telegram_user"]["id"] = telegram_id;
        auth_data["telegram_user"]["username"] = username;
        auth_data["telegram_user"]["first_name"] = first_name;
        const std::string response = http_post(link_url, auth_data.dump());

        if (response.empty()) {
            bot.getApi().sendMessage(
                message->chat->id, "Server did not return a response."
            );
            return;
        }
        nlohmann::json login_response = nlohmann::json::parse(response);
        std::string answer;
        if (login_response.value("success", false)) {
            answer = username + ", вход подтвержден. Вернитесь в приложение.";
        } else {
            answer = "Ошибка входа: " +
                     login_response.value("error", "unknown error");
        }
        bot.getApi().sendMessage(message->chat->id, answer);
    });
    /*
        bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) {
            bot.getApi().sendMessage(
                message->chat->id,
                "commands:\n"
                "/start - start bot\n"
                "/account - account you logged in with\n"
                "/unlink - unlink Telegram from your account\n"
            );
        });

        bot.getEvents().onCommand("account", [&bot,
       &sessions](TgBot::Message::Ptr message) { const std::int64_t telegram_id
       = message->from->id; auto it = sessions.find(telegram_id); if (it ==
       sessions.end()) { bot.getApi().sendMessage(message->chat->id, "You are
       not logged in. Use /start to authorize."); return;
            }

            const std::string url =
                "http://localhost:8080/account?session-id=" + it->second;
            const std::string response = http_get(url);

            if (response.empty()) {
                bot.getApi().sendMessage(message->chat->id, "Server error.");
                return;
            }

            bot.getApi().sendMessage(message->chat->id, "Your account data:\n" +
       response);
        });

        bot.getEvents().onCommand("unlink", [&bot,
       &sessions](TgBot::Message::Ptr message) { const std::int64_t telegram_id
       = message->from->id; auto it = sessions.find(telegram_id); if (it ==
       sessions.end()) { bot.getApi().sendMessage(message->chat->id, "You are
       not logged in. Use /start to authorize."); return;
            }

            std::string url =
       "http://localhost:8080/account/telegram?session-id=" + it->second;
            http_delete(url);
            sessions.erase(it);

            bot.getApi().sendMessage(message->chat->id, "Telegram unlinked from
       your account.");
        });

        bot.getEvents().onNonCommandMessage([&bot](TgBot::Message::Ptr msg) {
            bot.getApi().sendMessage(msg->chat->id, "To authorize, open the
       login through the application.");
        });
    */
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
