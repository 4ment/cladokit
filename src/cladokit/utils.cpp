// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/utils.hpp"

#include <algorithm>
#include <any>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace cladokit {
std::vector<std::string> SplitTopLevel(const std::string &str) {
    std::vector<std::string> tokens;
    std::string token;
    int brace_depth = 0;

    for (char ch : str) {
        if (ch == ',' && brace_depth == 0) {
            tokens.push_back(token);
            token.clear();
        } else {
            if (ch == '{') brace_depth++;
            if (ch == '}') brace_depth--;
            token += ch;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

void ParseComment(const std::string &comment,
                  std::map<std::string, std::any> &annotations,
                  const std::unordered_map<std::string, Converter> &converters) {
    size_t start = comment.find("&") + 1;
    size_t end = comment.find_last_of("]");
    std::string content = comment.substr(start, end - start);

    auto tokens = SplitTopLevel(content);

    for (const auto &token : tokens) {
        size_t eq_pos = token.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = token.substr(0, eq_pos);
            std::string value = token.substr(eq_pos + 1);

            auto it = converters.find(key);
            if (it != converters.end()) {
                annotations[key] = it->second(value);
            } else {
                // Store as string if key not in converters
                annotations[key] = value;
            }
        }
    }
}
}  // namespace cladokit
