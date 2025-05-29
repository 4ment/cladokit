// Copyright 2025 Mathieu Fourment
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace cladokit {
struct NewickExportOptions {
    bool includeInternalNodeName = false;
    bool includeBranchLengths = true;
    bool includeRawComment = false;
    std::vector<std::string> annotationKeys;
    int decimalPrecision = -1;  // -1 means no precision limit

    NewickExportOptions() = default;

    NewickExportOptions(bool internalNames, bool branchLengths, bool rawComment,
                        std::vector<std::string> keys = {})
        : includeInternalNodeName(internalNames),
          includeBranchLengths(branchLengths),
          includeRawComment(rawComment),
          annotationKeys(std::move(keys)) {}
};
}  // namespace cladokit
