// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/tree.hpp"

#include <iostream>
#include <stack>
#include <string>
#include <unordered_set>

using cladokit::Tree;
using std::string;
using std::vector;

Tree::Tree(const Node::NodePtr &root) : root_(root) {
    for (auto it = root->begin_postorder(); it != root->end_postorder(); ++it) {
        auto node = *it;
        if (node->IsLeaf()) {
            node->SetId(leafCount_++);
            taxonNames_->push_back(node->Name());
        }
    }
    for (auto it = root->begin_postorder(); it != root->end_postorder(); ++it) {
        auto node = *it;
        if (!node->IsLeaf()) {
            node->SetId(leafCount_ + internalCount_++);
        }
    }
    nodeCount_ = leafCount_ + internalCount_;
}

Tree::Tree(const Node::NodePtr &root, std::shared_ptr<vector<string>> taxonNames)
    : root_(root), taxonNames_(taxonNames) {
    UpdateIDs();
}

void Tree::UpdateIDs() {
    internalCount_ = 0;
    leafCount_ = taxonNames_->size();
    for (auto it = root_->begin_postorder(); it != root_->end_postorder(); ++it) {
        auto node = *it;
        if (!node->IsLeaf()) {
            node->SetId(leafCount_ + internalCount_++);
        } else {
            auto it = std::find(taxonNames_->begin(), taxonNames_->end(), node->Name());
            if (it != taxonNames_->end()) {
                size_t taxonIndex = std::distance(taxonNames_->begin(), it);
                node->SetId(taxonIndex);
            } else {
                std::cerr << "Error: taxon name " << node->Name()
                          << " not found in taxon names" << std::endl;
            }
        }
    }
    nodeCount_ = leafCount_ + internalCount_;
}

bool Tree::DeRoot() {
    size_t degree = root_->ChildCount();
    if (degree > 2) {
        root_->MakeBinary();
        UpdateIDs();
    }
    return degree > 2;
}

bool Tree::MakeBinary() {
    bool madeBinary = false;
    for (auto it = root_->begin_postorder(); it != root_->end_postorder(); ++it) {
        auto node = *it;
        if (node->IsLeaf()) continue;
        size_t degree = node->ChildCount();
        if (degree > 2) {
            madeBinary |= node->MakeBinary();
        }
    }
    if (madeBinary) {
        UpdateIDs();
    }
    return madeBinary;
}

string Tree::Newick() { return root_->Newick() + ";"; }

std::string Tree::Newick(const NewickExportOptions &options) {
    return root_->Newick(options) + ";";
}

std::shared_ptr<Tree> Tree::FromNewick(const string &newick) {
    auto taxonNames = std::make_shared<std::vector<std::string>>();
    return FromNewick(newick, taxonNames);
}

std::shared_ptr<Tree> Tree::FromNewick(
    const string &newick, std::shared_ptr<std::vector<std::string>> taxonNames) {
    size_t taxonCounter = 0;
    std::stack<Node::NodePtr> nodeStack;
    std::vector<std::string> currentTaxonNames;
    bool justClosed = false;

    for (size_t i = 0; i < newick.size(); i++) {
        char c = newick.at(i);
        if (c == '[') {
            size_t start = ++i;
            while (newick.at(i) != ']') {
                i++;
            }
            const std::string comment = newick.substr(start, i - start + 1);
            nodeStack.top()->SetComment(comment);
            //   std::cout << "Comment: " << comment << " for "
            //             << nodeStack.top()->GetName() << std::endl;
        } else if (c == ':') {
            size_t start = ++i;
            // there is a comment between the colon and the branch length
            if (newick.at(i) == '[') {
                while (newick.at(i) != ']') {
                    i++;
                }
                const std::string comment = newick.substr(start, i - start + 1);
                nodeStack.top()->SetComment(comment);
                start = ++i;
            }

            for (; i < newick.size(); i++) {
                c = newick.at(i);
                if (c == '[' || c == ',' || c == ')' || c == ';') {
                    i--;
                    break;
                }
            }
            const string branchLengthString = newick.substr(start, i - start + 1);
            //   std::cout << "Branch length: " << branchLengthString << " for "
            //             << nodeStack.top()->GetName() << std::endl;
            double branchLength = std::stod(branchLengthString);
            nodeStack.top()->SetDistance(branchLength);
        } else if (c != '(' && c != ')' && c != ',' && c != ';') {
            size_t start = i;
            for (; i < newick.size(); i++) {
                c = newick.at(i);
                if (c == ':' || c == '[' || c == ',' || c == ')' || c == ';') {
                    i--;
                    break;
                }
            }
            const string identifier = newick.substr(start, i - start + 1);

            if (justClosed) {
                nodeStack.top()->SetName(identifier);
            } else {
                size_t taxonIndex = 0;
                // auto it = std::find(taxonNames.begin(), taxonNames.end(),
                // identifier); if (it != taxonNames.end()) {
                //     taxonIndex = std::distance(taxonNames.begin(), it);
                // } else {
                taxonIndex = taxonCounter++;
                currentTaxonNames.push_back(identifier);
                // }
                auto node = std::make_shared<Node>(identifier);
                node->SetId(taxonIndex);
                // std::cout << "Taxon: " << identifier << " (" << taxonIndex << ")"
                //           << std::endl;
                nodeStack.top()->AddChild(node);
                nodeStack.push(node);
            }
        } else if (c == '(') {
            justClosed = false;
            auto node = std::make_shared<Node>();
            if (!nodeStack.empty()) {
                nodeStack.top()->AddChild(node);
            }
            nodeStack.push(node);
        } else if (c == ')' || c == ',') {
            //   std::cout << "pop Node: " << nodeStack.top()->GetName() << std::endl;
            nodeStack.pop();
            // check if there is a name after the closing parenthesis
            justClosed = c == ')';
        } else {
            // std::cout << "Unknown character: " << c << std::endl;
        }
    }

    if (taxonNames->empty()) {
        *taxonNames = currentTaxonNames;
    } else if (std::unordered_set<std::string>(taxonNames->begin(), taxonNames->end()) !=
               std::unordered_set<std::string>(currentTaxonNames.begin(),
                                               currentTaxonNames.end())) {
        std::cerr << "Error: taxon names do not match" << std::endl;
        for (const auto &name : currentTaxonNames) {
            if (std::find(taxonNames->begin(), taxonNames->end(), name) ==
                taxonNames->end()) {
                std::cerr << "Missing taxon name: " << name << std::endl;
            }
        }
        for (const auto &name : *taxonNames) {
            if (std::find(currentTaxonNames.begin(), currentTaxonNames.end(), name) ==
                currentTaxonNames.end()) {
                std::cerr << "Extra taxon name: " << name << std::endl;
            }
        }
    }

    auto tree = std::make_shared<Tree>(nodeStack.top(), taxonNames);
    return tree;
}

