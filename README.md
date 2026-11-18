# ISE Barcode Management & Point of Sale System

A C++ desktop application that generates and scans six-digit **Code 128** barcodes and provides a complete **Point of Sale (POS) system** for managing products, recording customer orders, processing payments, and generating bills.

The project is developed in increments as part of the ISE course project.

- **Increment 1:** Barcode generation, image scanning, camera scanning, and graphical interface.
- **Increment 2:** Product management, POS order management, inventory validation, payment handling, and bill generation.

---

## Features

### Increment 1 – Barcode Management

- **Generate Barcode:** Enter a six-digit code and automatically save its Code 128 barcode as a `.bmp` image.
- **Image Scanner:** Select an image (`.bmp`, `.png`, `.jpg`, `.jpeg`) and read the barcode.
- **Camera Scanner:** Scan a barcode live using the laptop camera.
- **GUI:** Navigate between barcode generation and scanning features through a graphical interface.

### Increment 2 – Point of Sale System

- **Product Management:** Maintain product records using `Product.txt`.
- **Product Lookup:** Retrieve product information by scanning its barcode.
- **Product Description:** Cashiers and managers can view product descriptions by scanning a barcode. Cashiers can also view descriptions while recording an order.
- **Order Management:** Create and maintain customer orders, including customer information, order date, time, status, and payment details.
- **Inventory Validation:** Add products to an order only when the requested quantity does not exceed the available stock.
- **Order Details:** Maintain individual product entries associated with each order.
- **Payment Management:** Record payment amount, payment mode, and merchant information.
- **Bill Generation:** Generate bills from recorded orders, including product prices, discounts, taxes, and total payable amount.
- **File Persistence:** Store product, order, order-detail, and bill records in text files.

---

## Technologies Used

| Purpose | Tool |
|---|---|
| Programming Language | C++ |
| GUI | SFML |
| File open/save dialogs | Windows API (`windows.h`, `commdlg.h`) |
| Image loading | `stb_image.h` |
| Camera handling | OpenCV |
| Barcode reading | ZXing-C++ |
| Data storage | Text files (`.txt`) |
| Development Environment | Visual Studio |
| Build Architecture | Windows x64 |

The project currently targets **Windows**.

---

## Project Structure

```text
BarcodeApp/
│
├── include/
│   ├── generation/
│   │   └── BarcodeGenerator.h
│   │
│   ├── image_scanner/
│   │   ├── imageScanner.h
│   │   └── stb_image.h
│   │
│   ├── camera_scanner/
│   │   └── CameraScanner.h
│   │
│   ├── gui/
│   │   ├── MainWindow.h
│   │   └── ProductCatalog.h
│   │
│   └── order/
│       ├── Order.h
│       └── OrderManager.h
│
├── src/
│   ├── generation/
│   │   └── BarcodeGenerator.cpp
│   │
│   ├── image_scanner/
│   │   └── imageScanner.cpp
│   │
│   ├── camera_scanner/
│   │   └── CameraScanner.cpp
│   │
│   ├── gui/
│   │   ├── MainWindow.cpp
│   │   └── ProductCatalog.cpp
│   │
│   ├── order/
│   │   └── OrderManager.cpp
│   │
│   └── main.cpp
│
├── barcodes/
│   └── Generated barcode images (.bmp)
│
├── Product.txt
├── Order.txt
├── OrderDetail.txt
├── Bill.txt
│
├── tests/
│   └── test_order_manager.cpp
│
└── README.md
```

*Note: The structure shows the main project files. Additional files may be added as the system develops.*

---

## System Architecture

The application is divided into separate modules. Each module is responsible for a specific part of the system.

```text
                       MainWindow (GUI)
                              |
          -----------------------------------------
          |                   |                   |
   Barcode Generation   Barcode Scanning    POS Management
                              |                   |
                    -----------------     ---------------------
                    |               |     |         |         |
               Image Scanner  Camera Scanner Product  Order   Billing
                                             Manager Manager
```

