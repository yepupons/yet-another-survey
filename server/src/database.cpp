#include "database.hpp"
#include <matplot/matplot.h>
#include <openssl/sha.h>
#include <algorithm>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace survey {
std::string Database::read_survey(int survey_id) {
    mongocxx::options::find opts;
    opts.projection(document{} << "sections.questions.answer" << 0 << finalize);
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize, opts
    );
    if (result) {
        return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Survey not found");
}

void Database::write_survey(const std::string &survey_data) {
    bsoncxx::document::value doc = bsoncxx::from_json(survey_data);
    auto insert_result = db()["surveys"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing survey into database failed");
    }

    nlohmann::json json = nlohmann::json::parse(survey_data);
    int survey_id = json["data"]["id"].get<int>();
    std::string creator_id = json["data"]["creator_id"].get<std::string>();

    auto find_result =
        db()["users"].find_one(document{} << "id" << creator_id << finalize);
    if (!find_result) {
        db()["users"].insert_one(
            document{} << "id" << creator_id << "created_surveys" << open_array
                       << close_array << "given_answers" << open_array
                       << close_array << finalize
        );
    }
    db()["users"].update_one(
        document{} << "id" << creator_id << finalize,
        document{} << "$push" << open_document << "created_surveys" << survey_id
                   << close_document << finalize
    );
}

/*
std::string Database::read_answer(int answer_id) {
    auto result = db()["answers"].find_one(
        document{} << "data.id" << answer_id << finalize
    );
    if (result) {
        return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Answer not found");
}
*/

void Database::write_answer(const std::string &answer_data) {
    bsoncxx::document::value doc = bsoncxx::from_json(answer_data);
    auto insert_result = db()["answers"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing answer into database failed");
    }

    nlohmann::json json = nlohmann::json::parse(answer_data);
    int answer_id = json["data"]["id"].get<int>();
    std::string respondent_id =
        json["data"]["respondent_id"].get<std::string>();

    auto find_result =
        db()["users"].find_one(document{} << "id" << respondent_id << finalize);
    if (!find_result) {
        db()["users"].insert_one(
            document{} << "id" << respondent_id << "created_surveys"
                       << open_array << close_array << "given_answers"
                       << open_array << close_array << finalize
        );
    }
    db()["users"].update_one(
        document{} << "id" << respondent_id << finalize,
        document{} << "$push" << open_document << "given_answers" << answer_id
                   << close_document << finalize
    );
}

std::string Database::read_passed_surveys(const std::string &session_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "data.survey_id" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto cursor = db()["answers"].find(
        document{} << "data.respondent_id" << session_id << finalize, opts
    );
    std::set<int> passed_surveys;
    for (auto &&doc : cursor) {
        auto elem = doc["data"]["survey_id"];
        if (!elem) {
            continue;
        }
        if (elem.type() == bsoncxx::type::k_int32) {
            passed_surveys.insert(elem.get_int32().value);
        } else if (elem.type() == bsoncxx::type::k_int64) {
            passed_surveys.insert(static_cast<int>(elem.get_int64().value));
        }
    }
    nlohmann::json result = nlohmann::json::array();
    for (int id : passed_surveys) {
        result.push_back(id);
    }
    return result.dump();
}

std::string Database::read_created_surveys(const std::string &session_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "created_surveys" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto result = db()["users"].find_one(
        document{} << "id" << session_id << finalize, opts
    );
    if (result) {
        return bsoncxx::to_json(
            result->view()["created_surveys"].get_array().value
        );
    }
    throw std::runtime_error("User not found");
}

