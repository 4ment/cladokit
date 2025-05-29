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

static std::unordered_set<BiPartition, BiPartitionHash> GetBiPartitionSet(
    const Tree::TreePtr& tree) {
    std::unordered_set<BiPartition, BiPartitionHash> result;
    const auto& bipartitions = tree->BiPartitions();
    size_t rootId = tree->Root()->Id();

    for (size_t i = 0; i < bipartitions.size(); ++i) {
        if (i == rootId) continue;
        const auto& bip = bipartitions[i];
        result.insert(bip);
    }

    return result;
}
}  // namespace cladokit
