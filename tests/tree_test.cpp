// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/tree.hpp"

#include <gtest/gtest.h>

using cladokit::Converter;
using cladokit::NewickExportOptions;
using cladokit::Tree;

TEST(TreeTest, CreateTreeFromNewick) {
    std::string newick = "((A:0.1,B:0.2),C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    EXPECT_EQ(tree->NodeCount(), 5);
    EXPECT_EQ(tree->LeafNodeCount(), 3);
    EXPECT_EQ(tree->InternalNodeCount(), 2);
    EXPECT_EQ(tree->Root()->Name(), "");
    EXPECT_TRUE(tree->IsRooted());
}

TEST(TreeTest, CompareTaxonNames) {
    std::string newick1 = "((A,B),C);";
    std::string newick2 = "((B,C),A);";
    std::string newick3 = "((C,A),B);";

    auto emptyTaxonNames = std::make_shared<std::vector<std::string>>();
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    std::map<std::string, size_t> taxonIndices = {{"A", 0}, {"B", 1}, {"C", 2}};

    auto tree0 = Tree::FromNewick(newick1);
    auto tree1 = Tree::FromNewick(newick1, taxonNames);
    auto tree2 = Tree::FromNewick(newick2, taxonNames);
    auto tree3 = Tree::FromNewick(newick3, taxonNames);

    EXPECT_EQ(*tree0->TaxonNames(), *taxonNames);
    EXPECT_EQ(*tree1->TaxonNames(), *taxonNames);

    auto checkLeafIds = [&](const Tree::TreePtr& tree) {
        for (auto it = tree->Root()->begin_postorder();
             it != tree->Root()->end_postorder(); ++it) {
            const auto& node = *it;
            if (node->IsLeaf()) {
                auto expectedIdIt = taxonIndices.find(node->Name());
                ASSERT_NE(expectedIdIt, taxonIndices.end())
                    << "Unknown taxon name: " << node->Name();
                EXPECT_EQ(expectedIdIt->second, node->Id())
                    << "Mismatch in taxon ID for: " << node->Name();
            }
        }
    };

    checkLeafIds(tree0);
    checkLeafIds(tree1);
    checkLeafIds(tree2);
    checkLeafIds(tree3);
}

TEST(TreeTest, CreateTreeFromNewickWithComments) {
    std::string newick = "((A:0.1,B:0.2)[&key=value],C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);
    tree->Root()->ChildAt(0)->ParseRawComment(
        {{"key", [](const std::string& val) -> std::any { return val; }}});

    EXPECT_EQ(tree->NodeCount(), 5);
    EXPECT_EQ(tree->LeafNodeCount(), 3);
    EXPECT_EQ(tree->InternalNodeCount(), 2);
    EXPECT_EQ(tree->Root()->Name(), "");
    EXPECT_TRUE(tree->IsRooted());
    EXPECT_TRUE(tree->Root()->ChildAt(0)->ContainsAnnotation("key"));
}

TEST(TreeTest, DeRootTree) {
    std::string newick = "(A:0.1,B:0.2,C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    EXPECT_FALSE(tree->IsRooted());
    EXPECT_TRUE(tree->DeRoot());
    EXPECT_TRUE(tree->IsRooted());
    EXPECT_FALSE(tree->DeRoot());
}

TEST(TreeTest, MakeBinaryTree) {
    std::string newick = "((A:0.1,B:0.2,C:0.3),D:0.4);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C", "D"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    EXPECT_EQ(tree->NodeCount(), 6);
    EXPECT_EQ(tree->LeafNodeCount(), 4);
    EXPECT_EQ(tree->InternalNodeCount(), 2);

    EXPECT_TRUE(tree->MakeBinary());

    EXPECT_EQ(tree->NodeCount(), 7);  // One internal node was added
}

TEST(TreeTest, NewickExport) {
    std::string newick = "((A:[&a=b]0.1,B:0.2):0.3,C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    EXPECT_EQ(tree->Newick(), "((A:0.1,B:0.2):0.3,C:2);");

    NewickExportOptions options;
    options.annotationKeys = {"key"};
    tree->Root()->ChildAt(0)->SetAnnotation("key", "value");
    EXPECT_EQ(tree->Newick(options), "((A:0.1,B:0.2):[&key=value]0.3,C:2);");

    options.annotationKeys.clear();
    options.includeRawComment = true;
    EXPECT_EQ(tree->Newick(options), "((A:[&a=b]0.1,B:0.2):0.3,C:2);");

    options.includeRawComment = false;
    options.decimalPrecision = 2;
    EXPECT_EQ(tree->Newick(options), "((A:0.10,B:0.20):0.30,C:2.00);");
}

