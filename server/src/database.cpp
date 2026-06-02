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
std::string Database::read_survey(const std::string &survey_id) {
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

std::string Database::read_survey_with_answers(const std::string &survey_id) {
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize
    );
    if (result) {
        return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Survey not found");
}

std::string Database::write_survey(
    const std::string &survey_id,
    const std::string &creator_id,
    const nlohmann::json &survey_data
) {
    bsoncxx::document::value doc = bsoncxx::from_json(survey_data.dump());
    auto insert_result = db()["surveys"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing survey into database failed");
    }
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
    nlohmann::json result;
    result["status"] = "Saved";
    result["survey_id"] = survey_id;
    return result.dump();
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

std::string Database::write_answer(
    const std::string &answer_id,
    const std::string &respondent_id,
    const std::string &answer_data
) {
    nlohmann::json json = nlohmann::json::parse(answer_data);
    json["data"]["id"] = answer_id;
    json["data"]["respondent_id"] = respondent_id;

    bsoncxx::document::value doc = bsoncxx::from_json(json.dump());
    auto insert_result = db()["answers"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing answer into database failed");
    }

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

    nlohmann::json result;
    result["status"] = "Saved";
    result["answer_id"] = answer_id;
    result["survey_id"] = json["data"]["survey_id"];
    return result.dump();
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
    std::set<std::string> passed_surveys;
    for (auto &&doc : cursor) {
        auto elem = doc["data"]["survey_id"];
        if (!elem) {
            continue;
        }
        if (elem.type() != bsoncxx::type::k_string) {
            continue;
        }
        passed_surveys.insert(std::string(elem.get_string().value));
    }
    nlohmann::json result = nlohmann::json::array();
    for (const auto &id : passed_surveys) {
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

bool Database::is_survey_creator(
    const std::string &survey_id,
    const std::string &user_id
) {
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << "data.creator_id" << user_id
                   << finalize
    );
    return static_cast<bool>(result);
}

std::string Database::read_statistics_json(const std::string &survey_id) {
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

std::string Database::read_statistics_txt(const std::string &survey_id) {
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

std::string Database::read_statistics_image(const std::string &survey_id, const std::string &image_format) {
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
    auto filename = survey_id + '.' + image_format;
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
Database::read_survey_results(const std::string &session_id, const std::string &survey_id) {
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
    const std::string access_token_hash = sha256(access_token);
    const auto access_token_expires_at = now + std::chrono::hours(12);
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
                       << bsoncxx::types::b_date{now} << "access_token_hash"
                       << access_token_hash << "access_token_expires_at"
                       << bsoncxx::types::b_date{access_token_expires_at}
                       << close_document << finalize
        );
    } else {
        user_id = generate_uuid();
        auto insert_user_result = db()["users"].insert_one(
            document{} << "id" << user_id << "telegram_user_id"
                       << telegram_user_id << "telegram_username"
                       << telegram_username << "telegram_first_name"
                       << telegram_first_name << "created_at"
                       << bsoncxx::types::b_date{now} << "updated_at"
                       << bsoncxx::types::b_date{now} << "access_token_hash"
                       << access_token_hash << "access_token_expires_at"
                       << bsoncxx::types::b_date{access_token_expires_at}
                       << "created_surveys" << open_array
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
    const std::string token_hash = sha256(access_token);
    auto result = db()["users"].find_one(
        document{} << "access_token_hash" << token_hash << finalize
    );
    if (!result) {
        throw std::invalid_argument("Invalid access token");
    }
    auto view = result->view();

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    if (view["access_token_expires_at"].get_date().value <= now_ms) {
        throw std::invalid_argument("Access token expired");
    }
    return std::string(view["id"].get_string().value);
}

std::string Database::save_rate(
    const std::string &rate_data,
    const std::string &survey_id,
    const std::string &user_id
) {
    nlohmann::json json = nlohmann::json::parse(rate_data);
    const std::string answer_id = json["answer_id"].get<std::string>();
    const std::string rate = json["rate"].get<std::string>();

    int rate_value = 0;
    if (rate == "like") {
        rate_value = 1;
    } else if (rate == "dislike") {
        rate_value = -1;
    } else {
        throw std::invalid_argument("Invalid rate");
    }

    auto survey_existence = db()["surveys"].find_one(
        document{} << "data.id" << survey_id
                   << finalize
    );
    if (!survey_existence){
        throw std::invalid_argument("Survey not found");
    }

    auto answer_result = db()["answers"].find_one(
        document{} << "data.id" << answer_id
                   << "data.survey_id" << survey_id
                   << "data.respondent_id" << user_id
                   << finalize
    );
    if (!answer_result) {
        throw std::invalid_argument("Answer not found");
    }

    auto existing_rate_result = db()["survey_rates"].find_one(
        document{} << "survey_id" << survey_id
                   << "answer_id" << answer_id
                   << "user_id" << user_id
                   << finalize
    );
    if (existing_rate_result) {
        throw std::invalid_argument("Survey already rated");
    }

    const std::string rate_id = generate_uuid();
    auto insert_rate_result = db()["survey_rates"].insert_one(
        document{} << "id" << rate_id
                   << "survey_id" << survey_id
                   << "answer_id" << answer_id
                   << "user_id" << user_id
                   << "value" << rate_value
                   << finalize
    );

    const std::string rate_counter_field =
        rate_value == 1 ? "data.likes_count" : "data.dislikes_count";

    auto update_survey_result = db()["surveys"].update_one(
        document{} << "data.id" << survey_id << finalize,
        document{} << "$inc" << open_document
                    << rate_counter_field << 1
                    << "data.ratings_count" << 1
                    << "data.rating_score" << rate_value
                   << close_document << finalize
    );


    nlohmann::json result;
    result["status"] = "Saved";
    result["rate_id"] = rate_id;
    result["survey_id"] = survey_id;
    result["answer_id"] = answer_id;
    result["rating_score_delta"] = rate_value;
    return result.dump();
}

std::string Database::get_top_surveys() {
    mongocxx::options::find opts;
    opts.sort(
        document{} << "data.rating_score" << -1
                   << "data.ratings_count" << -1
                   << finalize
    );
    opts.limit(10);

    auto cursor = db()["surveys"].find(
        document{} << finalize,
        opts
    );

    auto str_or = [](bsoncxx::document::view doc, const char *key, const char *def = "") -> std::string {
        auto el = doc[key];
        return (el && el.type() == bsoncxx::type::k_string)
            ? std::string(el.get_string().value) : def;
    };
    auto int_or = [](bsoncxx::document::view doc, const char *key, int def = 0) -> int {
        auto el = doc[key];
        return (el && el.type() == bsoncxx::type::k_int32)
            ? el.get_int32().value : def;
    };

    nlohmann::json result = nlohmann::json::array();
    for (auto &&survey : cursor) {
        const auto data = survey["data"].get_document().value;
        nlohmann::json survey_json;
        survey_json["id"] = str_or(data, "id");
        survey_json["title"] = str_or(survey, "title");
        survey_json["description"] = str_or(survey, "description");
        survey_json["likes_count"] = int_or(data, "likes_count");
        survey_json["dislikes_count"] = int_or(data, "dislikes_count");
        survey_json["ratings_count"] = int_or(data, "ratings_count");
        survey_json["rating_score"] = int_or(data, "rating_score");
        result.push_back(survey_json);
    }
    return result.dump();
}

void Database::revoke_access_token(const std::string &access_token) {
    const std::string token_hash = sha256(access_token);
    db()["users"].update_one(
        document{} << "access_token_hash" << token_hash << finalize,
        document{} << "$unset" << open_document
                   << "access_token_hash" << ""
                   << "access_token_expires_at" << ""
                   << close_document << finalize
    );
}
}  // namespace survey