std::string Database::read_statistics_json(int survey_id) {
    auto survey = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize
    );
    if (!survey) {
        throw std::runtime_error("Survey not found");
    }
    nlohmann::json survey_data =
        nlohmann::json::parse(bsoncxx::to_json(survey->view()));
    nlohmann::json result;
    result["total_answers"] = db()["answers"].count_documents(
        document{} << "data.survey_id" << survey_id << finalize
    );
    result["sections"] = nlohmann::json::array();
    for (int i = 0; i < survey_data["sections"].size(); ++i) {
        result["sections"].push_back(nlohmann::json::array());
        for (int j = 0; j < survey_data["sections"][i]["questions"].size();
             ++j) {
            result["sections"].back().push_back(nlohmann::json::object());
        }
    }

    mongocxx::pipeline p{};
    p.match(document{} << "data.survey_id" << survey_id << finalize);
    p.unwind(
        document{} << "path" << "$sections" << "includeArrayIndex" << "section"
                   << finalize
    );
    p.unwind(
        document{} << "path" << "$sections" << "includeArrayIndex" << "question"
                   << finalize
    );
    p.unwind(document{} << "path" << "$sections.answer" << finalize);
    p.match(
        document{} << "sections.answer" << open_document << "$ne" << ""
                   << close_document << finalize
    );
    p.group(
        document{} << "_id"
                   << (document{} << "section" << "$section" << "question"
                                  << "$question" << "answer"
                                  << "$sections.answer" << finalize)
                   << "count" << (document{} << "$sum" << 1 << finalize)
                   << finalize
    );
    auto cursor = db()["answers"].aggregate(p);

    for (auto &&doc : cursor) {
        auto el = doc["_id"];
        int section = el["section"].get_int64();
        int question = el["question"].get_int64();
        std::string answer;
        switch (el["answer"].type()) {
            case bsoncxx::type::k_string:
                answer = el["answer"].get_string().value.data();
                break;
            case bsoncxx::type::k_int32:
                answer = std::to_string(el["answer"].get_int32().value);
                break;
            default:
                continue;
        }
        auto count = doc["count"];
        result.at("sections")[section][question][answer] =
            count.type() == bsoncxx::type::k_int32
                ? count.get_int32().value
                : static_cast<int>(count.get_int64().value);
    }
    return result.dump();
}

std::string Database::read_statistics_txt(int survey_id) {
    auto survey = nlohmann::json::parse(read_survey(survey_id));
    auto stats = nlohmann::json::parse(read_statistics_json(survey_id));

    std::stringstream file;
    file << "Survey: " << survey["title"] << "\n\n";
    file << "Total number of answers: " << stats["total_answers"] << "\n\n";
    file << "Statistic by sections:\n\n";
    for (int i = 0; i < survey["sections"].size(); ++i) {
        file << "Section " << survey["sections"][i]["title"] << ":\n\n";
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            file << "Question " << question["text"] << ":\n";
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    file << question["options"][k - 1] << ": ";
                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        file << stats["sections"][i][j][std::to_string(k)];
                    } else {
                        file << 0;
                    }
                    file << " answer(s)\n";
                }
            } else if (question["type"] == "text") {
                for (const auto &answer_count :
                     stats["sections"][i][j].items()) {
                    file << '\"' << answer_count.key()
                         << "\": " << answer_count.value() << "answer(s)\n";
                }
            }
            file << '\n';
        }
    }
    return file.str();
}

