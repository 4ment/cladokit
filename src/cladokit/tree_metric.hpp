// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include "cladokit/bipartition.hpp"
#include "cladokit/tree.hpp"

namespace cladokit {

class TreeMetric {
   public:
    virtual ~TreeMetric() = default;
    virtual double Compute(const Tree::TreePtr& tree1, const Tree::TreePtr& tree2) = 0;

   protected:
    TreeMetric() = default;
};

class RobinsonFouldsMetric : public TreeMetric {
   public:
    double Compute(const Tree::TreePtr& tree1, const Tree::TreePtr& tree2) override {
        auto bip1 = GetBiPartitionSet(tree1);
        auto bip2 = GetBiPartitionSet(tree2);

        size_t shared = 0;
        for (const auto& b : bip1) {
            if (bip2.find(b) != bip2.end()) {
                ++shared;
            }
        }

        size_t total = bip1.size() + bip2.size();
        return static_cast<double>(total - 2 * shared);
    }
};
}  // namespace cladokit
