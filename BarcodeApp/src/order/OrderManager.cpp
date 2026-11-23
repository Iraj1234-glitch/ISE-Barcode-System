#include "../../include/order/OrderManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <ctime>

// ==========================================
// OrderDetail Implementation
// ==========================================

OrderDetail::OrderDetail() {
    orderId = "";
    productId = "";
    productBarcode = "";
    quantity = 0;
}

OrderDetail::OrderDetail(const std::string& oId, const std::string& pId,
                         const std::string& barcode, int qty) {
    orderId = oId;
    productId = pId;
    productBarcode = barcode;
    quantity = qty;
}

std::string OrderDetail::getOrderId() const {
    return orderId;
}

std::string OrderDetail::getProductId() const {
    return productId;
}

std::string OrderDetail::getProductBarcode() const {
    return productBarcode;
}

int OrderDetail::getQuantity() const {
    return quantity;
}

void OrderDetail::setOrderId(const std::string& id) {
    orderId = id;
}

void OrderDetail::setProductId(const std::string& id) {
    productId = id;
}

void OrderDetail::setProductBarcode(const std::string& barcode) {
    productBarcode = barcode;
}

void OrderDetail::setQuantity(int qty) {
    quantity = qty;
}


// ==========================================
// Order Implementation
// ==========================================

Order::Order() {
    orderId = "";
    orderDate = "";
    orderTime = "";
    customerName = "";
    customerContact = "";
    orderStatus = "in-progress";
    paymentAmount = 0.0;
    paymentMode = "cash";
    paymentMerchantName = "None";
    paymentDate = "";
    detailCount = 0;
}

Order::Order(const std::string& id, const std::string& date, const std::string& time,
             const std::string& custName, const std::string& custContact,
             const std::string& status, double amount,
             const std::string& payMode, const std::string& merchant,
             const std::string& payDate) {
    orderId = id;
    orderDate = date;
    orderTime = time;
    customerName = custName;
    customerContact = custContact;
    orderStatus = status;
    paymentAmount = amount;
    paymentMode = payMode;
    paymentMerchantName = merchant;
    paymentDate = payDate;
    detailCount = 0;
}

std::string Order::getOrderId() const {
    return orderId;
}

std::string Order::getOrderDate() const {
    return orderDate;
}

std::string Order::getOrderTime() const {
    return orderTime;
}

std::string Order::getCustomerName() const {
    return customerName;
}

std::string Order::getCustomerContact() const {
    return customerContact;
}

std::string Order::getOrderStatus() const {
    return orderStatus;
}

double Order::getPaymentAmount() const {
    return paymentAmount;
}

std::string Order::getPaymentMode() const {
    return paymentMode;
}

std::string Order::getPaymentMerchantName() const {
    return paymentMerchantName;
}

std::string Order::getPaymentDate() const {
    return paymentDate;
}

int Order::getDetailCount() const {
    return detailCount;
}

const OrderDetail& Order::getDetail(int index) const {
    if (index >= 0 && index < detailCount) {
        return details[index];
    }
    static OrderDetail emptyDetail;
    return emptyDetail;
}

void Order::setOrderId(const std::string& id) {
    orderId = id;
}

void Order::setOrderDate(const std::string& date) {
    orderDate = date;
}

void Order::setOrderTime(const std::string& time) {
    orderTime = time;
}

void Order::setCustomerName(const std::string& name) {
    customerName = name;
}

void Order::setCustomerContact(const std::string& contact) {
    customerContact = contact;
}

void Order::setOrderStatus(const std::string& status) {
    orderStatus = status;
}

void Order::setPaymentAmount(double amount) {
    paymentAmount = amount;
}

void Order::setPaymentMode(const std::string& mode) {
    paymentMode = mode;
}

void Order::setPaymentMerchantName(const std::string& merchant) {
    paymentMerchantName = merchant;
}

void Order::setPaymentDate(const std::string& payDate) {
    paymentDate = payDate;
}