std::string Database::read_statistics_image(
    int survey_id,
    const std::string &image_format
) {
    using namespace matplot;

    auto survey = nlohmann::json::parse(read_survey(survey_id));
    auto stats = nlohmann::json::parse(read_statistics_json(survey_id));

    int total_rows = 0;
    for (const auto &section : survey["sections"]) {
        total_rows += section["questions"].size();
    }

    auto f = figure(true);
    f->size(1200, 300 * total_rows);

    int plot_index = 1;
    for (int i = 0; i < survey["sections"].size(); ++i) {
        const auto &section = survey["sections"][i];
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            subplot(total_rows, 1, plot_index++);
            title(
                "[Section: " + section["title"].get<std::string>() + "] " +
                "Question: " + question["text"].get<std::string>()
            );

            std::vector<double> values;
            std::vector<std::string> labels;
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    std::string label = question["options"][k - 1];
                    if (label.size() > 20) {
                        label = label.substr(0, 17) + "...";
                    }
                    labels.push_back(label);

                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        values.push_back(
                            stats["sections"][i][j][std::to_string(k)]
                        );
                    } else {
                        values.push_back(0);
                    }
                }
            } else if (question["type"] == "text") {
                if (stats["sections"][i][j].empty()) {
                    labels.push_back("No answers yet");
                    values.push_back(0);
                } else {
                    std::vector<std::pair<std::string, int>> answers;
                    for (const auto &item : stats["sections"][i][j].items()) {
                        answers.emplace_back(item.key(), item.value());
                    }
                    std::sort(
                        answers.begin(), answers.end(),
                        [](auto &a, auto &b) { return a.second > b.second; }
                    );

                    for (int k = 0; k < std::min(10UL, answers.size()); ++k) {
                        std::string label = answers[k].first;
                        if (label.size() > 20) {
                            label = label.substr(0, 17) + "...";
                        }
                        labels.push_back(label);
                        values.push_back(answers[k].second);
                    }
                }
            }
            bar(values);

            xticks(iota(1, labels.size()));
            xticklabels(labels);

            int max_value = *std::max_element(values.begin(), values.end());
            if (max_value < 15) {
                yticks(iota(0, max_value + 1));
            }
            ylim({0, static_cast<double>(max_value + 1)});
        }
    }
    auto filename = std::to_string(survey_id) + '.' + image_format;
    f->save(filename);

    bool file_ready = false;
    for (int attempt = 0; attempt < 100; ++attempt) {
        if (std::filesystem::exists(filename) &&
            std::filesystem::file_size(filename) > 0) {
            file_ready = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    if (!file_ready) {
        throw std::runtime_error("Unable to create file");
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file on server");
    }

    std::string image_data{std::istreambuf_iterator<char>{file}, {}};
    file.close();
    std::filesystem::remove(filename);

    return image_data;
}

std::string
Database::read_survey_results(const std::string &session_id, int survey_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "data.survey_id" << 1 << "sections" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto cursor = db()["answers"].find(
        document{} << "data.respondent_id" << session_id << "data.survey_id"
                   << survey_id << finalize,
        opts
    );
    nlohmann::json results = nlohmann::json::array();
    for (auto &&doc : cursor) {
        if (doc["sections"]) {
            results.push_back(nlohmann::json::parse(
                bsoncxx::to_json(doc["sections"].get_array().value)
            ));
        }
    }
    return results.dump();
}

std::string Database::get_result(const std::string &user_result_data) {
    Database::write_answer(user_result_data);
    nlohmann::json user_json = nlohmann::json::parse(user_result_data);
    int survey_id = user_json.at("data").at("survey_id").get<int>();
    mongocxx::options::find opts;
    opts.projection(
        document{} << "sections.questions.answer" << 1
                   << "sections.questions.type" << 1 << "_id" << 0 << finalize
    );
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize, opts
    );

    nlohmann::json survey_json =
        nlohmann::json::parse(bsoncxx::to_json(result->view()));
    nlohmann::json out;
    out["data"] = {
        {"survey_id", survey_id},
    };
    out["sections"] = nlohmann::json::array();
    int question_amount = 0;
    int correct_answers = 0;
    for (int section_indx = 0; section_indx < survey_json["sections"].size();
         ++section_indx) {
        if (user_json["sections"].at(section_indx).empty()) {
            out["sections"].push_back(nlohmann::json::array());
            continue;
        }
        nlohmann::json section_result = nlohmann::json::array();
        for (int question_indx = 0;
             question_indx <
             survey_json["sections"].at(section_indx)["questions"].size();
             ++question_indx) {
            ++question_amount;
            const auto &question =
                survey_json["sections"].at(section_indx)["questions"].at(
                    question_indx
                );
            std::string type = question.at("type");
            const auto &correct_answer = question.at("answer");
            const auto &user_answer = user_json["sections"]
                                          .at(section_indx)
                                          .at(question_indx)
                                          .at("answer");

            if (type == "single" || type == "multiple") {
                if (correct_answer == user_answer) {
                    ++correct_answers;
                    section_result.emplace_back(1);
                    continue;
                }
                section_result.emplace_back(0);
            } else if (type == "text") {
                if (std::find(
                        correct_answer.begin(), correct_answer.end(),
                        user_answer
                    ) != correct_answer.end()) {
                    ++correct_answers;
                    section_result.emplace_back(1);
                    continue;
                }
                section_result.emplace_back(0);
            }
        }
        out["sections"].push_back(section_result);
    }
    out["data"]["question_amount"] = question_amount;
    out["data"]["correct_answers"] = correct_answers;
    return out.dump();
}

