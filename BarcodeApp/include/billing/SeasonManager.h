#pragma once

#include <string>
#include <vector>

// Structure representing a single seasonal discount period
struct Season {
    std::string startDate;        // Format: YYYY-MM-DD
    std::string endDate;          // Format: YYYY-MM-DD
    double discountPercentage;   // Discount percentage, e.g. 10.0 for 10%
    bool isActive;                // true if season is active, false otherwise

    // Default constructor
    Season();

    // Parameterized constructor
    Season(const std::string& start, const std::string& end, double discount, bool active);
};

// Class to manage seasonal discounts and persistence in Seasons.txt
class SeasonManager {
private:
    std::vector<Season> seasons;
    std::string filePath;

    // Internal helper to clean whitespace and trailing '#'
    static std::string cleanLine(const std::string& line);

public:
    // Constructor
    SeasonManager();
    explicit SeasonManager(const std::string& path);

    // ==========================================
    // File Persistence Functions
    // ==========================================
    // Loads seasons from Seasons.txt. Returns true on success or if file is newly created.
    bool loadSeasons();
    bool loadSeasons(const std::string& path);

    // Saves all seasons to Seasons.txt in exact 4-field format:
    // StartDate EndDate DiscountPercentage isActive#
    bool saveSeasons();
    bool saveSeasons(const std::string& path);

    // ==========================================
    // Season Management Operations
    // ==========================================
    // Adds a new season after validating dates and discount percentage
    bool addSeason(const std::string& start, const std::string& end, double discount, bool active, std::string& errorMsg);

    // Activates an existing season by index, ensuring it does not overlap with other active seasons
    bool activateSeason(int index, std::string& errorMsg);

    // Deactivates an existing season by index
    bool deactivateSeason(int index, std::string& errorMsg);

    // Edits an existing season's attributes
    bool editSeason(int index, const std::string& start, const std::string& end, double discount, bool active, std::string& errorMsg);

    // Deletes a season by index
    bool deleteSeason(int index);

    // Returns the number of loaded seasons
    int getSeasonCount() const;

    // Returns a pointer to the season at the specified index, or nullptr if out of bounds
    const Season* getSeasonAt(int index) const;
    Season* getSeasonAt(int index);

    // ==========================================
    // Automatic Seasonal Discount Retrieval (Req 15)
    // ==========================================
    // Checks today's date against all active seasons.
    // Returns the matching discount percentage (e.g. 10.0). If no active season matches, returns 0.0.
    // Optionally populates outSeasonInfo with details of the matching season.
    double getActiveSeasonDiscount(const std::string& currentDate, std::string* outSeasonInfo = nullptr) const;

    // ==========================================
    // Date and Validation Helper Functions
    // ==========================================
    // Validates that a date string matches YYYY-MM-DD format and has valid day/month/year
    static bool isValidDate(const std::string& date);

    // Compares two dates in YYYY-MM-DD format.
    // Returns -1 if date1 < date2, 0 if date1 == date2, +1 if date1 > date2
    static int compareDates(const std::string& date1, const std::string& date2);

    // Checks if checkDate is within [startDate, endDate] inclusive
    static bool isDateInRange(const std::string& checkDate, const std::string& startDate, const std::string& endDate);

    // Checks if two date ranges [s1, e1] and [s2, e2] overlap
    static bool doRangesOverlap(const std::string& s1, const std::string& e1, const std::string& s2, const std::string& e2);

    // Returns the current system date as YYYY-MM-DD
    static std::string getCurrentDate();

    // Returns the current system time as HH:MM:SS
    static std::string getCurrentTime();
};
