// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/newick.hpp"

#include <memory>
#include <string>

#include "cladokit/tree.hpp"
#include "cladokit/treeio.hpp"

using cladokit::NewickFile;
using cladokit::Node;
using cladokit::Tree;
using std::string;
using std::vector;

size_t NewickFile::Count() {
    if (count_ > 0) return count_;

    string buffer;
    while (!in_.eof()) {
        std::getline(in_, buffer, '\n');
        if (!buffer.empty() && buffer.at(0) == '(') {
            count_++;
        }
    }
    in_.clear();
    in_.seekg(0, std::ios::beg);
    return count_;
}

vector<std::shared_ptr<Tree>> NewickFile::Parse() {
    vector<std::shared_ptr<Tree>> trees;
    string buffer;
    while (!in_.eof()) {
        std::getline(in_, buffer, '\n');
        if (buffer.size() > 0 && buffer.at(0) == '(') {
            auto tree = Tree::FromNewick(buffer, taxonNames_);
            trees.push_back(tree);
        }
    }
    return trees;
}

// point to the first tree in the file
bool NewickFile::HasNext() {
    // there is a tree in currentTreeString_ so it has not been parsed
    if (currentTreeString_ != "") {
        return true;
    }
    string buffer;
    while (!in_.eof()) {
        std::getline(in_, currentTreeString_, '\n');
        if (currentTreeString_.size() > 0 && currentTreeString_.at(0) == '(') {
            return true;
        }
    }
    currentTreeString_.clear();
    return false;
}

// move to the next tree in the file
void NewickFile::SkipNext() {
    while (!in_.eof()) {
        std::getline(in_, currentTreeString_, '\n');
        if (currentTreeString_.size() > 0 && currentTreeString_.at(0) == '(') {
            return;
        }
    }

    if (in_.eof()) {
        currentTreeString_.clear();
    }
}

std::shared_ptr<Tree> NewickFile::Next() {
    std::shared_ptr<Tree> tree;
    if (HasNext()) {
        tree = Tree::FromNewick(currentTreeString_, taxonNames_);
        currentTreeString_.clear();
    }
    return tree;
}
