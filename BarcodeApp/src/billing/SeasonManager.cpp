#include "../../include/billing/SeasonManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>

// ==========================================
// Season Structure Implementation
// ==========================================

Season::Season() {
    startDate = "";
    endDate = "";
    discountPercentage = 0.0;
    isActive = false;
}

Season::Season(const std::string& start, const std::string& end, double discount, bool active) {
    startDate = start;
    endDate = end;
    discountPercentage = discount;
    isActive = active;
}

// ==========================================
// SeasonManager Class Implementation
// ==========================================

SeasonManager::SeasonManager() {
    filePath = "Seasons.txt";
    loadSeasons();
}

SeasonManager::SeasonManager(const std::string& path) {
    filePath = path;
    loadSeasons();
}

// Strips whitespace and trailing '#' delimiter
std::string SeasonManager::cleanLine(const std::string& line) {
    if (line.empty()) {
        return "";
    }

    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }

    size_t end = line.find_last_not_of(" \t\r\n");
    std::string trimmed = line.substr(start, (end - start + 1));

    // Remove trailing '#' if present
    if (!trimmed.empty() && trimmed.back() == '#') {
        trimmed.pop_back();
    }

    // Trim again after removing '#'
    start = trimmed.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    end = trimmed.find_last_not_of(" \t\r\n");
    return trimmed.substr(start, (end - start + 1));
}

// Loads seasons from Seasons.txt
// Format: StartDate EndDate DiscountPercentage isActive#
// Time Complexity: O(N) where N is number of lines in Seasons.txt
bool SeasonManager::loadSeasons() {
    return loadSeasons(filePath);
}

bool SeasonManager::loadSeasons(const std::string& path) {
    filePath = path;
    seasons.clear();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Safe handling for missing file: create an empty file so it is ready
        std::ofstream createFile(filePath);
        createFile.close();
        return true;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue; // Skip blank lines safely
        }

        std::stringstream ss(cleaned);
        std::string sDate = "";
        std::string eDate = "";
        double discount = 0.0;
        std::string activeStr = "";

        ss >> sDate >> eDate >> discount >> activeStr;

        // Ensure all 4 fields are present and valid
        if (sDate.empty() || eDate.empty() || activeStr.empty()) {
            continue; // Skip malformed lines safely
        }

        if (!isValidDate(sDate) || !isValidDate(eDate)) {
            continue;
        }

        if (discount < 0.0 || discount > 100.0) {
            continue;
        }

        bool active = (activeStr == "true" || activeStr == "1");

        seasons.push_back(Season(sDate, eDate, discount, active));
    }

    file.close();
    return true;
}

// Saves all seasons to Seasons.txt
// Format: StartDate EndDate DiscountPercentage isActive#
// Time Complexity: O(N) where N is number of seasons
bool SeasonManager::saveSeasons() {
    return saveSeasons(filePath);
}

bool SeasonManager::saveSeasons(const std::string& path) {
    filePath = path;

    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filePath << " for writing.\n";
        return false;
    }

    for (size_t i = 0; i < seasons.size(); ++i) {
        file << seasons[i].startDate << " "
             << seasons[i].endDate << " "
             << std::fixed << std::setprecision(2) << seasons[i].discountPercentage << " "
             << (seasons[i].isActive ? "true" : "false")
             << "#\n";
    }

    file.close();
    return true;
}

// Adds a new season after validation
bool SeasonManager::addSeason(const std::string& start, const std::string& end,
                              double discount, bool active, std::string& errorMsg) {
    // 1. Validate start date
    if (!isValidDate(start)) {
        errorMsg = "Invalid start date format. Must be YYYY-MM-DD.";
        return false;
    }

    // 2. Validate end date
    if (!isValidDate(end)) {
        errorMsg = "Invalid end date format. Must be YYYY-MM-DD.";
        return false;
    }

    // 3. Ensure start date is not after end date
    if (compareDates(start, end) > 0) {
        errorMsg = "Start date cannot be after end date.";
        return false;
    }

    // 4. Validate discount percentage (0% to 100%)
    if (discount < 0.0 || discount > 100.0) {
        errorMsg = "Discount percentage must be between 0.0 and 100.0%.";
        return false;
    }

    // 5. If active, check for overlapping active seasons
    if (active) {
        for (size_t i = 0; i < seasons.size(); ++i) {
            if (seasons[i].isActive && doRangesOverlap(start, end, seasons[i].startDate, seasons[i].endDate)) {
                errorMsg = "Active season overlaps with existing active season (" +
                           seasons[i].startDate + " to " + seasons[i].endDate + ").";
                return false;
            }
        }
    }

    // 6. Add and save
    seasons.push_back(Season(start, end, discount, active));
    if (!saveSeasons()) {
        errorMsg = "Failed to write season to file.";
        seasons.pop_back();
        return false;
    }

    errorMsg = "Season added successfully.";
    return true;
}

