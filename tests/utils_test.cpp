// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/utils.hpp"

#include <gtest/gtest.h>

#include <any>
#include <string>

// using namespace cladokit;

TEST(ParseCommentTest, ParsesSimpleKeyValuePairs) {
    std::string input = "[&key1=1.23,key2=42,key3=abc,key4={0.1,0.2}]";
    std::map<std::string, std::any> annotations;
    std::unordered_map<std::string, cladokit::Converter> converters = {
        {"key1", [](const std::string& val) -> std::any { return std::stod(val); }},
        {"key2", [](const std::string& val) -> std::any { return std::stoi(val); }},
        {"key3", [](const std::string& val) -> std::any { return val; }}};
    cladokit::ParseRawComment(input, annotations, converters);

    EXPECT_TRUE(annotations.find("key1") != annotations.end());
    EXPECT_TRUE(annotations.find("key2") != annotations.end());
    EXPECT_TRUE(annotations.find("key3") != annotations.end());

    EXPECT_EQ(std::any_cast<double>(annotations["key1"]), 1.23);
    EXPECT_EQ(std::any_cast<int>(annotations["key2"]), 42);
    EXPECT_EQ(std::any_cast<std::string>(annotations["key3"]), "abc");
    EXPECT_EQ(std::any_cast<std::string>(annotations["key4"]), "{0.1,0.2}");
}

TEST(StartsWithCaseInsensitiveTest, SimpleTests) {
    EXPECT_TRUE(cladokit::StartsWithCaseInsensitive("Target", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitive("Taarget", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitive("Ta", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitive(" Target", "tar"));
}
TEST(StartsWithCaseInsensitiveLeftTrimTest, SimpleTests) {
    EXPECT_TRUE(cladokit::StartsWithCaseInsensitiveLeftTrim("Target", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitiveLeftTrim("Taarget", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitiveLeftTrim("Ta", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitiveLeftTrim("", "tar"));
    EXPECT_FALSE(cladokit::StartsWithCaseInsensitiveLeftTrim("     ", "tar"));
    EXPECT_TRUE(cladokit::StartsWithCaseInsensitiveLeftTrim("  Target", "tar"));
}
TEST(SplitTopLevelTest, SimpleCases) {
    std::string input = "a,b,c";
    auto result = cladokit::SplitTopLevel(input);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");

    input = "a,{b,c},d";
    result = cladokit::SplitTopLevel(input);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "{b,c}");
    EXPECT_EQ(result[2], "d");

    input = "{a,b},{c,d}";
    result = cladokit::SplitTopLevel(input);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "{a,b}");
    EXPECT_EQ(result[1], "{c,d}");
}
