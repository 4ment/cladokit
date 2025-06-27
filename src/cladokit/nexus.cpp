// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#include "cladokit/nexus.hpp"

#include <map>
#include <string>
#include <vector>

#include "cladokit/tree.hpp"
#include "cladokit/utils.hpp"

using cladokit::NexusFile;
using cladokit::Node;
using cladokit::Tree;
using std::string;
using std::vector;

size_t NexusFile::Count() {
    if (count_ > 0) return count_;

    bool found = findBlock("trees");
    if (!found) {
        std::cerr << "Error: trees block not found" << std::endl;
        return 0;
    }

    string buffer;
    while (!in_.eof()) {
        std::getline(in_, buffer, '\n');
        if (StartsWithCaseInsensitive(buffer, "end;")) {
            break;
        }
        if (StartsWithCaseInsensitive(buffer, "tree")) {
            count_++;
        }
    }
    in_.clear();
    in_.seekg(0, std::ios::beg);
    return count_;
}

vector<std::shared_ptr<Tree>> NexusFile::Parse() {
    vector<std::shared_ptr<Tree>> trees;

    if (!taxonNames_->empty()) {
        for (size_t i = 0; i < taxonNames_->size(); i++) {
            taxonMap_[taxonNames_->at(i)] = i;
        }
    }

    bool found = findBlock("trees");
    if (!found) {
        std::cerr << "Error: trees block not found" << std::endl;
        return trees;
    }
    string buffer;
    while (!in_.eof()) {
        std::getline(in_, buffer, '\n');
        if (StartsWithCaseInsensitive(buffer, "end;")) {
            break;
        } else if (StartsWithCaseInsensitiveLeftTrim(buffer, "translate")) {
            ParseTranslate();
        } else if (StartsWithCaseInsensitive(buffer, "tree")) {
            auto tree = ParseTreeLine(buffer);
            trees.push_back(tree);
        }
    }
    return trees;
}

std::shared_ptr<Tree> NexusFile::ParseTreeLine(const string &line) {
    size_t start = 4;
    // Go to the first '(' of the newick tree
    // while avoiding comments at the beginning.
    // For example: tree STATE_1 = [&R] (A,B);
    while (line[start] != '(') {
        if (line[start] == '[') {
            while (line[start] != ']') {
                start++;
            }
        }
        start++;
    }
    // We don't want comments at the end either.
    size_t end = line.size() - 1;
    while (line[end] != ';') {
        if (line[end] == ']') {
            while (line[end] != '[') {
                end--;
            }
        }
        end--;
    }

    string newick = line.substr(start, end - start + 1);

    if (!translateMap_.empty()) {
        // provide empty taxon names because at this stage the taxa in the newick tree
        // are just numbers.
        auto emptyTaxonNames = std::make_shared<std::vector<std::string>>();
        auto tree = Tree::FromNewick(newick, emptyTaxonNames);
        for (auto it = tree->Root()->begin_postorder();
             it != tree->Root()->end_postorder(); ++it) {
            auto node = *it;
            if (node->IsLeaf()) {
                // Translate the name if there was translate block
                // For example: "2" -> "sequencexyz"
                auto it2 = translateMap_.find(node->Name());
                if (it2 != translateMap_.end()) {
                    node->SetName(it2->second);
                }
                auto it3 = taxonMap_.find(node->Name());
                // change ID of node using its index in taxonMap
                if (it3 != taxonMap_.end()) {
                    node->SetId(it3->second);
                } else {  // if it is not found in taxonMap, add it.
                    taxonMap_[node->Name()] = node->Id();
                }
            }
        }
        if (!taxonMap_.empty() && taxonNames_->empty()) {
            taxonNames_->resize(taxonMap_.size());
            for (const auto &[name, index] : taxonMap_) {
                (*taxonNames_)[index] = name;
            }
        }
        tree->SetTaxonNames(taxonNames_);
        return tree;
    } else {
        return Tree::FromNewick(newick, taxonNames_);
    }
}