// Activates an inactive season, preventing overlap with existing active seasons
bool SeasonManager::activateSeason(int index, std::string& errorMsg) {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        errorMsg = "Selected season index is invalid.";
        return false;
    }

    if (seasons[index].isActive) {
        errorMsg = "Season is already active.";
        return true;
    }

    // Check overlap with any other active season
    for (size_t i = 0; i < seasons.size(); ++i) {
        if (static_cast<int>(i) != index && seasons[i].isActive) {
            if (doRangesOverlap(seasons[index].startDate, seasons[index].endDate,
                                seasons[i].startDate, seasons[i].endDate)) {
                errorMsg = "Cannot activate: overlaps with active season (" +
                           seasons[i].startDate + " to " + seasons[i].endDate + ").";
                return false;
            }
        }
    }

    seasons[index].isActive = true;
    if (!saveSeasons()) {
        seasons[index].isActive = false;
        errorMsg = "Failed to save activated season to file.";
        return false;
    }

    errorMsg = "Season activated successfully.";
    return true;
}

// Deactivates an active season
bool SeasonManager::deactivateSeason(int index, std::string& errorMsg) {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        errorMsg = "Selected season index is invalid.";
        return false;
    }

    seasons[index].isActive = false;
    if (!saveSeasons()) {
        seasons[index].isActive = true;
        errorMsg = "Failed to save deactivated season to file.";
        return false;
    }

    errorMsg = "Season deactivated successfully.";
    return true;
}

// Edits an existing season
bool SeasonManager::editSeason(int index, const std::string& start, const std::string& end,
                               double discount, bool active, std::string& errorMsg) {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        errorMsg = "Selected season index is invalid.";
        return false;
    }

    if (!isValidDate(start) || !isValidDate(end)) {
        errorMsg = "Invalid date format. Must be YYYY-MM-DD.";
        return false;
    }

    if (compareDates(start, end) > 0) {
        errorMsg = "Start date cannot be after end date.";
        return false;
    }

    if (discount < 0.0 || discount > 100.0) {
        errorMsg = "Discount percentage must be between 0.0 and 100.0%.";
        return false;
    }

    // Check overlap if active
    if (active) {
        for (size_t i = 0; i < seasons.size(); ++i) {
            if (static_cast<int>(i) != index && seasons[i].isActive) {
                if (doRangesOverlap(start, end, seasons[i].startDate, seasons[i].endDate)) {
                    errorMsg = "Active season overlaps with existing active season (" +
                               seasons[i].startDate + " to " + seasons[i].endDate + ").";
                    return false;
                }
            }
        }
    }

    // Store backup in case save fails
    Season oldSeason = seasons[index];
    seasons[index] = Season(start, end, discount, active);

    if (!saveSeasons()) {
        seasons[index] = oldSeason;
        errorMsg = "Failed to save edited season to file.";
        return false;
    }

    errorMsg = "Season updated successfully.";
    return true;
}

// Deletes a season
bool SeasonManager::deleteSeason(int index) {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        return false;
    }

    seasons.erase(seasons.begin() + index);
    return saveSeasons();
}

int SeasonManager::getSeasonCount() const {
    return static_cast<int>(seasons.size());
}

const Season* SeasonManager::getSeasonAt(int index) const {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        return nullptr;
    }
    return &seasons[index];
}

Season* SeasonManager::getSeasonAt(int index) {
    if (index < 0 || index >= static_cast<int>(seasons.size())) {
        return nullptr;
    }
    return &seasons[index];
}

