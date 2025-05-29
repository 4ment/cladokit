// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
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

    size_t NodeCount() const { return nodeCount_; }

    size_t InternalNodeCount() const { return internalCount_; }

    size_t LeafNodeCount() const { return leafCount_; }

    Node::NodePtr Root() const { return root_; }

    bool IsRooted() const { return root_->ChildCount() == 2; }

    bool DeRoot();  // return true if the tree was de-rooted

    bool MakeBinary();  // return true if any node was made binary

    void ReRootAbove(std::shared_ptr<Node> node);

    void UpdateIDs();

    std::string Newick();

    std::string Newick(const NewickExportOptions& options);

    void ComputeBiPartitions();

    const std::vector<std::vector<bool>>& BiPartitions() const { return bitSets_; }

    static std::shared_ptr<Tree> FromNewick(const std::string& newick);

    static std::shared_ptr<Tree> FromNewick(
        const std::string& newick, std::shared_ptr<std::vector<std::string>> taxonNames);

   private:
    Node::NodePtr root_;
    size_t leafCount_ = 0;
    size_t internalCount_ = 0;
    size_t nodeCount_ = 0;
    std::shared_ptr<std::vector<std::string>> taxonNames_;
    std::vector<std::vector<bool>> bitSets_;
};
}  // namespace cladokit
