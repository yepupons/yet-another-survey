#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <cstdlib>
#include "database.hpp"

using namespace drogon;

// TODO: maybe not throw raw error to client, but just write it in debug mode +
// log?

static std::string bearer_token(const HttpRequestPtr &request) {
    std::string auth = request->getHeader("authorization");
    if (auth.empty()) {
        auth = request->getHeader("Authorization");
    }

    const std::string prefix = "Bearer ";
    if (auth.rfind(prefix, 0) != 0) {
        throw std::invalid_argument("Missing access token");
    }
    return auth.substr(prefix.size());
}

static void require_bot_secret(const HttpRequestPtr &request) {
    const char *expected = std::getenv("BOT_CONFIRM_SECRET");
    if (!expected || std::string(expected).empty()) {
        throw std::runtime_error("Bot secret is not configured");
    }

    const std::string provided = request->getHeader("X-Bot-Secret");
    if (provided != expected) {
        throw std::invalid_argument("Forbidden");
    }
}

int main(int argc, char *argv[]) {
    survey::Database db;
    app().addListener("127.0.0.1", 8080);

    app().registerPreRoutingAdvice([](const drogon::HttpRequestPtr &req,
                                      drogon::AdviceCallback &&acb,
                                      drogon::AdviceChainCallback &&accb) {
        if (req->method() == drogon::Options) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->addHeader(
                "Access-Control-Allow-Origin", "http://localhost:8054"
            );
            resp->addHeader(
                "Access-Control-Allow-Methods", "GET, POST, OPTIONS"
            );
            resp->addHeader(
                "Access-Control-Allow-Headers",
                "mime-version, Content-Type, Authorization"
            );
            resp->addHeader("Access-Control-Allow-Credentials", "true");

            acb(resp);
            return;
        }
        accb();
    });

    app().registerPostHandlingAdvice([](const drogon::HttpRequestPtr &req,
                                        const drogon::HttpResponsePtr &resp) {
        resp->addHeader("Access-Control-Allow-Origin", "http://localhost:8054");
        resp->addHeader("Access-Control-Allow-Credentials", "true");
    });

    app().registerHandler(
        "/survey",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string survey_id = request->getParameter("id");
                const auto survey_data = db.read_survey(survey_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(survey_data);
                cb(resp);
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error reading survey "
                          << request->getParameter("id") << ": " << e.what()
                          << std::endl;
#endif
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/survey",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            std::string out;
            try {
                auto survey_data =
                    nlohmann::json::parse(std::string(request->getBody()));
                const std::string creator_id =
                    db.user_id_by_access_token(bearer_token(request));
                const std::string survey_id = survey::Database::generate_uuid();
                out = db.write_survey(survey_id, creator_id, survey_data.dump());
#ifdef YAZ_DEBUG
                std::cerr << "Received survey: " << survey_data.dump(2)
                          << std::endl;
#endif
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error saving survey "
                          << request->getParameter("id") << ": " << e.what()
                          << std::endl;
#endif
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setContentTypeCode(CT_APPLICATION_JSON);
            resp->setBody(out);
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/answer",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            std::string out;
            try {
                auto answer_data =
                    nlohmann::json::parse(std::string(request->getBody()));
                const std::string respondent_id =
                    db.user_id_by_access_token(bearer_token(request));
                const std::string answer_id = survey::Database::generate_uuid();
                out = db.write_answer(
                    answer_id, respondent_id, answer_data.dump()
                );
#ifdef YAZ_DEBUG
                std::cerr << "Received answer: " << answer_data.dump(2)
                          << std::endl;
#endif
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error saving answer: " << e.what() << std::endl;
#endif
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setContentTypeCode(CT_APPLICATION_JSON);
            resp->setBody(out);
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/passed-surveys",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string session_id =
                    db.user_id_by_access_token(bearer_token(request));
                const auto passed_surveys_data =
                    db.read_passed_surveys(session_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(passed_surveys_data);
                cb(resp);
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error reading passed surveys for "
                          << request->getParameter("session-id") << ": "
                          << e.what() << std::endl;
#endif
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/survey-results",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string session_id =
                    db.user_id_by_access_token(bearer_token(request));
                std::string survey_id = request->getParameter("survey-id");
                const auto survey_results_data =
                    db.read_survey_results(session_id, survey_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(survey_results_data);
                cb(resp);
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error reading survey results for "
                          << request->getParameter("session-id")
                          << " and survey "
                          << request->getParameter("survey-id") << ": "
                          << e.what() << std::endl;
#endif
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/created-surveys",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string session_id =
                    db.user_id_by_access_token(bearer_token(request));
                const auto created_surveys_data =
                    db.read_created_surveys(session_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(created_surveys_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/statistics",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string survey_id = request->getParameter("survey-id");
                std::string format = request->getParameter("format");
                std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) {
                    return std::tolower(c);
                });
                const std::string requester_id = db.user_id_by_access_token(bearer_token(request));
                if (!db.is_survey_creator(survey_id, requester_id)) {
                    throw std::invalid_argument("Forbidden");
                }

                auto resp = HttpResponse::newHttpResponse();
                std::string survey_statistics_data;
                if (format == "json") {
                    survey_statistics_data = db.read_statistics_json(survey_id);
                    resp->setContentTypeCode(CT_APPLICATION_JSON);
                } else if (format == "txt") {
                    survey_statistics_data = db.read_statistics_txt(survey_id);
                    resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
                } else if (format == "jpg" || format == "jpeg" || format == "png") {
                    survey_statistics_data = db.read_statistics_image(survey_id, format);
                    resp->setContentTypeString("image/" + format);
                } else {
                    throw std::runtime_error("Bad format");
                }
                resp->setBody(survey_statistics_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/check",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            std::string out;
            try {
                auto user_answers = request->getJsonObject();
                const std::string respondent_id =
                    db.user_id_by_access_token(bearer_token(request));
                const std::string answer_id = survey::Database::generate_uuid();
                out = db.get_result(
                    answer_id, respondent_id, user_answers->toStyledString()
                );
            } catch (const std::exception &e) {
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setContentTypeCode(CT_APPLICATION_JSON);
            resp->setBody(out);
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/image",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            MultiPartParser file_upload;
            if (file_upload.parse(request) != 0 ||
                file_upload.getFiles().empty()) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody("No file uploaded or invalid form data");
                cb(resp);
                return;
            }
            const auto &file = file_upload.getFiles()[0];
            try {
                resp->setBody(db.write_image(file));
                cb(resp);
            } catch (const std::exception &e) {
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/image",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string image_oid = request->getParameter("id");
                const auto image_data = db.read_image(image_oid);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_IMAGE_JPG);
                resp->setBody(image_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/api/auth/telegram/challenge",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            std::string challenge_uuid = survey::Database::generate_uuid();
            std::string token = "login_" + survey::Database::generate_token();
            std::string hashed_token = survey::Database::sha256(token);

            Json::Value result;
            db.write_telegram_challenge(challenge_uuid, hashed_token);
            result["challenge_id"] = challenge_uuid;
            result["telegram_url"] =
                "https://t.me/yet_another_survey_bot?start=" + token;
            result["expires_in"] = 300;
            auto resp = HttpResponse::newHttpJsonResponse(result);
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/api/auth/telegram/challenge/{1}",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb,
            const std::string &challenge_id
        ) {
            try {
                std::string status = db.get_challenge_status(challenge_id);
                Json::Value result;
                result["status"] = status;
                auto resp = HttpResponse::newHttpJsonResponse(result);
                cb(resp);
            } catch (const std::exception &e) {
                Json::Value result;
                result["status"] = "error";
                result["error"] = e.what();
                auto resp = HttpResponse::newHttpJsonResponse(result);
                resp->setStatusCode(k404NotFound);
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/api/auth/telegram/confirm",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                require_bot_secret(request);
                auto resp = HttpResponse::newHttpResponse();
                auto login_data = request->getJsonObject()->toStyledString();
                auto login_status = db.bot_check_login_data(login_data);
                if (login_status == 1) {
                    resp->setBody("{\"success\" : true}");
                } else {
                    resp->setBody("{\"success\" : false}");
                }
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                cb(resp);
            } catch (const std::exception &e) {
                Json::Value result;
                result["success"] = false;
                result["error"] = e.what();

                auto resp = HttpResponse::newHttpJsonResponse(result);
                resp->setStatusCode(k403Forbidden);
                cb(resp);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/api/auth/telegram/complete",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                auto login_data = request->getJsonObject()->toStyledString();
                auto user_data = db.complete_login(login_data);
                auto resp = HttpResponse::newHttpResponse();
                resp->setBody(user_data);
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                cb(resp);
            } catch (const std::exception &e) {
                Json::Value result;
                result["error"] = e.what();
                auto resp = HttpResponse::newHttpJsonResponse(result);
                resp->setStatusCode(k400BadRequest);
                cb(resp);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/api/surveys/{1}/ratings",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb,
            const std::string &survey_id
        ) {
            try {
                auto rating_data = request->getJsonObject()->toStyledString();
                std::string user_id = db.user_id_by_access_token(bearer_token(request));
                std::string result = db.save_rate(rating_data, survey_id, user_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setBody(result);
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                cb(resp);
            } catch (const std::exception &e) {
                Json::Value result;
                result["error"] = e.what();
                auto resp = HttpResponse::newHttpJsonResponse(result);
                resp->setStatusCode(k400BadRequest);
                cb(resp);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/api/surveys/top",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                std::string result = db.get_top_surveys();
                auto resp = HttpResponse::newHttpResponse();
                resp->setBody(result);
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                cb(resp);
            } catch (const std::exception &e) {
                Json::Value result;
                result["error"] = e.what();
                auto resp = HttpResponse::newHttpJsonResponse(result);
                resp->setStatusCode(k400BadRequest);
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/api/auth/logout",
        [&db](
            const HttpRequestPtr &request, 
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                db.revoke_access_token(bearer_token(request));
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k204NoContent);
                cb(resp);
            } catch (...) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k401Unauthorized);
                cb(resp);
            }
        },
        {Post}
    );

    app().run();
    return 0;
}