int Order::findDetailIndex(const std::string& productId) const {
    for (int i = 0; i < detailCount; i++) {
        if (details[i].getProductId() == productId || details[i].getProductBarcode() == productId) {
            return i;
        }
    }
    return -1;
}

int Order::findDetailIndexByBarcode(const std::string& barcode) const {
    for (int i = 0; i < detailCount; i++) {
        if (details[i].getProductBarcode() == barcode) {
            return i;
        }
    }
    return -1;
}

bool Order::hasProduct(const std::string& productId) const {
    return findDetailIndex(productId) != -1;
}

int Order::getProductQuantity(const std::string& productId) const {
    int idx = findDetailIndex(productId);
    if (idx != -1) {
        return details[idx].getQuantity();
    }
    return 0;
}

bool Order::addProduct(const std::string& productId, const std::string& productBarcode, int quantity) {
    if (quantity <= 0) {
        return false;
    }

    // Check if the product already exists in this order (by product ID or barcode)
    int existingIndex = findDetailIndex(productId);
    if (existingIndex == -1 && !productBarcode.empty()) {
        existingIndex = findDetailIndexByBarcode(productBarcode);
    }

    if (existingIndex != -1) {
        // Increment the existing quantity consistently
        int newQuantity = details[existingIndex].getQuantity() + quantity;
        details[existingIndex].setQuantity(newQuantity);
        return true;
    }

    // Check capacity
    if (detailCount >= MAX_DETAILS_PER_ORDER) {
        return false;
    }

    // Add new product detail
    details[detailCount] = OrderDetail(orderId, productId, productBarcode, quantity);
    detailCount++;
    return true;
}

bool Order::updateProductQuantity(const std::string& productId, int newQuantity) {
    if (newQuantity <= 0) {
        return false;
    }

    int index = findDetailIndex(productId);
    if (index != -1) {
        details[index].setQuantity(newQuantity);
        return true;
    }

    return false;
}

bool Order::removeProduct(const std::string& productId) {
    int index = findDetailIndex(productId);
    if (index == -1) {
        index = findDetailIndexByBarcode(productId);
    }
    if (index == -1) {
        return false;
    }

    // Shift subsequent details down
    for (int i = index; i < detailCount - 1; i++) {
        details[i] = details[i + 1];
    }
    detailCount--;
    return true;
}

void Order::clearDetails() {
    detailCount = 0;
}


// ==========================================
// Helper String Functions (No std::vector used)
// ==========================================

static std::string trimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, (end - start + 1));
}

static std::string cleanLine(const std::string& line) {
    std::string cleaned = trimString(line);
    // Remove the trailing '#' requirement if present
    if (!cleaned.empty() && cleaned.back() == '#') {
        cleaned.pop_back();
    }
    return trimString(cleaned);
}

// Quote strings that contain spaces or are empty so space-separated parsing is preserved
static std::string formatField(const std::string& str) {
    if (str.empty()) {
        return "\"\"";
    }
    if (str.find(' ') != std::string::npos || str.find('\t') != std::string::npos) {
        return "\"" + str + "\"";
    }
    return str;
}

// Splits a space-separated line into tokens supporting quoted strings ("John Doe")
static int splitSpaceQuoted(const std::string& line, std::string tokens[], int maxTokens) {
    int count = 0;
    std::string current = "";
    bool inQuotes = false;
    bool hadQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            hadQuotes = true;
        } else if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!current.empty() || hadQuotes) {
                if (count < maxTokens) {
                    tokens[count++] = current;
                }
                current = "";
                hadQuotes = false;
            }
        } else {
            current += c;
        }
    }

    if (!current.empty() || hadQuotes) {
        if (count < maxTokens) {
            tokens[count++] = current;
        }
    }

    return count;
}


// ==========================================
// Date & 7-Day Calculation Implementation
// ==========================================