// Automatically retrieves active seasonal discount based on current date (Requirement 15)
// Time Complexity: O(N) where N is number of seasons
double SeasonManager::getActiveSeasonDiscount(const std::string& currentDate, std::string* outSeasonInfo) const {
    // Loop through all saved seasons
    for (size_t i = 0; i < seasons.size(); ++i) {
        // Ignore inactive seasons
        if (!seasons[i].isActive) {
            continue;
        }

        // Check if currentDate is within [startDate, endDate] inclusive
        if (isDateInRange(currentDate, seasons[i].startDate, seasons[i].endDate)) {
            if (outSeasonInfo != nullptr) {
                std::stringstream ss;
                ss << "Active Season: " << seasons[i].startDate << " to " << seasons[i].endDate
                   << " (" << std::fixed << std::setprecision(1) << seasons[i].discountPercentage << "% off)";
                *outSeasonInfo = ss.str();
            }
            return seasons[i].discountPercentage;
        }
    }

    if (outSeasonInfo != nullptr) {
        *outSeasonInfo = "No active seasonal discount applicable today.";
    }
    return 0.0;
}

// ==========================================
// Date Helper Functions
// ==========================================

// Validates YYYY-MM-DD
bool SeasonManager::isValidDate(const std::string& date) {
    // Must be exactly 10 characters: YYYY-MM-DD
    if (date.length() != 10) {
        return false;
    }

    if (date[4] != '-' || date[7] != '-') {
        return false;
    }

    // Check all characters are digits except dashes
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) {
            return false;
        }
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (year < 2000 || year > 2100) return false;
    if (month < 1 || month > 12) return false;

    // Number of days in each month
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    // Leap year check
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap) {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1]) {
        return false;
    }

    return true;
}

// Compares two dates: returns -1 if d1 < d2, 0 if equal, +1 if d1 > d2
int SeasonManager::compareDates(const std::string& d1, const std::string& d2) {
    int y1 = std::stoi(d1.substr(0, 4));
    int m1 = std::stoi(d1.substr(5, 2));
    int day1 = std::stoi(d1.substr(8, 2));

    int y2 = std::stoi(d2.substr(0, 4));
    int m2 = std::stoi(d2.substr(5, 2));
    int day2 = std::stoi(d2.substr(8, 2));

    if (y1 < y2) return -1;
    if (y1 > y2) return 1;

    if (m1 < m2) return -1;
    if (m1 > m2) return 1;

    if (day1 < day2) return -1;
    if (day1 > day2) return 1;

    return 0;
}

// Checks if checkDate is within [startDate, endDate] inclusive
bool SeasonManager::isDateInRange(const std::string& checkDate, const std::string& startDate, const std::string& endDate) {
    if (!isValidDate(checkDate) || !isValidDate(startDate) || !isValidDate(endDate)) {
        return false;
    }

    int cmpStart = compareDates(checkDate, startDate);
    int cmpEnd = compareDates(checkDate, endDate);

    // checkDate >= startDate AND checkDate <= endDate
    return (cmpStart >= 0 && cmpEnd <= 0);
}

// Checks if two date ranges [s1, e1] and [s2, e2] overlap
bool SeasonManager::doRangesOverlap(const std::string& s1, const std::string& e1,
                                    const std::string& s2, const std::string& e2) {
    // Overlap occurs if NOT (e1 < s2 OR e2 < s1)
    if (compareDates(e1, s2) < 0) return false;
    if (compareDates(e2, s1) < 0) return false;
    return true;
}

// Returns local system date formatted as YYYY-MM-DD
std::string SeasonManager::getCurrentDate() {
    std::time_t t = std::time(nullptr);
    std::tm nowLocal;
#if defined(_WIN32)
    localtime_s(&nowLocal, &t);
#else
    nowLocal = *std::localtime(&t);
#endif

    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(4) << (nowLocal.tm_year + 1900) << "-"
       << std::setw(2) << (nowLocal.tm_mon + 1) << "-"
       << std::setw(2) << nowLocal.tm_mday;

    return ss.str();
}

// Returns local system time formatted as HH:MM:SS
std::string SeasonManager::getCurrentTime() {
    std::time_t t = std::time(nullptr);
    std::tm nowLocal;
#if defined(_WIN32)
    localtime_s(&nowLocal, &t);
#else
    nowLocal = *std::localtime(&t);
#endif

    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << nowLocal.tm_hour << ":"
       << std::setw(2) << nowLocal.tm_min << ":"
       << std::setw(2) << nowLocal.tm_sec;

    return ss.str();
}