TEST(TreeTest, ComputeBipartitions) {
    std::string newick = "((A:0.1,B:0.2),C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    tree->ComputeBiPartitions();
    const auto& bipartitions = tree->BiPartitions();
    std::vector<bool> allTrue(3, true);

    EXPECT_EQ(bipartitions[tree->Root()->Id()], allTrue);  // Root includes all taxa
    EXPECT_EQ(bipartitions.size(), 5);                     // 5 nodes in the tree
    EXPECT_EQ(bipartitions[0].size(), 3);                  // 3 leaves
}

TEST(TreeTest, PostOrderIterator) {
    std::string newick = "((A:0.1,B:0.2),C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    size_t count = 0;
    for (auto it = tree->Root()->begin_postorder(); it != tree->Root()->end_postorder();
         ++it) {
        count++;
    }

    EXPECT_EQ(count, 5);  // 5 nodes in the tree

    auto it = tree->Root()->begin_postorder();
    EXPECT_EQ((*it)->Name(), "A");
    ++it;
    EXPECT_EQ((*it)->Name(), "B");
    ++it;
    ++it;
    EXPECT_EQ((*it)->Name(), "C");
    ++it;
    EXPECT_TRUE((*it)->IsRoot());
    ++it;
    EXPECT_EQ(it, tree->Root()->end_postorder());
}

TEST(TreeTest, PreOrderIterator) {
    std::string newick = "((A:0.1,B:0.2),C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    size_t count = 0;
    for (auto it = tree->Root()->begin_preorder(); it != tree->Root()->end_preorder();
         ++it) {
        count++;
    }

    EXPECT_EQ(count, 5);  // 5 nodes in the tree

    auto it = tree->Root()->begin_preorder();
    EXPECT_EQ(*it, tree->Root());
    ++it;
    ++it;
    EXPECT_EQ((*it)->Name(), "A");
    ++it;
    EXPECT_EQ((*it)->Name(), "B");
    ++it;
    EXPECT_EQ((*it)->Name(), "C");
    ++it;
    EXPECT_EQ(it, tree->Root()->end_preorder());
}

TEST(TreeTest, ParseRawComment) {
    std::string newick = "((A:0.1,B:0.2)[&key=value],C:2);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    std::unordered_map<std::string, Converter> converters = {
        {"key", [](const std::string& val) -> std::any { return val; }}};

    for (auto it = tree->Root()->begin_postorder(); it != tree->Root()->end_postorder();
         ++it) {
        auto node = *it;
        node->ParseRawComment(converters);
    }

    EXPECT_TRUE(tree->Root()->ChildAt(0)->ContainsAnnotation("key"));
    EXPECT_EQ(std::any_cast<std::string>(tree->Root()->ChildAt(0)->Annotation("key")),
              "value");
    EXPECT_EQ(tree->Root()->ChildAt(0)->Annotation<std::string>("key"), "value");
}

TEST(TreeTest, RerootAbove) {
    std::string newick = "((A:0.1,B:0.2):0.3,C:0.4);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    // Reroot should be ignored if called on the root
    tree->ReRootAbove(tree->Root());
    EXPECT_EQ(tree->Newick(), "((A:0.1,B:0.2):0.3,C:0.4);");

    // Reroot above A
    auto nodeToReroot = tree->Root()->ChildAt(0)->ChildAt(0);
    tree->ReRootAbove(nodeToReroot);
    EXPECT_EQ(tree->Newick(), "(A:0.05,(B:0.2,C:0.7):0.05);");
}

TEST(TreeTest, RerootAbove2) {
    std::string newick = "(((A:1,B:2):3,C:4):5,D:6);";
    auto taxonNames = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"A", "B", "C", "D"});
    auto tree = Tree::FromNewick(newick, taxonNames);

    // Reroot above A
    auto nodeToReroot = tree->Root()->ChildAt(0)->ChildAt(0)->ChildAt(0);
    tree->ReRootAbove(nodeToReroot);
    EXPECT_EQ(tree->Newick(), "(A:0.5,(B:2,(C:4,D:11):3):0.5);");

    // Check that all nodes have correct parent pointers
    for (auto it = tree->Root()->begin_postorder(); it != tree->Root()->end_postorder();
         ++it) {
        auto node = *it;
        if (node != tree->Root()) {
            for (auto child : node->Children()) {
                EXPECT_EQ(child->Parent(), node);
            }
        }
    }
    EXPECT_TRUE(tree->Root()->IsRoot());
}
