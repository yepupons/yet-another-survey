#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <drogon/MultiPart.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <string>

namespace survey {
class Database {
    mongocxx::instance instance_;
    mongocxx::client client_;

    mongocxx::database db() {
        return client_["yas_db"];
    }

    mongocxx::gridfs::bucket bucket() {
        return client_["yas_db"].gridfs_bucket();
    }

public:
    Database() : instance_{}, client_{mongocxx::uri{}} {
    }

    void link_telegram(const std::string &session_id, std::int64_t telegram_id);
    void unlink_telegram(const std::string &session_id);

    std::string read_account(const std::string &session_id);
    std::string read_survey(int survey_id);
    void write_survey(const std::string &survey_data);

    // std::string read_answer(int answer_id);
    void write_answer(const std::string &answer_data);

    std::string read_passed_surveys(const std::string &session_id);
    std::string read_created_surveys(const std::string &session_id);

    std::string read_statistics(int survey_id);
    std::string
    read_survey_results(const std::string &session_id, int survey_id);
    std::string get_result(const std::string &user_result_data);

    std::string write_image(const drogon::HttpFile &file);
    std::string read_image(const std::string &image_oid);
    void write_telegram_challenge(
        const std::string &challenge_uuid,
        const std::string &token_hash
    );

    static std::string generate_token();
    static std::string generate_uuid();
    static std::string sha256(const std::string &input);

    bool bot_check_login_data(const std::string &user_login_data);
    void confirm_telegram_challenge(
        const std::string &token_hashed,
        const std::string &telegram_user_data
    );
    std::string get_challenge_status(const std::string &challenge_id);
    std::string complete_login(const std::string &user_challenge_data);
    std::string user_id_by_access_token(const std::string &access_token);
};
}  // namespace survey

#endif
