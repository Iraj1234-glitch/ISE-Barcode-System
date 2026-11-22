#pragma once
#include <SFML/Graphics.hpp>
#include "ProductCatalog.h"
#include "OrderManager.h"

class ReportManager {
public:
    ReportManager(const ProductCatalog* pc, const OrderManager* om);

    // Call this from MainWindow::drawCurrentPage()
    void draw(sf::RenderWindow& target);

    // Navigation between the six reports
    void nextReport();
    void prevReport();

private:
    const ProductCatalog* catalog_;
    const OrderManager* orderMgr_;

    enum ReportId {
        INCOMPLETE_ORDERS = 0,
        COMPLETE_ORDERS,
        DAILY_SALES,
        WEEKLY_SALES,
        MONTHLY_SALES,
        YEARLY_SALES,
        CUSTOM_PERIOD,
        TOTAL_SALES_DISCOUNTS,
        TOP10_SOLD,
        TOP10_STOCK
    };
    ReportId current_ = INCOMPLETE_ORDERS;

    sf::Font font_;
    sf::Text header_;
    std::vector<sf::Text> rows_;

    void rebuild();               // recompute rows_ for the current report
    std::string formatCurrency(double v) const;
};