The GUI connects the modules and handles user interaction. The modules keep their respective responsibilities separate.

---

## Main Modules

### 1. Barcode Generator (`BarcodeGenerator`)

Creates a Code 128 barcode image from a six-digit code.

1. Checks that the code contains exactly six digits and does not start with zero.
2. Uses Code 128 Code B and calculates the checksum.
3. Adds the Stop symbol.
4. Converts the Code 128 patterns into black and white bars.
5. Creates a 24-bit BMP image.
6. Automatically saves the image using the barcode number as its filename.

Example:

```text
Barcode: 123456
Saved file: barcodes/123456.bmp
```

Function:

```cpp
bool generateBarcode(const string& code, const string& outputPath);
```

Returns `true` when the barcode is successfully saved and `false` if generation fails.

### 2. Image Scanner (`readBarcodeFromImage`)

Reads a Code 128 barcode from an image.

1. Loads the image in grayscale using `stb_image`.
2. Examines multiple horizontal rows.
3. Converts pixels into black and white patterns.
4. Measures bar and space widths.
5. Matches the measured patterns with Code 128 symbols.
6. Validates the checksum.
7. Accepts a result when multiple rows agree on the same six-digit code.

Function:

```cpp
bool readBarcodeFromImage(string path, string& code, string& error);
```

On success, `code` contains the scanned barcode. On failure, `error` contains an appropriate message.

### 3. Camera Scanner (`CameraScanner`)

Scans barcodes using the laptop camera.

1. Opens the default camera using OpenCV.
2. Captures frames continuously.
3. Converts frames into grayscale.
4. Uses ZXing-C++ to detect and decode barcodes.
5. Accepts valid Code 128 barcodes containing six digits.
6. Displays the scanned result.
7. Allows the user to stop scanning using ESC.

Function:

```cpp
string scanUsingCamera();
```

Returns the scanned code or an empty string if no valid barcode is detected.

### 4. Product Management (`ProductCatalog`)

Manages product information stored in `Product.txt`.

Responsibilities include:

- Loading product records from the file.
- Finding products using their barcode or product ID.
- Providing product information to the POS system.
- Displaying product descriptions.
- Checking product availability.
- Supporting stock validation and updates.

Product records contain the following information:

| Field | Description |
|---|---|
| Product ID | Unique product identifier |
| Barcode | Six-digit barcode |
| Product Acronym | Short product name (maximum 8 characters) |
| Product Name | Full product name |
| Unit | Count or weight |
| Unit Price | Price per unit |
| Tax Amount | Tax in PKR |
| Discount per Unit | Discount applied to each unit |
| Available Quantity | Current stock |
| Barcode File | Barcode image filename or path |

### 5. Order Management (`OrderManager`)

Handles the creation, maintenance, and storage of customer orders.

Responsibilities include:

- Creating orders with unique eight-digit order IDs.
- Recording customer name and optional contact information.
- Storing order date and time.
- Maintaining order status.
- Adding products to orders.
- Updating product quantities.
- Maintaining order details separately.
- Recording payment information.
- Loading and saving order records.

An order can contain multiple products. Each product entry is associated with its order ID.

Order statuses:

- `in-progress`
- `on-hold`
- `paid`
- `archived`

Supported payment modes:

- `cash`
- `card`
- `wallet`

### 6. Billing

The billing module generates bills using recorded order and product information.

Responsibilities include:

- Retrieving the selected order.
- Calculating product totals.
- Applying discounts.
- Calculating taxes.
- Calculating the final payable amount.
- Generating a unique nine-digit bill ID.
- Saving bill information in `Bill.txt`.

A bill includes:

- Bill ID
- Customer name
- Order ID
- Product information
- Unit prices
- Discounts
- Quantities
- Total amount before tax
- Total tax
- Total discount
- Final payable amount
- Payment status

### 7. GUI (`MainWindow`)

Provides the graphical interface for the barcode management and POS system.

The GUI is responsible for:

