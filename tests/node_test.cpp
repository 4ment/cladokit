// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/node.hpp"

#include <gtest/gtest.h>

#include <cmath>

using cladokit::NewickExportOptions;
using cladokit::Node;

TEST(NodeTest, CreateNode) {
    Node::NodePtr node = std::make_shared<Node>("TestNode");
    EXPECT_EQ(node->Name(), "TestNode");
    EXPECT_EQ(node->Id(), 0);
    EXPECT_TRUE(std::isnan(node->Distance()));
}

TEST(NodeTest, SetAndGetName) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetName("NewName");
    EXPECT_EQ(node->Name(), "NewName");
}

TEST(NodeTest, SetAndGetId) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetId(42);
    EXPECT_EQ(node->Id(), 42);
}

TEST(NodeTest, SetAndGetDistance) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetDistance(3.14);
    EXPECT_DOUBLE_EQ(node->Distance(), 3.14);
}

TEST(NodeTest, AddAndRemoveChild) {
    Node::NodePtr parent = std::make_shared<Node>("Parent");
    Node::NodePtr child = std::make_shared<Node>("Child");

    EXPECT_TRUE(child->IsRoot());

    EXPECT_TRUE(parent->AddChild(child));
    EXPECT_EQ(parent->ChildCount(), 1);
    EXPECT_EQ(parent->ChildAt(0)->Name(), "Child");
    EXPECT_FALSE(child->IsRoot());

    EXPECT_TRUE(parent->RemoveChild(child));
    EXPECT_EQ(parent->ChildCount(), 0);
    EXPECT_TRUE(child->IsRoot());
}

TEST(NodeTest, SetAndGetParent) {
    Node::NodePtr parent = std::make_shared<Node>("Parent");
    Node::NodePtr child = std::make_shared<Node>("Child");
    child->SetParent(parent);
    EXPECT_EQ(child->Parent()->Name(), "Parent");
    EXPECT_FALSE(child->IsRoot());

    child->RemoveParent();
    EXPECT_TRUE(child->IsRoot());
}

TEST(NodeTest, IsRootAndIsLeaf) {
    Node::NodePtr root = std::make_shared<Node>("Root");
    Node::NodePtr child = std::make_shared<Node>("Child");

    EXPECT_TRUE(root->IsRoot());
    EXPECT_TRUE(root->IsLeaf());

    root->AddChild(child);
    EXPECT_FALSE(root->IsLeaf());
    EXPECT_FALSE(child->IsRoot());
    EXPECT_TRUE(child->IsLeaf());
}

TEST(NodeTest, ContainsAnnotation) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetAnnotation("key", "value");

    EXPECT_TRUE(node->ContainsAnnotation("key"));
    EXPECT_FALSE(node->ContainsAnnotation("nonexistent"));
}

TEST(NodeTest, SetAndGetAnnotation) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetAnnotation("key", 42);

    EXPECT_EQ(std::any_cast<int>(node->Annotation("key")), 42);

    node->SetAnnotation("key", "string_value");
    EXPECT_EQ(std::any_cast<std::string>(node->Annotation("key")), "string_value");

    EXPECT_THROW(node->Annotation("nonexistent"), std::out_of_range);
}

TEST(NodeTest, SetAndGetComment) {
    Node::NodePtr node = std::make_shared<Node>();
    node->SetComment("This is a comment");

    EXPECT_EQ(node->Comment(), "This is a comment");

    node->SetComment("");
    EXPECT_EQ(node->Comment(), "");
}

TEST(NodeTest, MakeBinary) {
    Node::NodePtr root = std::make_shared<Node>("Root");
    Node::NodePtr child1 = std::make_shared<Node>("Child1");
    Node::NodePtr child2 = std::make_shared<Node>("Child2");
    Node::NodePtr child3 = std::make_shared<Node>("Child3");

    root->AddChild(child1);
    root->AddChild(child2);
    root->AddChild(child3);

    EXPECT_EQ(root->ChildCount(), 3);

    bool madeBinary = root->MakeBinary();
    EXPECT_TRUE(madeBinary);

    EXPECT_EQ(root->ChildCount(), 2);
    EXPECT_EQ(root->ChildAt(0)->ChildCount(),
              2);  // First child should have two children
    EXPECT_EQ(root->ChildAt(0)->Parent(), root);

    madeBinary = root->MakeBinary();
    EXPECT_FALSE(madeBinary);
}

TEST(NodeTest, NewickExport) {
    Node::NodePtr node = std::make_shared<Node>("TestNode");
    node->SetDistance(1.23);

    NewickExportOptions options;
    options.includeBranchLengths = true;

    EXPECT_EQ(node->Newick(options), "TestNode:1.23");

    options.includeBranchLengths = false;
    EXPECT_EQ(node->Newick(options), "TestNode");
}

TEST(NodeTest, NewickExportWithChildren) {
    Node::NodePtr root = std::make_shared<Node>("Root");
    Node::NodePtr child1 = std::make_shared<Node>("Child1");
    Node::NodePtr child2 = std::make_shared<Node>("Child2");

    root->AddChild(child1);
    root->AddChild(child2);

    NewickExportOptions options;
    options.includeBranchLengths = false;

    EXPECT_EQ(root->Newick(options), "(Child1,Child2)");

    options.includeBranchLengths = true;
    child1->SetDistance(0.5);
    child2->SetDistance(0.75);

    EXPECT_EQ(root->Newick(options), "(Child1:0.5,Child2:0.75)");

    options.includeInternalNodeName = true;
    EXPECT_EQ(root->Newick(options), "(Child1:0.5,Child2:0.75)Root");
}

TEST(NodeTest, PostOrderIterator) {
    Node::NodePtr root = std::make_shared<Node>("Root");
    Node::NodePtr child1 = std::make_shared<Node>("Child1");
    Node::NodePtr child2 = std::make_shared<Node>("Child2");

    root->AddChild(child1);
    root->AddChild(child2);

    auto it = root->begin_postorder();
    EXPECT_EQ((*it)->Name(), "Child1");
    ++it;
    EXPECT_EQ((*it)->Name(), "Child2");
    ++it;
    EXPECT_EQ((*it)->Name(), "Root");
    ++it;
    EXPECT_TRUE(it == root->end_postorder());
}
