// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "cladokit/tree.hpp"

namespace cladokit {
class TreeFile {
   public:
    explicit TreeFile(std::istream &in) : in_(in) {}

    TreeFile(std::istream &in, std::shared_ptr<std::vector<std::string>> taxonNames)
        : in_(in), taxonNames_(taxonNames) {}

    virtual ~TreeFile() = default;

    virtual size_t Count() = 0;

    virtual std::vector<std::shared_ptr<Tree>> Parse() = 0;

    virtual std::shared_ptr<Tree> Next() = 0;

    virtual bool HasNext() = 0;

    virtual void SkipNext() = 0;

   protected:
    std::istream &in_;
    std::shared_ptr<std::vector<std::string>> taxonNames_;
    size_t count_ = 0;
};
}  // namespace cladokit