std::string Database::write_image(const drogon::HttpFile &file) {
    auto upload_stream = bucket().open_upload_stream(file.getFileName());
    auto data = file.fileContent();
    upload_stream.write(
        reinterpret_cast<const uint8_t *>(data.data()), data.size()
    );
    auto result = upload_stream.close();
    return result.id().get_oid().value.to_string();
}

std::string Database::read_image(const std::string &image_oid) {
    bsoncxx::types::b_oid oid{bsoncxx::oid(image_oid)};
    auto download_stream =
        bucket().open_download_stream(bsoncxx::types::bson_value::view(oid));

    std::string data;
    data.resize(download_stream.file_length());
    download_stream.read(
        reinterpret_cast<uint8_t *>(&data[0]), download_stream.file_length()
    );

    return data;
}

std::string Database::generate_token() {
    std::random_device rd;
    std::ostringstream oss;
    for (size_t i = 0; i < 16; ++i) {
        unsigned int randomByte = rd() & 0xFF;
        oss << std::hex << std::setw(2) << std::setfill('0') << randomByte;
    }
    return oss.str();
}

std::string Database::generate_uuid() {
    std::random_device rd;
    std::array<unsigned char, 16> bytes{};
    for (auto &byte : bytes) {
        byte = static_cast<unsigned char>(rd() & 0xFF);
    }
    bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3F) | 0x80);
    std::ostringstream oss;
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            oss << "-";
        }
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

std::string Database::sha256(const std::string &input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char *>(input.c_str()), input.size(),
        hash
    );
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

void Database::write_telegram_challenge(
    const std::string &challenge_uuid,
    const std::string &token_hash
) {
    auto now = std::chrono::system_clock::now();
    auto result = db()["telegram_challenges"].insert_one(
        document{} << "_id" << challenge_uuid << "token_hash" << token_hash
                   << "status" << "pending" << "telegram_user"
                   << bsoncxx::types::b_null{} << "created_at"
                   << bsoncxx::types::b_date{now} << "expires_at"
                   << bsoncxx::types::b_date{now + std::chrono::minutes(3)}
                   << "confirmed_at" << bsoncxx::types::b_null{} << "used_at"
                   << bsoncxx::types::b_null{} << finalize
    );
}

void Database::confirm_telegram_challenge(
    const std::string &token_hashed,
    const std::string &telegram_user_data
) {
    nlohmann::json telegram_user = nlohmann::json::parse(telegram_user_data);
    auto now = std::chrono::system_clock::now();
    auto result = db()["telegram_challenges"].update_one(
        document{} << "token_hash" << token_hashed << "status" << "pending"
                   << finalize,
        document{} << "$set" << open_document << "status" << "confirmed"
                   << "telegram_user" << open_document << "id"
                   << telegram_user.at("id").get<std::int64_t>() << "username"
                   << telegram_user.value("username", "") << "first_name"
                   << telegram_user.value("first_name", "") << close_document
                   << "confirmed_at" << bsoncxx::types::b_date{now}
                   << close_document << finalize
    );

    if (!result || result->modified_count() != 1) {
        throw std::runtime_error("Challenge was not updated");
    }
}

bool Database::bot_check_login_data(const std::string &user_login_data) {
    nlohmann::json login_data = nlohmann::json::parse(user_login_data);
    std::string token_sha256 =
        sha256(login_data.at("token").get<std::string>());
    auto result = db()["telegram_challenges"].find_one(
        document{} << "token_hash" << token_sha256 << finalize
    );
    if (!result) {
        throw std::invalid_argument("Challenge not found");
    }
    auto view = result->view();
    std::string status(view["status"].get_string().value);
    if (status != "pending") {
        throw std::invalid_argument("Challenge was already closed");
    }
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    auto expires_at = view["expires_at"].get_date().value;

    if (now_ms > expires_at) {
        throw std::invalid_argument("Challenge expired");
    }
    confirm_telegram_challenge(
        token_sha256, login_data.at("telegram_user").dump()
    );
    return true;
}