static int toJulianDayNumber(int y, int m, int d) {
    int a = (14 - m) / 12;
    int y2 = y + 4800 - a;
    int m2 = m + 12 * a - 3;
    return d + (153 * m2 + 2) / 5 + 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 - 32045;
}

bool OrderManager::parseDate(const std::string& dateStr, int& year, int& month, int& day) {
    std::string s = trimString(dateStr);
    if (s.length() < 10) {
        return false;
    }

    // Format: YYYY-MM-DD or YYYY/MM/DD
    if ((s[4] == '-' || s[4] == '/') && (s[7] == '-' || s[7] == '/')) {
        try {
            year = std::stoi(s.substr(0, 4));
            month = std::stoi(s.substr(5, 2));
            day = std::stoi(s.substr(8, 2));
            return (year >= 1900 && month >= 1 && month <= 12 && day >= 1 && day <= 31);
        } catch (...) {
            return false;
        }
    }

    // Format: DD-MM-YYYY or DD/MM/YYYY
    if ((s[2] == '-' || s[2] == '/') && (s[5] == '-' || s[5] == '/')) {
        try {
            day = std::stoi(s.substr(0, 2));
            month = std::stoi(s.substr(3, 2));
            year = std::stoi(s.substr(6, 4));
            return (year >= 1900 && month >= 1 && month <= 12 && day >= 1 && day <= 31);
        } catch (...) {
            return false;
        }
    }

    return false;
}

int OrderManager::daysBetweenDates(const std::string& fromDate, const std::string& toDate) {
    int fromY = 0, fromM = 0, fromD = 0;
    int toY = 0, toM = 0, toD = 0;

    if (!parseDate(fromDate, fromY, fromM, fromD)) {
        return -99999;
    }
    if (!parseDate(toDate, toY, toM, toD)) {
        return -99999;
    }

    int jdnFrom = toJulianDayNumber(fromY, fromM, fromD);
    int jdnTo = toJulianDayNumber(toY, toM, toD);

    return (jdnTo - jdnFrom);
}

