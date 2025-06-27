// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/node.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "cladokit/newick_options.hpp"

using cladokit::Node;
using std::string;

Node::Node() : Node("") {}

Node::Node(const std::string &name) : name_(name) {}

const string &Node::Name() const { return name_; }

void Node::SetName(const string &name) { name_ = name; }

size_t Node::Id() const { return id_; }

void Node::SetId(size_t id) { id_ = id; }

double Node::Distance() const { return distance_; }

void Node::SetDistance(double distance) { distance_ = distance; }

std::vector<Node::NodePtr> Node::Siblings() const {
    std::vector<NodePtr> siblings;
    if (parent_.expired()) {
        return siblings;  // No parent, no siblings
    }

    siblings.reserve(parent_.lock()->Children().size() - 1);

    for (const auto &node : parent_.lock()->Children()) {
        if (node != shared_from_this()) {
            siblings.push_back(node);
        }
    }
    return siblings;
}

bool Node::AddChild(const NodePtr &node) {
    if (find(children_.begin(), children_.end(), node) == children_.end()) {
        node->SetParent(shared_from_this());
        children_.push_back(node);
        return true;
    }
    return false;
}

bool Node::RemoveChild(NodePtr node) {
    for (size_t i = 0; i < children_.size(); i++) {
        if (children_.at(i) == node) {
            node->RemoveParent();
            children_.erase(children_.begin() + i);
            return true;
        }
    }
    return false;
}

bool Node::IsRoot() const { return parent_.expired(); }

bool Node::IsLeaf() const { return children_.empty(); }

void Node::Collapse() {
    auto parent = Parent();
    parent->RemoveChild(shared_from_this());
    for (auto child : children_) {
        parent->AddChild(child);
    }
    children_.clear();
}

// Node annotations
std::vector<std::string> Node::AnnotationKeys() const {
    std::vector<std::string> keys;
    keys.reserve(annotations_.size());
    for (const auto &pair : annotations_) {
        keys.push_back(pair.first);
    }
    return keys;
}
std::any Node::Annotation(const std::string &key) const {
    auto it = annotations_.find(key);
    if (it == annotations_.end()) {
        throw std::out_of_range("Key not found: " + key);
    }
    return it->second;
}
void Node::SetAnnotation(const string &key, std::any value) {
    annotations_[key] = std::move(value);
}

void Node::SetAnnotation(const std::string &key, const char *value) {
    annotations_[key] = std::string(value);
}

bool Node::ContainsAnnotation(const string &key) const {
    return annotations_.find(key) != annotations_.end();
}

void Node::RemoveAnnotation(const string &key) { annotations_.erase(key); }

// Branch annotations
std::vector<std::string> Node::BranchAnnotationKeys() const {
    std::vector<std::string> keys;
    keys.reserve(branchAnnotations_.size());
    for (const auto &pair : branchAnnotations_) {
        keys.push_back(pair.first);
    }
    return keys;
}
std::any Node::BranchAnnotation(const std::string &key) const {
    auto it = branchAnnotations_.find(key);
    if (it == branchAnnotations_.end()) {
        throw std::out_of_range("Key not found: " + key);
    }
    return it->second;
}
void Node::SetBranchAnnotation(const string &key, std::any value) {
    branchAnnotations_[key] = std::move(value);
}

void Node::SetBranchAnnotation(const std::string &key, const char *value) {
    branchAnnotations_[key] = std::string(value);
}

bool Node::ContainsBranchAnnotation(const string &key) const {
    return branchAnnotations_.find(key) != branchAnnotations_.end();
}

void Node::RemoveBranchAnnotation(const string &key) { branchAnnotations_.erase(key); }

bool Node::MakeBinary() {
    bool madeBinary = false;
    while (ChildCount() > 2) {
        // take the first two children and create a new node
        // with them as children
        // and add the new node as a child of the current node
        NodePtr child0 = children_.at(0);
        NodePtr child1 = children_.at(1);
        RemoveChild(child0);
        RemoveChild(child1);
        auto newNode = std::make_shared<Node>();
        newNode->AddChild(child0);
        newNode->AddChild(child1);
        newNode->SetDistance(0);
        children_.insert(children_.begin(), newNode);
        newNode->SetParent(shared_from_this());
        madeBinary = true;
    }
    return madeBinary;
}

string Node::Newick() const {
    NewickExportOptions options;
    return Newick(options);
}

std::pair<string, string> Node::MakeCommentForNewick(
    const NewickExportOptions &options) const {
    string comment =
        BuildCommentForNewick(comment_, annotations_, options, options.annotationKeys);
    string branchComment = BuildCommentForNewick(branchComment_, branchAnnotations_,
                                                 options, options.branchAnnotationKeys);
    return std::make_pair(comment, branchComment);
}

void Node::ParseComment(const std::unordered_map<string, Converter> &converters) {
    if (comment_.empty()) return;

    ParseRawComment(comment_, annotations_, converters);
}

void Node::ParseBranchComment(const std::unordered_map<string, Converter> &converters) {
    if (branchComment_.empty()) return;

    ParseRawComment(branchComment_, branchAnnotations_, converters);
}

string Node::Newick(const NewickExportOptions &options) const {
    std::ostringstream oss;
    double distance = Distance();
    auto [comment, branchComment] = MakeCommentForNewick(options);

    if (this->IsLeaf()) {
        oss << Name();
        if (options.includeBranchLengths && !std::isnan(distance)) {
            if (options.decimalPrecision > 0) {
                oss << comment << ":" << branchComment << std::fixed
                    << std::setprecision(options.decimalPrecision) << distance;
            } else {
                oss << comment << ":" << branchComment << distance;
            }
        } else {
            oss << comment;
        }
    } else {
        oss << "(";
        string newickStr;
        for (const auto &child : children_) {
            newickStr += child->Newick(options) + ",";
        }
        newickStr.pop_back();
        oss << newickStr << ")";

        if (options.includeInternalNodeName && !name_.empty()) {
            oss << name_;
        }
        if (options.includeBranchLengths && !std::isnan(distance)) {
            if (options.decimalPrecision > 0) {
                oss << comment << ":" << branchComment << std::fixed
                    << std::setprecision(options.decimalPrecision) << distance;
            } else {
                oss << comment << ":" << branchComment << distance;
            }
        } else {
            oss << comment;
        }
    }
    return oss.str();
}

void Node::ComputeDescendantBitset(size_t size) {
    if (descendantBitset_.size() != size) {
        descendantBitset_.resize(size, false);
    }

    if (IsLeaf()) {
        descendantBitset_[Id()] = true;
    } else {
        std::fill(descendantBitset_.begin(), descendantBitset_.end(), false);

        for (auto child : Children()) {
            auto &childBitset = child->DescendantBitset();
            for (size_t i = 0; i < size; i++) {
                descendantBitset_[i] = descendantBitset_[i] | childBitset[i];
            }
        }
    }
}