std::string Database::get_challenge_status(const std::string &challenge_id) {
    auto result = db()["telegram_challenges"].find_one(
        document{} << "_id" << challenge_id << finalize
    );
    if (!result) {
        throw std::invalid_argument("Challenge not found");
    }
    auto view = result->view();
    return std::string(view["status"].get_string().value);
}

std::string Database::complete_login(const std::string &user_challenge_data) {
    const nlohmann::json challenge_data =
        nlohmann::json::parse(user_challenge_data);
    const std::string challenge_id =
        challenge_data.at("challenge_id").get<std::string>();

    auto result = db()["telegram_challenges"].find_one(
        document{} << "_id" << challenge_id << finalize
    );
    if (!result) {
        throw std::invalid_argument("Challenge not found");
    }

    auto view = result->view();
    const std::string status(view["status"].get_string().value);
    if (status != "confirmed") {
        throw std::invalid_argument("Challenge is not confirmed");
    }

    auto used_at = view["used_at"];
    if (!used_at || used_at.type() != bsoncxx::type::k_null) {
        throw std::invalid_argument("Challenge was already used");
    }

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    );
    auto expires_at = view["expires_at"].get_date().value;
    if (expires_at <= now_ms) {
        throw std::invalid_argument("Challenge expired");
    }

    auto telegram_user = view["telegram_user"].get_document().view();
    const std::int64_t telegram_user_id = telegram_user["id"].get_int64().value;
    const std::string telegram_username =
        std::string(telegram_user["username"].get_string().value);
    const std::string telegram_first_name =
        std::string(telegram_user["first_name"].get_string().value);
    const std::string access_token = "access_" + generate_token();
    auto user_result = db()["users"].find_one(
        document{} << "telegram_user_id" << telegram_user_id << finalize
    );

    std::string user_id;
    if (user_result) {
        user_id = std::string(user_result->view()["id"].get_string().value);
        auto update_user_result = db()["users"].update_one(
            document{} << "telegram_user_id" << telegram_user_id << finalize,
            document{} << "$set" << open_document << "telegram_username"
                       << telegram_username << "telegram_first_name"
                       << telegram_first_name << "updated_at"
                       << bsoncxx::types::b_date{now} << "access_token"
                       << access_token << close_document << finalize
        );
    } else {
        user_id = generate_uuid();
        auto insert_user_result = db()["users"].insert_one(
            document{} << "id" << user_id << "telegram_user_id"
                       << telegram_user_id << "telegram_username"
                       << telegram_username << "telegram_first_name"
                       << telegram_first_name << "created_at"
                       << bsoncxx::types::b_date{now} << "updated_at"
                       << bsoncxx::types::b_date{now} << "access_token"
                       << access_token << "created_surveys" << open_array
                       << close_array << "given_answers" << open_array
                       << close_array << finalize
        );
    }

    auto update_challenge_result = db()["telegram_challenges"].update_one(
        document{} << "_id" << challenge_id << "status" << "confirmed"
                   << "used_at" << bsoncxx::types::b_null{} << finalize,
        document{} << "$set" << open_document << "status" << "used" << "used_at"
                   << bsoncxx::types::b_date{now} << close_document << finalize
    );
    if (!update_challenge_result ||
        update_challenge_result->modified_count() != 1) {
        throw std::runtime_error("Challenge was not completed");
    }

    nlohmann::json user_data;
    user_data["access_token"] = access_token;
    user_data["user"]["id"] = user_id;
    user_data["user"]["telegram_user_id"] = telegram_user_id;
    user_data["user"]["telegram_username"] = telegram_username;
    user_data["user"]["telegram_first_name"] = telegram_first_name;
    return user_data.dump();
}

std::string Database::user_id_by_access_token(const std::string &access_token) {
    auto result = db()["users"].find_one(
        document{} << "access_token" << access_token << finalize
    );
    if (!result) {
        throw std::invalid_argument("Invalid access token");
    }
    return std::string(result->view()["id"].get_string().value);
}

}  // namespace survey