std::string OrderManager::getCurrentSystemDate() {
    std::time_t now = std::time(nullptr);
    std::tm localTm = {};
#if defined(_MSC_VER)
    localtime_s(&localTm, &now);
#else
    localTm = *std::localtime(&now);
#endif
    std::ostringstream oss;
    oss << (localTm.tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << (localTm.tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << localTm.tm_mday;
    return oss.str();
}

bool OrderManager::isWithinSevenDays(const std::string& paymentDate, const std::string& currentDate) {
    if (paymentDate.empty()) {
        return false;
    }
    std::string refDate = currentDate.empty() ? getCurrentSystemDate() : currentDate;
    int diff = daysBetweenDates(paymentDate, refDate);
    return (diff >= 0 && diff <= 7);
}


// ==========================================
// OrderManager Implementation
// ==========================================

OrderManager::OrderManager() {
    orderCount = 0;
    orderFilePath = "Order.txt";
    orderDetailFilePath = "OrderDetail.txt";
}

OrderManager::OrderManager(const std::string& orderFile, const std::string& orderDetailFile) {
    orderCount = 0;
    orderFilePath = orderFile;
    orderDetailFilePath = orderDetailFile;
}

bool OrderManager::isValidStatus(const std::string& status) const {
    return (status == "in-progress" ||
            status == "on-hold" ||
            status == "paid" ||
            status == "archived" ||
            status == "cancelled");
}

bool OrderManager::isValidPaymentMode(const std::string& mode) const {
    return (mode == "cash" ||
            mode == "card" ||
            mode == "wallet");
}

std::string OrderManager::extractDayFromDate(const std::string& date) const {
    // If format is YYYY-MM-DD or YYYY/MM/DD (length >= 10, separator at 4)
    if (date.length() >= 10 && (date[4] == '-' || date[4] == '/')) {
        return date.substr(8, 2);
    }
    // If format is DD-MM-YYYY or DD/MM/YYYY (separator at 2)
    if (date.length() >= 10 && (date[2] == '-' || date[2] == '/')) {
        return date.substr(0, 2);
    }
    // If date string is directly 2 digits (e.g. "15")
    if (date.length() == 2 && std::isdigit(date[0]) && std::isdigit(date[1])) {
        return date;
    }
    // Default fallback to "01"
    return "01";
}

bool OrderManager::isValidOrderId(const std::string& orderId, const std::string& orderDate) const {
    // Must be exactly 8 characters long
    if (orderId.length() != 8) {
        return false;
    }

    // Must consist only of digits
    for (int i = 0; i < 8; i++) {
        if (!std::isdigit(orderId[i])) {
            return false;
        }
    }

    // First digit cannot be '0'
    if (orderId[0] == '0') {
        return false;
    }

    // Last two digits must indicate order date day (if orderDate provided)
    if (!orderDate.empty()) {
        std::string expectedDay = extractDayFromDate(orderDate);
        std::string actualDay = orderId.substr(6, 2);
        if (actualDay != expectedDay) {
            return false;
        }
    }

    return true;
}

std::string OrderManager::generateOrderId(const std::string& orderDate) const {
    std::string day = extractDayFromDate(orderDate);
    int highestSeq = 100000;

    // Scan existing orders to find the highest sequential number with the same day
    for (int i = 0; i < orderCount; i++) {
        std::string id = orders[i].getOrderId();
        if (id.length() == 8 && id.substr(6, 2) == day) {
            int seq = 0;
            bool valid = true;
            for (int j = 0; j < 6; j++) {
                if (std::isdigit(id[j])) {
                    seq = seq * 10 + (id[j] - '0');
                } else {
                    valid = false;
                    break;
                }
            }
            if (valid && seq > highestSeq) {
                highestSeq = seq;
            }
        }
    }

    int nextSeq = highestSeq + 1;
    return std::to_string(nextSeq) + day;
}

bool OrderManager::createOrder(const std::string& orderId, const std::string& orderDate,
                               const std::string& orderTime, const std::string& customerName,
                               const std::string& customerContact, const std::string& orderStatus,
                               double paymentAmount, const std::string& paymentMode,
                               const std::string& paymentMerchantName,
                               const std::string& paymentDate) {
    if (orderCount >= MAX_ORDERS) {
        return false;
    }

    if (!isValidOrderId(orderId, orderDate)) {
        return false;
    }

    if (!isValidStatus(orderStatus)) {
        return false;
    }

    if (!isValidPaymentMode(paymentMode)) {
        return false;
    }

    if (paymentAmount < 0.0) {
        return false;
    }

    if (customerName.empty()) {
        return false;
    }

    // Order ID must be unique
    if (findOrder(orderId) != nullptr) {
        return false;
    }

    orders[orderCount] = Order(orderId, orderDate, orderTime, customerName, customerContact,
                               orderStatus, paymentAmount, paymentMode, paymentMerchantName, paymentDate);
    orderCount++;

    saveAllOrders();
    return true;
}

// Requirement 3: Order Editing & Policy Enforcement
bool OrderManager::canEditOrder(const std::string& orderId, bool managerApproval,
                                const std::string& currentDate, std::string* outReason) const {
    const Order* order = findOrder(orderId);
    if (order == nullptr) {
        if (outReason) *outReason = "Order not found with ID: " + orderId;
        return false;
    }

    std::string status = order->getOrderStatus();

    if (status == "archived") {
        if (outReason) *outReason = "Archived orders cannot be modified.";
        return false;
    }

    if (status == "cancelled") {
        if (outReason) *outReason = "Cancelled orders cannot be modified.";
        return false;
    }

    if (status == "in-progress" || status == "on-hold") {
        if (outReason) *outReason = "Order is active and available for modification.";
        return true;
    }

    if (status == "paid") {
        // Enforce Requirement 3: Paid orders can only be edited within 7 days AND with manager approval
        std::string payDate = order->getPaymentDate();
        if (payDate.empty()) {
            if (outReason) *outReason = "Error: Paid order has no recorded payment date.";
            return false;
        }

        std::string refDate = currentDate.empty() ? getCurrentSystemDate() : currentDate;
        int diffDays = daysBetweenDates(payDate, refDate);

        if (diffDays < 0) {
            if (outReason) *outReason = "Error: Payment date is in the future relative to current date.";
            return false;
        }

        if (diffDays > 7) {
            if (outReason) *outReason = "Error: Paid orders cannot be edited after 7 days from payment (Elapsed: " +
                                       std::to_string(diffDays) + " days).";
            return false;
        }

        if (!managerApproval) {
            if (outReason) *outReason = "Error: Manager approval is strictly required to edit a paid order.";
            return false;
        }

        if (outReason) *outReason = "Success: Paid order approved for modification within 7 days.";
        return true;
    }

    if (outReason) *outReason = "Error: Unrecognized order status '" + status + "'.";
    return false;
}

bool OrderManager::openOrder(const std::string& orderId, Order& outOrder) const {
    const Order* order = findOrder(orderId);
    if (order != nullptr) {
        outOrder = *order;
        return true;
    }
    return false;
}

Order* OrderManager::openOrderForEditing(const std::string& orderId, bool managerApproval,
                                        const std::string& currentDate, std::string* outReason) {
    if (!canEditOrder(orderId, managerApproval, currentDate, outReason)) {
        return nullptr;
    }
    return findOrder(orderId);
}

bool OrderManager::addProductToOrder(const std::string& orderId, const std::string& productId,
                                     const std::string& productBarcode, int quantity,
                                     bool managerApproval, const std::string& currentDate) {
    std::string reason;
    if (!canEditOrder(orderId, managerApproval, currentDate, &reason)) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    bool added = order->addProduct(productId, productBarcode, quantity);
    if (added) {
        saveAllOrderDetails();
    }
    return added;
}

bool OrderManager::updateProductQuantityInOrder(const std::string& orderId, const std::string& productId,
                                                int newQuantity, bool managerApproval,
                                                const std::string& currentDate) {
    std::string reason;
    if (!canEditOrder(orderId, managerApproval, currentDate, &reason)) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    bool updated = false;
    if (newQuantity <= 0) {
        updated = order->removeProduct(productId);
    } else {
        updated = order->updateProductQuantity(productId, newQuantity);
    }

    if (updated) {
        saveAllOrderDetails();
    }
    return updated;
}

bool OrderManager::removeProductFromOrder(const std::string& orderId, const std::string& productId,
                                          bool managerApproval, const std::string& currentDate) {
    std::string reason;
    if (!canEditOrder(orderId, managerApproval, currentDate, &reason)) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    bool removed = order->removeProduct(productId);
    if (removed) {
        saveAllOrderDetails();
    }
    return removed;
}

// Requirement 3 & 10: Holding and Resuming Orders
bool OrderManager::holdOrder(const std::string& orderId) {
    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }
    if (order->getOrderStatus() != "in-progress") {
        return false;
    }
    order->setOrderStatus("on-hold");
    return saveAllOrders();
}

bool OrderManager::resumeOrder(const std::string& orderId) {
    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }
    if (order->getOrderStatus() != "on-hold") {
        return false;
    }
    order->setOrderStatus("in-progress");
    return saveAllOrders();
}