void NexusFile::PointToFirstTree() {
    if (translateParsed_ == false) {
        if (!taxonNames_->empty()) {
            for (size_t i = 0; i < taxonNames_->size(); i++) {
                taxonMap_[taxonNames_->at(i)] = i;
            }
        }

        bool found = findBlock("trees");
        if (!found) {
            std::cerr << "Error: trees block not found" << std::endl;
        } else {
            string buffer;
            while (!in_.eof()) {
                std::getline(in_, buffer, '\n');
                if (StartsWithCaseInsensitive(buffer, "end;")) {
                    // should not be here
                    break;
                } else if (StartsWithCaseInsensitiveLeftTrim(buffer, "translate")) {
                    ParseTranslate();
                } else if (StartsWithCaseInsensitive(buffer, "tree")) {
                    currentTreeString_ = buffer;
                    break;
                }
            }
        }
    }
    translateParsed_ = true;
}

bool NexusFile::HasNext() {
    // there is a tree in currentTreeString_ so it has not been parsed
    if (currentTreeString_ != "") {
        return true;
    }
    // If translateParsed_ is false, we need to initialize the parser
    // and parse the first tree line
    if (translateParsed_ == false) {
        PointToFirstTree();
        return currentTreeString_ != "";
    }

    while (!in_.eof()) {
        std::getline(in_, currentTreeString_, '\n');
        if (cladokit::StartsWithCaseInsensitive(currentTreeString_, "end;")) {
            break;
        } else if (cladokit::StartsWithCaseInsensitive(currentTreeString_, "tree")) {
            return true;
        }
    }
    currentTreeString_.clear();
    return false;
}

void NexusFile::SkipNext() {
    if (HasNext()) {
        while (!in_.eof()) {
            std::getline(in_, currentTreeString_, '\n');
            if (cladokit::StartsWithCaseInsensitive(currentTreeString_, "tree")) {
                return;
            }
        }
    }

    if (in_.eof()) {
        currentTreeString_.clear();
    }
}

std::shared_ptr<Tree> NexusFile::Next() {
    std::shared_ptr<Tree> tree;
    if (HasNext()) {
        tree = ParseTreeLine(currentTreeString_);
        currentTreeString_.clear();
    }
    return tree;
}

void NexusFile::ParseTranslate() {
    bool done = false;
    string line;
    while (!in_.eof() && !done) {
        line = nextLineUncommented();
        RightTrim(line);
        LeftTrim(line);

        if (line == ";") break;

        size_t found = line.find(";");

        if (found != string::npos) {
            line = line.substr(0, found);
            done = true;
        }

        found = line.find(",");

        if (found == string::npos) {
            size_t index = line.find_first_of("\t ");
            string shorthand = line.substr(0, index);
            line.erase(0, index + 1);
            translateMap_[shorthand] = line;
        } else {
            std::size_t start = 0;

            while (found != string::npos) {
                string temp = line.substr(start, found);
                if (temp.size() != 0) {
                    std::size_t index = temp.find_first_of("\t ");
                    string shorthand = temp.substr(0, index);
                    temp.erase(0, index + 1);
                    RightTrim(temp);
                    LeftTrim(temp);
                    translateMap_[shorthand] = temp;
                }
                start = found + 1;
                found = line.find(",", start);
            }
        }
    }
}

// it can return an empty string if it is a (single or multiple line) comment
// with nothing around assumes one comment per line assumes it is not eof
string NexusFile::nextLineUncommented() {
    string buffer;

    getline(in_, buffer, '\n');

    std::size_t indexStart = buffer.find_first_of("[");

    if (indexStart == std::string::npos) {
        return buffer;
    }

    string line = buffer;

    std::size_t indexClose = buffer.find_first_of("]");

    // single line comment
    if (indexClose != std::string::npos) {
        line.erase(indexStart, indexClose - indexStart);
    } else {  // multiple line comment
        while (indexClose == std::string::npos) {
            getline(in_, buffer, '\n');
            indexClose = buffer.find_first_of("]");
        }
        buffer.erase(0, indexClose);
        line += buffer;
    }

    return line;
}

bool NexusFile::findBlock(const string &blockName) {
    bool found = false;
    string buffer;
    while (!in_.eof()) {
        getline(in_, buffer, '\n');
        if (StartsWithCaseInsensitive(buffer, "begin " + blockName)) {
            found = true;
            break;
        }
    }
    return found;
}
