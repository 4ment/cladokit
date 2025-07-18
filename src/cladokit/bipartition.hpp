// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cladokit/tree.hpp"

namespace cladokit {
using BiPartition = std::vector<bool>;

struct BiPartitionHash {
    std::size_t operator()(const BiPartition& v) const {
        // return std::hash<std::vector<bool>>{}(v);
        std::size_t h = 0;
        for (bool b : v) {
            h = (h << 1) ^ std::hash<bool>{}(b);
        }
        return h;
    }
};

using BiPartitionSet = std::unordered_set<BiPartition, BiPartitionHash>;

static BiPartitionSet GetBiPartitionSet(const Tree::TreePtr& tree) {
    std::unordered_set<BiPartition, BiPartitionHash> result;
    for (auto it = tree->Root()->begin_postorder(); it != tree->Root()->end_postorder();
         ++it) {
        auto node = *it;
        if (!node->IsRoot() && !node->IsLeaf()) {
            result.insert(node->DescendantBitset());
        }
    }

    return result;
}
}  // namespace cladokit