int OrderManager::getOnHoldOrders(Order outOrders[], int maxCapacity) const {
    int count = 0;
    for (int i = 0; i < orderCount && count < maxCapacity; i++) {
        if (orders[i].getOrderStatus() == "on-hold") {
            outOrders[count++] = orders[i];
        }
    }
    return count;
}

int OrderManager::getOnHoldOrderCount() const {
    int count = 0;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].getOrderStatus() == "on-hold") {
            count++;
        }
    }
    return count;
}

int OrderManager::getActiveOrderCount() const {
    int count = 0;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].getOrderStatus() == "in-progress") {
            count++;
        }
    }
    return count;
}

int OrderManager::getPaidOrderCount() const {
    int count = 0;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].getOrderStatus() == "paid") {
            count++;
        }
    }
    return count;
}

Order* OrderManager::getLatestOnHoldOrder() {
    for (int i = orderCount - 1; i >= 0; i--) {
        if (orders[i].getOrderStatus() == "on-hold") {
            return &orders[i];
        }
    }
    return nullptr;
}

Order* OrderManager::findOrder(const std::string& orderId) {
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].getOrderId() == orderId) {
            return &orders[i];
        }
    }
    return nullptr;
}

const Order* OrderManager::findOrder(const std::string& orderId) const {
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].getOrderId() == orderId) {
            return &orders[i];
        }
    }
    return nullptr;
}

