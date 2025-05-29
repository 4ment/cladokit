// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "cladokit/tree.hpp"
#include "cladokit/treeio.hpp"

namespace cladokit {
class NewickFile : public TreeFile {
   public:
    using TreeFile::TreeFile;

    size_t Count() override;

    std::vector<std::shared_ptr<Tree>> Parse() override;

    std::shared_ptr<Tree> Next() override;

    bool HasNext() override;

    void SkipNext() override;

   private:
    std::string currentTreeString_ = "";
};
}  // namespace cladokit
