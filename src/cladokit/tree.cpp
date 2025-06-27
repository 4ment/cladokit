// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/tree.hpp"

#include <iostream>
#include <stack>
#include <string>
#include <unordered_set>

using cladokit::Node;
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
    nodes_.resize(leafCount_);
    for (auto it = root->begin_postorder(); it != root->end_postorder(); ++it) {
        auto node = *it;
        if (!node->IsLeaf()) {
            node->SetId(leafCount_ + internalCount_++);
            nodes_.push_back(node);
        } else {
            nodes_[node->Id()] = node;
        }
    }
    nodeCount_ = leafCount_ + internalCount_;
}

Tree::Tree(const Node::NodePtr &root, std::shared_ptr<vector<string>> taxonNames)
    : root_(root), taxonNames_(taxonNames) {
    UpdateIDs();
}

void Tree::SetTaxonNames(std::shared_ptr<std::vector<string>> taxonNames) {
    taxonNames_ = taxonNames;
    UpdateIDs();
}

void Tree::UpdateIDs() {
    internalCount_ = 0;
    leafCount_ = taxonNames_->size();
    nodes_.clear();
    nodes_.resize(leafCount_);

    for (auto it = root_->begin_postorder(); it != root_->end_postorder(); ++it) {
        auto node = *it;
        if (!node->IsLeaf()) {
            node->SetId(leafCount_ + internalCount_++);
            nodes_.push_back(node);
        } else {
            auto it = std::find(taxonNames_->begin(), taxonNames_->end(), node->Name());
            if (it != taxonNames_->end()) {
                size_t taxonIndex = std::distance(taxonNames_->begin(), it);
                node->SetId(taxonIndex);
                nodes_[taxonIndex] = node;
            } else {
                std::cerr << "Error: taxon name " << node->Name()
                          << " not found in taxon names" << std::endl;
            }
        }
    }
    nodeCount_ = leafCount_ + internalCount_;
}

Node::NodePtr Tree::LeafFromName(const string &name) const {
    auto it = std::find(taxonNames_->begin(), taxonNames_->end(), name);
    if (it != taxonNames_->end()) {
        size_t index = static_cast<size_t>(std::distance(taxonNames_->begin(), it));
        return nodes_.at(index);
    } else {
        return nullptr;
    }
}

bool Tree::MakeRooted() {
    size_t degree = root_->ChildCount();
    if (degree > 2) {
        ReRootAbove(root_->ChildAt(0));
    }
    return degree > 2;
}

bool Tree::MakeUnRooted() {
    auto children = root_->Children();
    size_t degree = children.size();
    if (degree == 2) {
        auto it = children.begin();
        while (it != children.end()) {
            auto node = *it;
            if (!node->IsLeaf()) {
                break;
            }
            ++it;
        }
        auto node = *it;
        node->Collapse();
        UpdateIDs();
    }
    return degree == 2;
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

string Tree::Newick(const NewickExportOptions &options) {
    return root_->Newick(options) + ";";
}

Tree::TreePtr Tree::FromNewick(const string &newick) {
    auto taxonNames = std::make_shared<std::vector<string>>();
    return FromNewick(newick, taxonNames);
}

Tree::TreePtr Tree::FromNewick(const string &newick,
                               std::shared_ptr<std::vector<string>> taxonNames) {
    size_t taxonCounter = 0;
    std::stack<Node::NodePtr> nodeStack;
    std::vector<string> currentTaxonNames;
    bool justClosed = false;

    for (size_t i = 0; i < newick.size(); i++) {
        char c = newick.at(i);
        // node comment
        if (c == '[') {
            size_t start = i++;
            while (newick.at(i) != ']') {
                i++;
            }
            const string comment = newick.substr(start, i - start + 1);
            nodeStack.top()->SetComment(comment);
            //   std::cout << "Comment: " << comment << " for "
            //             << nodeStack.top()->GetName() << std::endl;
        } else if (c == ':') {
            size_t start = ++i;
            // branch comment
            if (newick.at(i) == '[') {
                while (newick.at(i) != ']') {
                    i++;
                }
                const string comment = newick.substr(start, i - start + 1);
                nodeStack.top()->SetBranchComment(comment);
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
                // should strip quotes if present and requested
                size_t taxonIndex = 0;
                taxonIndex = taxonCounter++;
                currentTaxonNames.push_back(identifier);
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
    } else if (std::unordered_set<string>(taxonNames->begin(), taxonNames->end()) !=
               std::unordered_set<string>(currentTaxonNames.begin(),
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

void Tree::ComputeDescendantBitset() {
    size_t bitsetSize = LeafNodeCount();
    for (auto it = root_->begin_postorder(); it != root_->end_postorder(); ++it) {
        auto node = *it;
        node->ComputeDescendantBitset(bitsetSize);
    }
}

void Tree::ReRootAbove(std::shared_ptr<Node> node) {
    // node is already the root
    if (node->IsRoot()) {
        return;
    }
    auto parentNode = node->Parent();

    // node is just below the root, just choose the middle of the branch
    if (parentNode->IsRoot()) {
        double midpoint = node->Distance() / 2;
        node->SetDistance(node->Distance() - midpoint);
        auto siblings = node->Siblings();

        // If the parent node is not binary, we need to make it binary.
        // The sibling of node could still be multifurcating.
        if (parentNode->ChildCount() > 2) {
            auto newNode = std::make_shared<Node>();
            for (auto sibling : siblings) {
                parentNode->RemoveChild(sibling);
                newNode->AddChild(sibling);
            }
            parentNode->AddChild(newNode);
            newNode->SetDistance(midpoint);

            UpdateIDs();
        } else {
            siblings[0]->SetDistance(siblings[0]->Distance() + midpoint);
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

            if (nParent) {
                auto temp2 = n->Parent();
                nParent->RemoveChild(n);  // n does not have a parent anymore
                n->SetParent(temp2);
            }
        }

        // nparent is the old root and the affected lineage disconnected.
        // n is the child of the old root that was flipped
        if (nParent && nParent->ChildCount() > 0) {
            auto unaffectedSibling = nParent->ChildAt(0);
            unaffectedSibling->SetDistance(unaffectedSibling->Distance() + branchLength);
            n->AddChild(unaffectedSibling);
        }

        root_ = newRoot;
        UpdateIDs();
    }
}

Tree::TreePtr Tree::Random(std::vector<string> taxonNames) {
    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto &name : taxonNames) {
        nodes.push_back(std::make_shared<Node>(name));
    }
    while (nodes.size() > 1) {
        int index1 = rand() % nodes.size();
        int index2 = rand() % nodes.size();
        while (index1 == index2) {
            index2 = rand() % nodes.size();
        }
        std::shared_ptr<Node> newNode = std::make_shared<Node>();
        newNode->AddChild(nodes[index1]);
        newNode->AddChild(nodes[index2]);
        nodes.push_back(newNode);
        nodes.erase(nodes.begin() + std::max(index1, index2));
        nodes.erase(nodes.begin() + std::min(index1, index2));
    }
    return std::make_shared<Tree>(nodes[0],
                                  std::make_shared<std::vector<string>>(taxonNames));
}
