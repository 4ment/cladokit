// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <any>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cladokit/newick_options.hpp"
#include "cladokit/utils.hpp"

namespace cladokit {
class Node : public std::enable_shared_from_this<Node> {
   public:
    using NodePtr = std::shared_ptr<Node>;

    Node();

    explicit Node(const std::string &name);

    const std::string &Name() const;

    void SetName(const std::string &name);

    size_t Id() const;

    void SetId(size_t id);

    double Distance() const;

    void SetDistance(double distance);

    size_t ChildCount() const { return children_.size(); }

    NodePtr ChildAt(size_t index) const { return children_.at(index); }

    const std::vector<NodePtr> &Children() const { return children_; }

    std::vector<NodePtr> Siblings() const;

    bool AddChild(const NodePtr &node);

    bool RemoveChild(NodePtr node);

    void RemoveParent() { parent_.reset(); }

    void SetParent(NodePtr parent) { parent_ = parent; }

    NodePtr Parent() const { return parent_.lock(); }

    bool IsRoot() const;

    bool IsLeaf() const;

    void Collapse();

    std::vector<std::string> AnnotationKeys() const;

    bool ContainsAnnotation(const std::string &key) const;

    void SetAnnotation(const std::string &key, std::any value);

    void SetAnnotation(const std::string &key, const char *value);

    std::any Annotation(const std::string &key) const;

    template <typename T>
    T Annotation(const std::string &key) const {
        auto it = annotations_.find(key);
        if (it == annotations_.end()) {
            throw std::out_of_range("Key not found: " + key);
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast &) {
            throw std::runtime_error("Type mismatch for key: " + key);
        }
    }

    void RemoveAnnotation(const std::string &key);

    void SetComment(const std::string &comment) { comment_ = comment; }

    const std::string &Comment() const { return comment_; }

    std::vector<std::string> BranchAnnotationKeys() const;

    bool ContainsBranchAnnotation(const std::string &key) const;

    void SetBranchAnnotation(const std::string &key, std::any value);

    void SetBranchAnnotation(const std::string &key, const char *value);

    std::any BranchAnnotation(const std::string &key) const;

    template <typename T>
    T BranchAnnotation(const std::string &key) const {
        auto it = branchAnnotations_.find(key);
        if (it == branchAnnotations_.end()) {
            throw std::out_of_range("Key not found: " + key);
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast &) {
            throw std::runtime_error("Type mismatch for key: " + key);
        }
    }

    void SetBranchComment(const std::string &branchComment) {
        branchComment_ = branchComment;
    }

    const std::string &BranchComment() const { return branchComment_; }

    void RemoveBranchAnnotation(const std::string &key);

    bool MakeBinary();

    bool IsBinary() const { return children_.size() == 2; }

    std::string Newick() const;

    std::string Newick(const NewickExportOptions &options) const;

    void ParseComment(
        const std::unordered_map<std::string, cladokit::Converter> &converters);

    void ParseBranchComment(
        const std::unordered_map<std::string, cladokit::Converter> &converters);

    void ComputeDescendantBitset(size_t size);

    const std::vector<bool> &DescendantBitset() const { return descendantBitset_; }

    class PostOrderIterator;
    class PreOrderIterator;

    PostOrderIterator begin_postorder();
    PostOrderIterator end_postorder();
    PreOrderIterator begin_preorder();
    PreOrderIterator end_preorder();

   private:
    std::string name_;
    size_t id_ = 0;
    std::weak_ptr<Node> parent_;
    std::vector<NodePtr> children_;
    double distance_ = std::numeric_limits<double>::quiet_NaN();
    std::map<std::string, std::any> annotations_;
    std::map<std::string, std::any> branchAnnotations_;
    std::string comment_;  // raw comment extraced from newick file
    std::string branchComment_;
    std::vector<bool> descendantBitset_;

    std::pair<std::string, std::string> MakeCommentForNewick(
        const NewickExportOptions &options) const;
};

class Node::PostOrderIterator {
   public:
    explicit PostOrderIterator(NodePtr root = nullptr) {
        if (root) push_left(root);
    }

    NodePtr operator*() const { return stack_.top().node; }

    PostOrderIterator &operator++() {
        Frame f = stack_.top();
        stack_.pop();
        if (!stack_.empty()) {
            Frame &parent = stack_.top();
            if (++parent.index < parent.node->children_.size()) {
                push_left(parent.node->children_[parent.index]);
            }
        }
        return *this;
    }

    bool operator==(const PostOrderIterator &other) const {
        return stack_ == other.stack_;
    }
    bool operator!=(const PostOrderIterator &other) const { return !(*this == other); }

   private:
    struct Frame {
        NodePtr node;
        size_t index = 0;

        bool operator==(const Frame &other) const {
            return node == other.node && index == other.index;
        }
    };

    std::stack<Frame> stack_;

    void push_left(NodePtr node) {
        while (node && !node->children_.empty()) {
            stack_.push({node, 0});
            node = node->children_[0];
        }
        if (node) stack_.push({node, 0});
    }
};

class Node::PreOrderIterator {
   public:
    explicit PreOrderIterator(NodePtr root = nullptr) {
        if (root) stack_.push(root);
    }

    NodePtr operator*() const { return stack_.top(); }

    PreOrderIterator &operator++() {
        if (stack_.empty()) return *this;

        NodePtr current = stack_.top();
        stack_.pop();

        // Push children in reverse order so left child is processed first
        const auto &children = current->children_;
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            stack_.push(*it);
        }

        return *this;
    }

    bool operator==(const PreOrderIterator &other) const {
        return stack_ == other.stack_;
    }

    bool operator!=(const PreOrderIterator &other) const { return !(*this == other); }

   private:
    std::stack<NodePtr> stack_;
};

inline Node::PostOrderIterator Node::begin_postorder() {
    return PostOrderIterator(shared_from_this());
}
inline Node::PostOrderIterator Node::end_postorder() { return PostOrderIterator(); }

inline Node::PreOrderIterator Node::begin_preorder() {
    return PreOrderIterator(shared_from_this());
}
inline Node::PreOrderIterator Node::end_preorder() { return PreOrderIterator(); }
}  // namespace cladokit