void Tree::ComputeBiPartitions() {
    if (bitSets_.size() != nodeCount_) {
        bitSets_.resize(nodeCount_, std::vector<bool>(LeafNodeCount(), false));
    }
    size_t bitsetSize = LeafNodeCount();
    for (auto it = root_->begin_postorder(); it != root_->end_postorder(); ++it) {
        auto node = *it;
        auto &nodeBitset = bitSets_[node->Id()];
        if (node->IsLeaf()) {
            nodeBitset[node->Id()] = true;
        } else {
            std::fill(nodeBitset.begin(), nodeBitset.end(), false);

            for (auto child : node->Children()) {
                auto &childBitset = bitSets_[child->Id()];
                for (size_t i = 0; i < bitsetSize; i++) {
                    nodeBitset[i] = nodeBitset[i] | childBitset[i];
                }
            }
        }
    }
}

void Tree::ReRootAbove(std::shared_ptr<Node> node) {
    // node is already the root
    if (node->IsRoot()) {
        return;
    }
    auto parentNode = node->Parent();

    // node is just below the root, just choose the middle of the branch
    if (node->Parent()->IsRoot()) {
        double midpoint = node->Distance() / 2;
        node->SetDistance(node->Distance() - midpoint);

        // If the parent node is not binary, we need to make it binary.
        // The sibliing of node could still be multifurcating.
        if (parentNode->ChildCount() > 2) {
            auto newNode = std::make_shared<Node>();
            for (auto child : parentNode->Children()) {
                if (child != node) {
                    parentNode->RemoveChild(child);
                    newNode->AddChild(child);
                }
            }
            parentNode->AddChild(newNode);
        }
        for (auto child : parentNode->Children()) {
            if (child != node) {
                child->SetDistance(child->Distance() + midpoint);
            }
        }
    } else {
        std::shared_ptr<Node> newRoot = std::make_shared<Node>();
        auto parent = node->Parent();
        auto grandParent = parent->Parent();

        newRoot->AddChild(node);
        newRoot->AddChild(parent);

        double branchLength = parent->Distance();
        double midpoint = node->Distance() / 2;

        node->SetDistance(midpoint);
        parent->SetDistance(midpoint);
        // the node to which we need to add its parent as a child recurssively
        auto n = parent;
        // the parent which is a ref to the rest of the old tree
        auto nParent = grandParent;

        grandParent->RemoveChild(parent);
        parent->RemoveChild(node);

        node->SetParent(newRoot);
        parent->SetParent(newRoot);

        while (!nParent->IsRoot()) {
            auto temp = nParent->Parent();
            n->AddChild(nParent);  // nparent has n as parent now

            double bl = nParent->Distance();
            nParent->SetDistance(branchLength);
            branchLength = bl;

            nParent->SetParent(n);

            n = nParent;
            nParent = temp;

            temp = n->Parent();
            nParent->RemoveChild(n);  // n does not have a parent anymore
            n->SetParent(temp);
        }

        // nparent is the old root and the affected lineage disconnected.
        // n is the child of the old root that was flipped
        auto unaffectedSibling = nParent->ChildAt(0);
        unaffectedSibling->SetDistance(unaffectedSibling->Distance() + branchLength);
        n->AddChild(unaffectedSibling);

        root_ = newRoot;
        UpdateIDs();
    }
}
