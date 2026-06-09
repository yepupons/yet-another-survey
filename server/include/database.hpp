#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <drogon/MultiPart.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <nlohmann/json.hpp>
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
    std::string read_survey(const std::string &survey_id);
    std::string read_survey_with_answers(const std::string &survey_id);
    std::string write_survey(const nlohmann::json &survey_data);
    std::string write_answer(const nlohmann::json &answer_data);

    std::string read_passed_surveys(const std::string &session_id);
    std::string read_created_surveys(const std::string &session_id);
    bool is_survey_creator(const std::string &survey_id, const std::string &user_id);

    std::string read_statistics_json(const std::string &survey_id);
    std::string read_statistics_txt(const std::string &survey_id);
    std::string read_statistics_image(const std::string &survey_id, const std::string &image_format);

    std::string read_survey_results(const std::string &answer_id);

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
    std::string save_rate(const std::string &rate_data, const std::string &survey_id, const std::string &user_id);
    std::string get_top_surveys(const std::string &session_id = "");
    void revoke_access_token(const std::string &access_token);

    std::string read_global_stats();
};
}  // namespace survey

#endif
