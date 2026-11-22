# ISE Barcode Management & Point of Sale System

A C++ desktop application that generates and scans six-digit **Code 128 barcodes** and provides a complete **Point of Sale (POS) system** for managing products, recording customer orders, processing payments, and generating bills.

The project is developed in increments as part of the Introduction to Software Engineering (ISE) course.

---

## 📌 Project Overview

The system combines barcode management with POS functionality to make product identification, order processing, inventory management, and billing easier and more reliable.

### Project Increments

| Increment | Description | Status |
|---|---|---|
| Increment 1 | Barcode generation, image scanning, camera scanning, and GUI | ✅ Completed |
| Increment 2 | Product management, order management, inventory validation, payment handling, and billing | ✅ Completed |
| Increment 3 | Category-based barcodes, enhanced product/order management, billing, discounts, reporting, and separate interfaces | 🔄 In Progress |

---

## ✨ Features

### Increment 1 – Barcode Management

- **Barcode Generation:** Generate six-digit Code 128 barcodes and save them as `.bmp` images.
- **Image Scanning:** Upload barcode images in BMP, PNG, JPG, or JPEG format and decode them.
- **Camera Scanning:** Scan barcodes live using a laptop camera.
- **Graphical Interface:** Navigate between barcode generation and scanning features through a GUI.

### Increment 2 – Point of Sale System

- **Product Management:** Maintain product records using `Product.txt`.
- **Product Lookup:** Retrieve product information by scanning its barcode.
- **Product Description:** Allow cashiers and managers to view product descriptions.
- **Order Management:** Create and maintain customer orders.
- **Inventory Validation:** Prevent adding quantities exceeding available stock.
- **Order Details:** Maintain individual product entries associated with each order.
- **Payment Management:** Record payment amounts, payment modes, and merchant information.
- **Bill Generation:** Generate bills containing product prices, discounts, taxes, and payable amounts.
- **File Persistence:** Store product, order, order-detail, and bill records in text files.

### Increment 3 – Enhancements (In Progress)

The following functionality is part of the current development increment. These features are under implementation and integration.

#### 1. Category Management and Barcode Generation
- Allow managers to configure product categories through the GUI.
- Store category ID, category name, and category acronym in `PCategory.txt`.
- Use the two-digit category ID as the first two digits of a product barcode.
- Store the category ID in the corresponding product record.
- Support product addition and editing through the manager interface.

#### 2. Order Management
- Allow cashiers to open and edit existing orders.
- Allow paid orders to be edited within seven days, subject to manager approval.
- Support holding and resuming orders.
- Minimize unnecessary clicks during order entry.
- Avoid requiring an additional click after barcode scanning.

#### 3. Inventory Management
- Provide managers with an inventory view.
- Allow managers to update inventory quantities.
- Update stock quantities whenever orders are processed.

#### 4. Billing Enhancements
- Generate and print bills in PDF format.
- Store detailed bill information, including:
  - Bill ID
  - Customer name
  - Order ID
  - Total before tax
  - Tax
  - Discount
  - Payable amount
  - Amount received
  - Returned balance
  - Payment status
  - Seasonal discount
  - Special discount

- Maintain individual bill product details in `BillDetail.txt`.

#### 5. Manager Reports
Provide reports for:
- Incomplete and completed orders with their totals.
- Cash sales over selected time periods.
- Sales and discounts.
- Top 10 best-selling products with their available stock.
- Top 10 products with the highest remaining stock.

#### 6. Separate User Interfaces
- Provide a separate interface for cashiers.
- Provide a separate interface for managers.
- Give managers access to product, category, inventory, discount, and reporting functionality.

#### 7. Seasonal and Special Discounts
- Allow managers to add and configure seasons.
- Store season start date, end date, discount percentage, and active status in `Seasons.txt`.
- Apply seasonal discounts when the bill date falls within an active season.
- Store the applied seasonal discount in the bill record.
- Support special discounts subject to the maximum allowed discount of 8%.

---

## 🛠️ Technologies Used

| Purpose | Technology |
|---|---|
| Programming Language | C++ |
| GUI | SFML |
| Barcode Generation | Code 128 |
| Barcode Reading | ZXing-C++ |
| Image Loading | stb_image |
| Camera Handling | OpenCV |
| File Dialogs | Windows API |
| Data Storage | Text Files |
| Development Environment | Visual Studio 2026 |
| Build Architecture | Windows x64 |