bool OrderManager::updateOrderStatus(const std::string& orderId, const std::string& newStatus) {
    if (!isValidStatus(newStatus)) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    order->setOrderStatus(newStatus);
    return saveAllOrders();
}

bool OrderManager::updatePaymentInformation(const std::string& orderId, double paymentAmount,
                                            const std::string& paymentMode, const std::string& merchantName,
                                            const std::string& paymentDate) {
    if (!isValidPaymentMode(paymentMode)) {
        return false;
    }

    if (paymentAmount < 0.0) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    order->setPaymentAmount(paymentAmount);
    order->setPaymentMode(paymentMode);
    order->setPaymentMerchantName(merchantName);
    if (!paymentDate.empty()) {
        order->setPaymentDate(paymentDate);
    }

    return saveAllOrders();
}

bool OrderManager::markOrderAsPaid(const std::string& orderId, double paymentAmount,
                                   const std::string& paymentMode, const std::string& merchantName,
                                   const std::string& paymentDate) {
    if (!isValidPaymentMode(paymentMode) || paymentAmount < 0.0) {
        return false;
    }

    Order* order = findOrder(orderId);
    if (order == nullptr) {
        return false;
    }

    std::string pDate = paymentDate.empty() ? getCurrentSystemDate() : paymentDate;
    order->setOrderStatus("paid");
    order->setPaymentAmount(paymentAmount);
    order->setPaymentMode(paymentMode);
    order->setPaymentMerchantName(merchantName);
    order->setPaymentDate(pDate);

    return saveAllOrders();
}

bool OrderManager::loadOrders() {
    std::ifstream file(orderFilePath);
    if (!file.is_open()) {
        // Safe handling for missing file: treat as empty state
        return true;
    }

    orderCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue;
        }

        std::string fields[12];
        int fieldCount = 0;

        // Support both comma-separated and space-separated quoted formats
        if (cleaned.find(',') != std::string::npos) {
            int count = 0;
            std::string current = "";
            for (size_t i = 0; i < cleaned.length(); i++) {
                if (cleaned[i] == ',') {
                    if (count < 12) fields[count++] = trimString(current);
                    current = "";
                } else {
                    current += cleaned[i];
                }
            }
            if (count < 12) fields[count++] = trimString(current);
            fieldCount = count;
        } else {
            fieldCount = splitSpaceQuoted(cleaned, fields, 12);
        }

        if (fieldCount >= 9) {
            std::string oId = fields[0];
            std::string oDate = fields[1];
            std::string oTime = fields[2];
            std::string cName = fields[3];
            std::string cContact = fields[4];
            std::string oStatus = fields[5];
            double pAmount = 0.0;
            try {
                pAmount = std::stod(fields[6]);
            } catch (...) {
                pAmount = 0.0;
            }
            std::string pMode = fields[7];
            std::string pMerchant = fields[8];
            std::string pDate = "";

            // Field 10 is the dedicated Payment Date (Requirement 3)
            if (fieldCount >= 10) {
                pDate = fields[9];
                if (pDate == "\"\"" || pDate == "None" || pDate == "-") {
                    pDate = "";
                }
            }

            if (orderCount < MAX_ORDERS) {
                orders[orderCount] = Order(oId, oDate, oTime, cName, cContact,
                                           oStatus, pAmount, pMode, pMerchant, pDate);
                orderCount++;
            }
        }
    }

    file.close();
    return true;
}

