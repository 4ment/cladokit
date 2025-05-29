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

string Node::MakeCommentForNewick(const NewickExportOptions &options) const {
    string comment;
    if (options.includeRawComment && !comment_.empty()) {
        comment = comment_;
    } else if (!options.annotationKeys.empty()) {
        comment = "[&";
        for (const auto &key : options.annotationKeys) {
            auto it = annotations_.find(key);
            if (it != annotations_.end()) {
                auto val = it->second;
                comment += key + "=";

                if (val.type() == typeid(double)) {
                    comment += std::to_string(std::any_cast<double>(val));
                } else if (val.type() == typeid(int)) {
                    comment += std::to_string(std::any_cast<int>(val));
                } else if (val.type() == typeid(std::string)) {
                    comment += std::any_cast<std::string>(val);
                } else {
                    std::cerr << "Warning: Unsupported type for annotation key '" << key
                              << "'. Only double, int, and string are supported. "
                              << val.type().name() << std::endl;
                    continue;  // Skip unsupported types
                }
                comment += ",";
            }
        }
        comment.pop_back();
        comment += "]";
        if (comment == "[]") {
            comment.clear();
        }
    }
    return comment;
}
void Node::ParseRawComment(const std::unordered_map<string, Converter> &converters) {
    if (comment_.empty()) return;

    ParseComment(comment_, annotations_, converters);
}

string Node::Newick(const NewickExportOptions &options) const {
    std::ostringstream oss;
    if (this->IsLeaf()) {
        oss << Name();
        string comment = MakeCommentForNewick(options);

        if (options.includeBranchLengths) {
            if (options.decimalPrecision > 0) {
                oss << ":" << comment << std::fixed
                    << std::setprecision(options.decimalPrecision) << Distance();
            } else {
                oss << ":" << comment << Distance();
            }
        } else {
            oss << comment;
        }
    } else {
        oss << "(";
        string comment = MakeCommentForNewick(options);
        string newickStr;
        for (const auto &child : children_) {
            newickStr += child->Newick(options) + ",";
        }
        newickStr.pop_back();
        oss << newickStr << ")";

        if (options.includeInternalNodeName && !name_.empty()) {
            oss << name_;
        }
        if (options.includeBranchLengths && !IsRoot()) {
            if (options.decimalPrecision > 0) {
                oss << ":" << comment << std::fixed
                    << std::setprecision(options.decimalPrecision) << Distance();
            } else {
                oss << ":" << comment << Distance();
            }
        } else {
            oss << comment;
        }
    }
    return oss.str();
}