The project currently targets **Windows**.

---

## 📂 Project Structure

```text
ISE-Barcode-System/
│
├── .vscode/
│
├── BarcodeApp/
│   ├── include/
│   │   ├── billing/
│   │   ├── camera_scanner/
│   │   ├── generation/
│   │   ├── gui/
│   │   ├── image_scanner/
│   │   ├── order/
│   │   └── product/
│   │
│   ├── src/
│   │   ├── billing/
│   │   ├── camera_scanner/
│   │   ├── generation/
│   │   ├── gui/
│   │   ├── image_scanner/
│   │   ├── order/
│   │   └── product/
│   │
│   ├── barcodes/
│   ├── BarcodeApp.vcxproj
│   ├── main.cpp
│   ├── Product.txt
│   ├── PCategory.txt
│   ├── Order.txt
│   ├── OrderDetail.txt
│   ├── Bill.txt
│   ├── BillDetail.txt
│   └── Seasons.txt
│
├── tests/
│
├── Product.txt
├── README.md
└── .gitignore
```

*Note: The structure represents the main project modules and data files. Additional files may be added as development progresses.*

---

## 🏗️ System Architecture

The application is organized into separate modules, each responsible for a specific functionality.

```text
                    Main Application
                           |
              ---------------------------
              |                         |
          Barcode System              POS System
              |                         |
       -----------------        ---------------------
       |       |       |        |        |          |
    Generate Image  Camera   Product   Order     Billing
    Barcode Scanner Scanner  Manager  Manager    Module
                                  |
                             Inventory
                                  |
                            Category Manager
```

The GUI connects the modules and handles user interaction while keeping individual module responsibilities separate.

---

## 🧩 Main Modules

### 1. Barcode Generator

**Class:** `BarcodeGenerator`

Generates Code 128 barcode images from numeric codes.

Responsibilities:
- Validate barcode input.
- Generate Code 128 patterns.
- Calculate the checksum.
- Add the Stop symbol.
- Convert barcode patterns into black and white bars.
- Save the resulting barcode as a BMP image.

Example:

```text
Barcode: 123456
Output: barcodes/123456.bmp
```

### 2. Image Scanner

Reads Code 128 barcodes from uploaded images.

Responsibilities:
- Load images using `stb_image`.
- Process image pixels.
- Identify barcode patterns.
- Decode barcode symbols.
- Validate the checksum.
- Return the decoded barcode.

### 3. Camera Scanner

**Class:** `CameraScanner`

Uses OpenCV and ZXing-C++ to scan barcodes through a camera.

Responsibilities:
- Open the default camera.
- Capture video frames.
- Detect and decode barcodes.
- Validate barcode format.
- Display scanned results.
- Stop scanning when requested.

### 4. Product Management

Manages product records and provides product information to the POS system.

Responsibilities:
- Load and save product records.
- Search products using their barcode or product ID.
- Display product descriptions.
- Check product availability.
- Support stock validation and updates.
- Support manager product operations.

### 5. Category Management

Manages product categories and their associated identifiers.

Responsibilities:
- Add and manage product categories.
- Store category information in `PCategory.txt`.
- Associate products with their categories.
- Support category-based barcode generation.

### 6. Order Management

Handles customer orders and their associated products.

Responsibilities:
- Create orders.
- Generate unique order IDs.
- Record customer information.
- Store order date and time.
- Add and update order products.
- Maintain order details separately.
- Support order status management.
- Save and load order records.

### 7. Billing

Generates bills using order and product information.

Responsibilities:
- Retrieve order information.
- Calculate product totals.
- Apply discounts.
- Calculate taxes.
- Calculate payable amounts.
- Record payment information.
- Generate and store bills.

### 8. GUI

Provides the graphical interface for the application.

Responsibilities:
- Display the application dashboard.
- Handle user input and navigation.
- Connect the different modules.
- Display product information.
- Support order processing.
- Display billing information.
- Provide separate cashier and manager interfaces.

---

## 💾 Data Storage and File Formats

The system uses text files for persistent data storage.

