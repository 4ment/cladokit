// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <any>
#include <cctype>
#include <functional>
#include <locale>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "cladokit/newick_options.hpp"

namespace cladokit {
// trim from start (in place)
inline void LeftTrim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }));
}

// trim from end (in place)
inline void RightTrim(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           [](unsigned char ch) { return !std::isspace(ch); })
                  .base(),
              str.end());
}

inline bool StartsWithCaseInsensitive(const std::string& target,
                                      const std::string& input_prefix) {
    if (target.size() < input_prefix.size()) return false;
    return std::equal(
        input_prefix.begin(), input_prefix.end(), target.begin(),
        [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

inline bool StartsWithCaseInsensitiveLeftTrim(const std::string& target,
                                              const std::string& inputPrefix) {
    auto targetBegin = std::find_if_not(target.begin(), target.end(),
                                        [](char c) { return c == ' ' || c == '\t'; });
    size_t targetLen = static_cast<size_t>(std::distance(targetBegin, target.end()));
    if (targetLen < inputPrefix.size()) return false;
    return std::equal(
        inputPrefix.begin(), inputPrefix.end(), targetBegin,
        [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

std::vector<std::string> SplitTopLevel(const std::string& str);
using Converter = std::function<std::any(const std::string&)>;
void ParseRawComment(const std::string& comment,
                     std::map<std::string, std::any>& annotations,
                     const std::unordered_map<std::string, Converter>& converters);

std::string BuildCommentForNewick(const std::string& rawComment,
                                  const std::map<std::string, std::any>& annotations,
                                  const NewickExportOptions& options,
                                  const std::vector<std::string>& annotationKeys);

}  // namespace cladokit
