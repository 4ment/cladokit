// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cladokit/treeio.hpp"

// use cladokit::Tree;
namespace cladokit {

class NexusFile : public TreeFile {
   public:
    using TreeFile::TreeFile;

    size_t Count() override;

    std::vector<std::shared_ptr<Tree>> Parse() override;

    std::shared_ptr<Tree> Next() override;

    bool HasNext() override;

    void SkipNext() override;

    void ParseTranslate();

    std::string nextLineUncommented();

    bool findBlock(const std::string &blockName);

   protected:
    std::shared_ptr<Tree> ParseTreeLine(const std::string &buffer);
    void PointToFirstTree();

   private:
    std::map<std::string, std::string> translateMap_;
    std::map<std::string, size_t> taxonMap_;
    bool translateParsed_ = false;
    std::string currentTreeString_ = "";
};
}  // namespace cladokit