bool OrderManager::loadOrderDetails() {
    std::ifstream file(orderDetailFilePath);
    if (!file.is_open()) {
        // Safe handling for missing file
        return true;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue;
        }

        std::string fields[6];
        int fieldCount = 0;

        if (cleaned.find(',') != std::string::npos) {
            int count = 0;
            std::string current = "";
            for (size_t i = 0; i < cleaned.length(); i++) {
                if (cleaned[i] == ',') {
                    if (count < 6) fields[count++] = trimString(current);
                    current = "";
                } else {
                    current += cleaned[i];
                }
            }
            if (count < 6) fields[count++] = trimString(current);
            fieldCount = count;
        } else {
            fieldCount = splitSpaceQuoted(cleaned, fields, 6);
        }

        if (fieldCount >= 4) {
            std::string oId = fields[0];
            std::string pId = fields[1];
            std::string pBarcode = fields[2];
            int qty = 1;
            try {
                qty = std::stoi(fields[3]);
            } catch (...) {
                qty = 1;
            }

            Order* order = findOrder(oId);
            if (order != nullptr) {
                order->addProduct(pId, pBarcode, qty);
            }
        }
    }

    file.close();
    return true;
}

bool OrderManager::loadAll() {
    bool okOrders = loadOrders();
    bool okDetails = loadOrderDetails();
    return okOrders && okDetails;
}

bool OrderManager::saveOrder(const Order& order) {
    Order* existing = findOrder(order.getOrderId());
    if (existing != nullptr) {
        *existing = order;
        return saveAllOrders();
    }

    if (orderCount >= MAX_ORDERS) {
        return false;
    }

    orders[orderCount++] = order;
    return saveAllOrders();
}

bool OrderManager::saveOrderDetails(const Order& order) {
    // Synchronize and write all order details to maintain consistency
    return saveAllOrderDetails();
}

bool OrderManager::saveAllOrders() {
    std::ofstream file(orderFilePath);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < orderCount; i++) {
        file << orders[i].getOrderId() << " "
             << orders[i].getOrderDate() << " "
             << orders[i].getOrderTime() << " "
             << formatField(orders[i].getCustomerName()) << " "
             << formatField(orders[i].getCustomerContact()) << " "
             << orders[i].getOrderStatus() << " "
             << std::fixed << std::setprecision(2) << orders[i].getPaymentAmount() << " "
             << orders[i].getPaymentMode() << " "
             << formatField(orders[i].getPaymentMerchantName());

        // Field 10: Payment date is persisted when available (Requirement 3)
        if (!orders[i].getPaymentDate().empty()) {
            file << " " << formatField(orders[i].getPaymentDate());
        }

        file << " #\n";
    }

    file.close();
    return true;
}

bool OrderManager::saveAllOrderDetails() {
    std::ofstream file(orderDetailFilePath);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < orderCount; i++) {
        int detailsCount = orders[i].getDetailCount();
        for (int j = 0; j < detailsCount; j++) {
            const OrderDetail& detail = orders[i].getDetail(j);
            file << detail.getOrderId() << " "
                 << detail.getProductId() << " "
                 << detail.getProductBarcode() << " "
                 << detail.getQuantity() << " #\n";
        }
    }

    file.close();
    return true;
}

bool OrderManager::saveAll() {
    return saveAllOrders() && saveAllOrderDetails();
}

int OrderManager::getOrderCount() const {
    return orderCount;
}

const Order* OrderManager::getOrderAt(int index) const {
    if (index >= 0 && index < orderCount) {
        return &orders[index];
    }
    return nullptr;
}

Order* OrderManager::getOrderAt(int index) {
    if (index >= 0 && index < orderCount) {
        return &orders[index];
    }
    return nullptr;
}

void OrderManager::clear() {
    orderCount = 0;
}
