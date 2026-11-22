#include "ReportManager.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

// Helper to create a text cell at a given position.
static sf::Text makeCell(const sf::Font& f, const std::string& txt,
                         float x, float y, unsigned size = 20)
{
    sf::Text t; t.setFont(f); t.setString(txt); t.setCharacterSize(size);
    t.setFillColor(sf::Color::White);
    t.setPosition(x, y);
    return t;
}

ReportManager::ReportManager(const ProductCatalog* pc,
                             const OrderManager* om)
    : catalog_(pc), orderMgr_(om)
{
    if (!font_.loadFromFile("assets/arial.ttf")) {
        // fallback – default font will be used automatically
    }
    header_.setFont(font_);
    header_.setCharacterSize(28);
    header_.setFillColor(sf::Color::White);
    header_.setPosition(220.f, 80.f);
    rebuild();
}

void ReportManager::nextReport()
{
    current_ = static_cast<ReportId>((current_ + 1) % 10);
    rebuild();
}
void ReportManager::prevReport()
{
    current_ = static_cast<ReportId>((current_ + 9) % 10);
    rebuild();
}

void ReportManager::draw(sf::RenderWindow& target)
{
    target.clear(sf::Color(15, 15, 30));
    target.draw(header_);
    for (auto& txt : rows_)
        target.draw(txt);

    // navigation hint
    sf::Text hint;
    hint.setFont(font_);
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(180, 180, 180));
    hint.setString("< Left  |  Right > to change report");
    hint.setPosition(250.f, target.getSize().y - 40.f);
    target.draw(hint);
}

std::string ReportManager::formatCurrency(double v) const
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << "$" << v;
    return ss.str();
}

// --------------------------------------------------------------
// Rebuild the current report into `rows_`
// --------------------------------------------------------------
void ReportManager::rebuild()
{
    rows_.clear();

    switch (current_) {
    case INCOMPLETE_ORDERS: {
        header_.setString("Incomplete Orders");
        auto vec = orderMgr_->getIncompleteOrders();   // vector<OrderInfo>
        float y = 130.f;
        rows_.push_back(makeCell(font_, "OrderID   Customer   Items   Sub‑total", 240.f, y, 22));
        y += 30.f;
        for (const auto& o : vec) {
            std::ostringstream ss;
            ss << o.id << "   " << o.customerName << "   " << o.itemCount
               << "   " << formatCurrency(o.subtotal);
            rows_.push_back(makeCell(font_, ss.str(), 240.f, y));
            y += 25.f;
        }
        break;
    }
    case COMPLETE_ORDERS: {
        header_.setString("Completed Orders");
        auto vec = orderMgr_->getCompletedOrders();
        float y = 130.f;
        rows_.push_back(makeCell(font_, "OrderID   Customer   Items   Total", 240.f, y, 22));
        y += 30.f;
        for (const auto& o : vec) {
            std::ostringstream ss;
            ss << o.id << "   " << o.customerName << "   " << o.itemCount
               << "   " << formatCurrency(o.total);
            rows_.push_back(makeCell(font_, ss.str(), 240.f, y));
            y += 25.f;
        }
        break;
    }
    case DAILY_SALES: case WEEKLY_SALES: case MONTHLY_SALES:
    case YEARLY_SALES: case CUSTOM_PERIOD: {
        std::string period;
        std::chrono::system_clock::time_point from, to = std::chrono::system_clock::now();
        if (current_ == DAILY_SALES)   { period = "Daily";   from = to - std::chrono::hours(24); }
        else if (current_ == WEEKLY_SALES)  { period = "Weekly";  from = to - std::chrono::hours(24*7); }
        else if (current_ == MONTHLY_SALES) { period = "Monthly"; from = to - std::chrono::hours(24*30); }
        else if (current_ == YEARLY_SALES)  { period = "Yearly";  from = to - std::chrono::hours(24*365); }
        else { period = "Custom"; from = to - std::chrono::hours(24); }   // placeholder

        header_.setString(period + " Cash Sales");
        double cashSum = orderMgr_->cashSalesBetween(from, to);
        rows_.push_back(makeCell(font_, "Total cash sales: " + formatCurrency(cashSum), 240.f, 150.f));
        break;
    }
    case TOTAL_SALES_DISCOUNTS: {
        header_.setString("Total Sales & Discounts");
        double gross = orderMgr_->totalSales();
        double disc  = orderMgr_->totalDiscounts();
        rows_.push_back(makeCell(font_, "Gross sales: " + formatCurrency(gross), 240.f, 150.f));
        rows_.push_back(makeCell(font_, "Total discounts: " + formatCurrency(disc), 240.f, 180.f));
        rows_.push_back(makeCell(font_, "Net sales: " + formatCurrency(gross - disc), 240.f, 210.f));
        break;
    }
    case TOP10_SOLD: {
        header_.setString("Top 10 Most Sold Items");
        auto vec = catalog_->topNSold(10);          // vector<ReportInfo>
        float y = 130.f;
        rows_.push_back(makeCell(font_, "Rank   Item   Sold   Stock", 240.f, y, 22));
        y += 30.f; int rank = 1;
        for (const auto& info : vec) {
            std::ostringstream ss;
            ss << rank << "   " << info.name << "   " << info.sold << "   " << info.stock;
            rows_.push_back(makeCell(font_, ss.str(), 240.f, y));
            y += 25.f; ++rank;
        }
        break;
    }
    case TOP10_STOCK: {
        header_.setString("Top 10 Items With Highest Stock");
        auto vec = catalog_->topNStock(10);
        float y = 130.f;
        rows_.push_back(makeCell(font_, "Rank   Item   Stock   Sold", 240.f, y, 22));
        y += 30.f; int rank = 1;
        for (const auto& info : vec) {
            std::ostringstream ss;
            ss << rank << "   " << info.name << "   " << info.stock << "   " << info.sold;
            rows_.push_back(makeCell(font_, ss.str(), 240.f, y));
            y += 25.f; ++rank;
        }
        break;
    }
    }
}