- Displaying the dashboard and navigation.
- Handling mouse clicks and keyboard input.
- Connecting barcode generation and scanning modules.
- Displaying product information.
- Supporting order recording and management.
- Displaying payment information.
- Supporting bill generation.
- Showing success and error messages.

---

## Data Storage and File Formats

The system uses text files to store persistent information.

Each record ends with `#`. Fields are separated by spaces, with quoted fields used where necessary to preserve values containing spaces.

### 1. Product.txt

Stores product information, with one product per line.

Field order:

```text
ProductID Barcode Acronym ProductName Unit UnitPrice Tax Discount AvailableQuantity BarcodeFile#
```

Example:

```text
P101 123456 COLA CocaCola count 100.00 18.00 5.00 50 barcodes/123456.bmp#
```

### 2. Order.txt

Stores order information, with one order per line.

Field order:

```text
OrderID OrderDate OrderTime CustomerName CustomerContact Status PaymentAmount PaymentMode MerchantName#
```

Example:

```text
10000115 2026-11-15 10:30:00 "John Doe" 03001234567 in-progress 0.00 cash None#
```

An optional customer contact can be stored as an empty quoted field.

### 3. OrderDetail.txt

Stores the products associated with each order. Each product entry is stored on a separate line.

Field order:

```text
OrderID ProductID Barcode Quantity#
```

Example:

```text
10000115 P101 123456 5#
10000115 P102 654321 2#
```

### 4. Bill.txt

Stores generated bill records. Each bill is stored as one record ending with `#`.

Bill information includes:

```text
BillID CustomerName OrderID ProductDetails TotalWithoutTax TotalTax TotalDiscount TotalPayable PaymentStatus#
```

Product details can contain multiple items in the same bill record, separated using the agreed item separator.

*The exact bill serialization format and product-item separator must remain consistent with the billing module's implementation.*

---

## Rules for Barcode Codes

All generated and accepted product barcodes must follow these rules:

- Exactly six digits.
- Only numeric characters are allowed.
- The first digit cannot be zero.

The generator, GUI, and camera scanner validate barcode input. The image scanner also validates the decoded result.

---

## Software Engineering Principles Used

| Principle | Application |
|---|---|
| **Modularity** | The application is divided into barcode generation, scanning, product, order, billing, and GUI modules. |
| **Separation of Concerns** | Each module handles its own functionality rather than mixing all logic inside the GUI. |
| **Single Responsibility Principle (SRP)** | Product management, order management, billing, and barcode operations have separate responsibilities. |
| **Open/Closed Principle (OCP)** | New functionality can be added through separate modules without unnecessarily modifying existing barcode functionality. |
| **Encapsulation** | Internal data and implementation details are managed through classes and controlled public functions. |
| **Abstraction** | The GUI uses simple module interfaces without needing to understand their internal implementation. |
| **Low Coupling** | Modules communicate through defined interfaces instead of depending on each other's internal code. |
| **High Cohesion** | Functions within each module work toward a common responsibility. |
| **Data-Driven Design** | Product and order information is loaded from external text files rather than being hardcoded. |
| **Event-Driven Programming** | The GUI responds to mouse clicks, keyboard input, and window events. |
| **Input Validation** | Barcode codes, product quantities, order information, and payment details are validated. |
| **Error Handling** | Invalid inputs, missing files, and unsuccessful operations are handled through error messages and return values. |
| **Data Integrity** | Barcode checksums, order IDs, product records, and stock validation help maintain reliable data. |
| **Reusability** | Separate modules and reusable functions support multiple parts of the application. |
| **Resource Management** | Files, camera resources, and image memory are released after use. |
| **Maintainability** | Separate header and implementation files make the project easier to understand and modify. |

---

## How to Build and Run

### Requirements

- Windows
- Visual Studio with C++ desktop development
- SFML 2.x
- OpenCV 4.x
- ZXing-C++
- `stb_image.h`
- Webcam for camera scanning

### Installation

Libraries can be installed using vcpkg:

```bash
vcpkg install sfml:x64-windows opencv4:x64-windows zxing-cpp:x64-windows
vcpkg integrate install
```

