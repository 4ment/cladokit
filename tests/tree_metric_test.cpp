// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/tree_metric.hpp"

#include <gtest/gtest.h>

#include "cladokit/tree.hpp"

using cladokit::BiPartition;
using cladokit::RobinsonFouldsMetric;
using cladokit::Tree;
using cladokit::TreeMetric;

TEST(TreeMetricTest, RobinsonFouldsMetric) {
    std::shared_ptr<std::vector<std::string>> taxonNames =
        std::make_shared<std::vector<std::string>>(
            std::vector<std::string>{"A", "B", "C"});
    auto tree1 = Tree::FromNewick("((A:0.1,B:0.2),C:2);", taxonNames);
    auto tree2 = Tree::FromNewick("((B:0.1,C:0.2),A:2);", taxonNames);

    tree1->ComputeBiPartitions();
    tree2->ComputeBiPartitions();

    RobinsonFouldsMetric metric;

    EXPECT_DOUBLE_EQ(metric.Compute(tree1, tree1), 0.0);
    EXPECT_DOUBLE_EQ(metric.Compute(tree1, tree2), 2.0);
}

TEST(TreeMetricTest, RobinsonFouldsMetric2) {
    std::shared_ptr<std::vector<std::string>> taxonNames =
        std::make_shared<std::vector<std::string>>();
    auto tree1 = Tree::FromNewick("((A,B),(C,D));", taxonNames);
    auto tree2 = Tree::FromNewick("(((A,B),C),D);", taxonNames);

    tree1->ComputeBiPartitions();
    tree2->ComputeBiPartitions();

    RobinsonFouldsMetric metric;

    EXPECT_DOUBLE_EQ(metric.Compute(tree1, tree1), 0.0);
    EXPECT_DOUBLE_EQ(metric.Compute(tree1, tree2), 2.0);
}