Each record ends with `#`. Fields are separated by spaces, with quoted fields used where necessary to preserve values containing spaces.

### Main Data Files

| File | Purpose |
|---|---|
| `Product.txt` | Stores product information and stock |
| `PCategory.txt` | Stores product categories |
| `Order.txt` | Stores customer order information |
| `OrderDetail.txt` | Stores products associated with orders |
| `Bill.txt` | Stores generated bill records |
| `BillDetail.txt` | Stores individual bill product details |
| `Seasons.txt` | Stores seasonal discount information |

### Product Information

Product records contain information such as:

- Product ID
- Barcode
- Product acronym
- Product name
- Unit
- Unit price
- Tax
- Discount
- Available quantity
- Barcode image
- Category ID (Increment 3)

### Category Information

Each category contains:

- Two-digit Category ID
- Category Name
- Category Acronym

### Seasonal Information

Each season contains:

- Start date
- End date
- Discount percentage
- Active/inactive status

*The exact field order and serialization formats must remain consistent with the corresponding file-handler implementations.*

---

## 📏 Barcode Validation Rules

Generated and accepted product barcodes follow these rules:

- Exactly six digits.
- Only numeric characters are allowed.
- The first digit cannot be zero.

**Increment 3 enhancement:** The first two digits represent the product category ID.

The barcode generator, GUI, and scanning modules validate barcode input or decoded results.

---

## 📚 Software Engineering Principles

| Principle | Application |
|---|---|
| Modularity | Dividing the system into independent modules |
| Separation of Concerns | Keeping GUI, product, order, and billing logic separate |
| Single Responsibility Principle | Giving each module a specific responsibility |
| Encapsulation | Managing internal data through classes |
| Abstraction | Providing simple interfaces between modules |
| Low Coupling | Reducing unnecessary dependencies between modules |
| High Cohesion | Keeping related functionality together |
| Reusability | Reusing modules and functions |
| Input Validation | Validating user input and records |
| Error Handling | Handling invalid input and unsuccessful operations |
| Data Integrity | Maintaining consistent product, order, and billing records |
| Maintainability | Organizing code into separate headers and implementation files |

---

## ⚙️ How to Build and Run

### Requirements

- Windows
- Visual Studio 2026
- C++ Desktop Development workload
- SFML 2.x
- OpenCV 4.x
- ZXing-C++
- vcpkg
- Webcam for camera scanning

### Build Steps

1. Clone or download the project repository.
2. Open the solution or `BarcodeApp.vcxproj` in Visual Studio.
3. Ensure the required dependencies are installed.
4. Select the **x64** configuration.
5. Build the solution.
6. Run the application.

The working directory must contain the required data files, particularly `Product.txt`.

---

## 🧪 Testing

The project includes module-level and integration-level tests.

Testing covers:

- Valid and invalid barcode generation.
- Barcode image scanning.
- Camera scanning.
- Product lookup.
- Product record management.
- Order creation and modification.
- Inventory validation.
- Bill calculations.
- File persistence.
- Integration between scanning, product lookup, and order processing.

Increment 3 functionality will be tested as implementation and integration progress.

---

## 👥 Team Members and Responsibilities

| Team Member | Responsibilities |
|---|---|
| **Iraj** | Barcode Generation, Category Management, Product Add/Edit |
| **Wajiha** | Camera Scanning, Order and Inventory Workflow |
| **Hiba** | Image Scanning |
| **Izzah** | Central GUI Integration and Final Integration Coordination |

---

## 📊 Project Status

| Increment | Scope | Status |
|---|---|---|
| Increment 1 | Barcode generation, image scanning, camera scanning, and GUI | ✅ Completed |
| Increment 2 | Product management, order management, inventory validation, payment handling, and billing | ✅ Completed |
| Increment 3 | Category-based barcodes, order and inventory enhancements, billing, discounts, reports, and separate interfaces | 🔄 **In Progress** |

### Current Development Phase

The team is currently working on Increment 3.

Individual modules are being developed and integrated through separate Git branches. Final GUI integration and overall system testing will follow as the modules become ready.

---

**ISE Barcode Management & Point of Sale System**  
*C++ Desktop Application | Introduction to Software Engineering Course*