### Build Steps

1. Clone or download the project repository.
2. Open `BarcodeApp.vcxproj` in Visual Studio.
3. Ensure all source files are included in the project.
4. Verify the required library and include paths.
5. Select the **x64** configuration.
6. Build the solution.
7. Run the application.

The working directory must contain the required data files, particularly `Product.txt`.

The program creates or uses the `barcodes` directory for generated barcode images. Order, order-detail, and bill files are created or updated when their respective operations are performed.

---

## How to Use

### Barcode Generation

1. Open the Generate Barcode page.
2. Enter a valid six-digit barcode.
3. Click Generate.
4. The barcode is automatically saved in the `barcodes` directory.
5. The generated barcode is displayed in the GUI.

### Image Scanning

1. Open the Image Scanner page.
2. Select an image containing a barcode.
3. Start scanning.
4. The decoded barcode is displayed.

### Camera Scanning

1. Open the Camera Scanner page.
2. Start the camera.
3. Show a valid barcode to the camera.
4. The scanned barcode is displayed when detected.
5. Press ESC to stop scanning.

### POS and Order Recording

1. Open the POS/order section.
2. Scan a product barcode using the camera.
3. The system retrieves the corresponding product information from `Product.txt`.
4. Enter the required quantity.
5. The system checks whether sufficient stock is available.
6. Add the product to the order if the quantity is valid.
7. Repeat to add more products.
8. Enter customer information and maintain the order.

### Product Description

- Scan a product barcode to retrieve its description.
- Cashiers can also use the Description option while recording an order.
- Managers can scan products to view their descriptions.

### Payment and Bill Generation

1. Open the required order.
2. Review the order and its product details.
3. Enter payment information.
4. Select the payment mode.
5. Generate the bill.
6. The system calculates the total amount, tax, discounts, and payable amount.
7. The generated bill is saved in `Bill.txt`.

---

## Testing

The system is tested at both module and integration levels.

| Module | Test Case | Expected Result |
|---|---|---|
| Barcode Generator | Generate a valid six-digit barcode | BMP image is saved automatically |
| Barcode Generator | Enter fewer than six digits | Validation error is displayed |
| Barcode Generator | Enter a code starting with zero | Validation error is displayed |
| Image Scanner | Scan a valid generated barcode image | Correct barcode is decoded |
| Image Scanner | Scan an image without a barcode | Appropriate error is displayed |
| Camera Scanner | Scan a valid barcode | Correct barcode is detected |
| Camera Scanner | Camera unavailable | Error is handled without crashing |
| Product Management | Load valid product records | Product information is loaded |
| Product Management | Search using an existing barcode | Correct product is returned |
| Product Management | Search using an invalid barcode | Product is not found |
| Order Management | Create a new order | Unique order ID is generated |
| Order Management | Add a product to an order | Product detail is recorded |
| Order Management | Add the same product again | Existing quantity is updated |
| Order Management | Save and reload orders | Order data is preserved |
| Inventory | Request quantity within available stock | Product can be added |
| Inventory | Request quantity greater than available stock | Product addition is rejected |
| Billing | Generate a bill for a valid order | Bill totals are calculated |
| Billing | Generate a bill with multiple products | All product details are included |
| Billing | Save a generated bill | Bill record is stored in `Bill.txt` |
| Integration | Scan a barcode and add the product to an order | Correct product is added |
| Integration | Complete an order and generate a bill | Correct bill is generated and saved |

---

## Team Members

| Name | Responsibility |
|---|---|
| **Iraj** | Order Management and File Persistence |
| **Izzah** | POS GUI and Module Integration |
| **Wajiha** | Billing and Bill Generation |
| **Hiba** | Product Management and Product Catalog |

---

## Project Status

| Increment | Scope | Status |
|---|---|---|
| Increment 1 | Barcode generation, image scanning, camera scanning, and GUI | Completed |
| Increment 2 | Product management, order management, inventory validation, payment handling, and billing | Completed |

