#ifndef OLLAMA_INTERATION_HPP_
#define OLLAMA_INTERATION_HPP_

#include "nlohmann/json_fwd.hpp"

nlohmann::json send_generate_request(const std::string &user_message);

#endif // OLLAMA_INTERATION_HPP_