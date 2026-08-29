#pragma once

#include "ContributionPackage.h"

#include <string>
#include <vector>

namespace ContributionPackage {

// --- Package parsing ---
bool parsePackage(const std::string& text,
                  const std::string& sourcePath,
                  Preview& preview);

// --- File I/O helpers ---
std::string readFile(const std::string& path, std::string& error);
std::string defaultRootDirectory();
std::string packageFileName(const std::string& packageId);

// --- Validation ---
bool validateIdText(const std::string& text, const std::string& label, std::string& error);

// --- JSON serialization helpers ---
std::string quoted(const std::string& text);
std::string lowerCopy(std::string text);
void appendJsonStringArray(std::ostringstream& out, const std::vector<std::string>& values);
void appendPackageSummary(std::ostringstream& out, const Package& package, bool includeItems);

} // namespace ContributionPackage
