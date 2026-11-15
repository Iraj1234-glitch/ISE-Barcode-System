#include "../../include/order/OrderManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>

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
    detailCount = 0;
}

Order::Order(const std::string& id, const std::string& date, const std::string& time,
             const std::string& custName, const std::string& custContact,
             const std::string& status, double amount,
             const std::string& payMode, const std::string& merchant) {
    orderId = id;
    orderDate = date;
    orderTime = time;
    customerName = custName;
    customerContact = custContact;
    orderStatus = status;
    paymentAmount = amount;
    paymentMode = payMode;
    paymentMerchantName = merchant;
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

int Order::findDetailIndex(const std::string& productId) const {
    for (int i = 0; i < detailCount; i++) {
        if (details[i].getProductId() == productId) {
            return i;
        }
    }
    return -1;
}

bool Order::addProduct(const std::string& productId, const std::string& productBarcode, int quantity) {
    if (quantity <= 0) {
        return false;
    }

    // Check if the product already exists in this order
    int existingIndex = findDetailIndex(productId);
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
    if (str.empty() || str.find(' ') != std::string::npos || str.find('\t') != std::string::npos) {
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
            status == "archived");
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
                               const std::string& paymentMerchantName) {
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
                               orderStatus, paymentAmount, paymentMode, paymentMerchantName);
    orderCount++;

    saveAllOrders();
    return true;
}

bool OrderManager::addProductToOrder(const std::string& orderId, const std::string& productId,
                                     const std::string& productBarcode, int quantity) {
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
                                            const std::string& paymentMode, const std::string& merchantName) {
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

        std::string fields[10];
        int fieldCount = 0;

        // Support both comma-separated and space-separated quoted formats
        if (cleaned.find(',') != std::string::npos) {
            int count = 0;
            std::string current = "";
            for (size_t i = 0; i < cleaned.length(); i++) {
                if (cleaned[i] == ',') {
                    if (count < 10) fields[count++] = trimString(current);
                    current = "";
                } else {
                    current += cleaned[i];
                }
            }
            if (count < 10) fields[count++] = trimString(current);
            fieldCount = count;
        } else {
            fieldCount = splitSpaceQuoted(cleaned, fields, 10);
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

            if (orderCount < MAX_ORDERS) {
                orders[orderCount] = Order(oId, oDate, oTime, cName, cContact,
                                           oStatus, pAmount, pMode, pMerchant);
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
             << formatField(orders[i].getPaymentMerchantName()) << " #\n";
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
