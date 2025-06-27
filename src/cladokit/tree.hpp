// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cladokit/newick_options.hpp"
#include "cladokit/node.hpp"

namespace cladokit {

class Tree {
   public:
    using TreePtr = std::shared_ptr<Tree>;

    explicit Tree(const Node::NodePtr& root);

    Tree(const Node::NodePtr& root, std::shared_ptr<std::vector<std::string>> taxonNames);

    std::shared_ptr<std::vector<std::string>> TaxonNames() const { return taxonNames_; }

    void SetTaxonNames(std::shared_ptr<std::vector<std::string>> taxonNames);

    size_t NodeCount() const { return nodeCount_; }

    size_t InternalNodeCount() const { return internalCount_; }

    size_t LeafNodeCount() const { return leafCount_; }

    Node::NodePtr Root() const { return root_; }

    Node::NodePtr NodeFromId(size_t id) const { return nodes_.at(id); }

    Node::NodePtr LeafFromName(const std::string& name) const;

    bool IsRooted() const { return root_->ChildCount() == 2; }

    bool MakeRooted();  // return true if the tree was unrooted (i.e. degree > 2)

    bool MakeUnRooted();  // return true if the tree was rooted (i.e. degree = 2)

    bool MakeBinary();  // return true if any node was made binary

    void ReRootAbove(std::shared_ptr<Node> node);

    void UpdateIDs();

    std::string Newick();

    std::string Newick(const NewickExportOptions& options);

    static std::shared_ptr<Tree> Random(std::vector<std::string> taxonNames);

    static std::shared_ptr<Tree> FromNewick(const std::string& newick);

    static std::shared_ptr<Tree> FromNewick(
        const std::string& newick, std::shared_ptr<std::vector<std::string>> taxonNames);

    void ComputeDescendantBitset();

   private:
    Node::NodePtr root_;
    size_t leafCount_ = 0;
    size_t internalCount_ = 0;
    size_t nodeCount_ = 0;
    std::shared_ptr<std::vector<std::string>> taxonNames_;
    std::vector<Node::NodePtr> nodes_;
    std::map<std::string, std::any> annotations_;
    std::string comment_;  // raw comment extraced from newick file
};
}  // namespace cladokit
