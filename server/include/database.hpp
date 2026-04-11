#ifndef DATABASE_HPP_
#define DATABASE_HPP_

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

public:
    Database() : instance_{}, client_{mongocxx::uri{}} {
    }

    std::string read_survey(int survey_id);
    void write_survey(const std::string &survey_data);

    // std::string read_answer(int answer_id);
    void write_answer(const std::string &answer_data);

    std::string read_passed_surveys(int session_id);
    std::string read_survey_results(int session_id, int survey_id);
    std::string get_result(const std::string &user_result_data);
};
}  // namespace survey

#endif
