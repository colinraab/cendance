#pragma once

#include "ProjectIO.h"

#include <string>

namespace ProjectIO {

bool loadProjectFile(const std::string& path,
                     ProjectSnapshot& outSnapshot,
                     std::string& error);

bool saveProjectFile(const ProjectSnapshot& snapshot,
                     const std::string& path,
                     std::string& error);

} // namespace ProjectIO
