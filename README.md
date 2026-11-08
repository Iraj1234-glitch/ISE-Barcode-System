# ISE Barcode System

### A C++ Desktop Application for Barcode Generation and Scanning

---

## 1. Project Overview

The **ISE Barcode System** is a desktop application designed to generate and scan Code 128 barcodes. It allows users to generate a barcode from a six-digit numeric code and decode barcodes using either a camera or an uploaded image.

This project is being developed as part of the Introduction to Software Engineering course.

## 2. Problem Statement

Manually entering and verifying numeric codes can be inconvenient and prone to errors. This project aims to provide a simple barcode-based solution that makes code generation and retrieval easier, faster, and more reliable.

## 3. Project Objectives

- Generate Code 128 barcodes from six-digit numeric codes.
- Validate user input before barcode generation.
- Scan barcodes using a computer camera.
- Upload barcode images and decode their contents.
- Provide a user-friendly desktop graphical interface.

## 4. Key Features

- Six-digit numeric input validation.
- Code 128 barcode generation.
- Barcode scanning through a camera.
- Barcode decoding from uploaded images.
- Graphical user interface for interacting with the application.

*Features are being implemented and integrated progressively.*

## 5. Technologies and Tools

| Technology | Purpose |
|---|---|
| C++ | Main programming language |
| ZXing-C++ | Barcode generation and decoding |
| OpenCV | Camera access and image processing |
| SFML | Desktop graphical user interface |
| Visual Studio | Development environment |
| Git and GitHub | Version control and collaboration |
| vcpkg | C++ dependency management |

## 6. Project Modules

The application consists of four main modules:

1. **Barcode Generation:** Validates six-digit input and generates a Code 128 barcode.
2. **Camera Scanner:** Uses OpenCV to capture camera frames and decode barcodes.
3. **Image Scanner:** Reads an uploaded barcode image and extracts its numeric code.
4. **Graphical User Interface:** Uses SFML to provide the interface through which users access the application's functionality.

## 7. Project Structure

```text
ISE-Barcode-System/
│
├── BarcodeApp/
│   ├── include/
│   │   ├── generation/
│   │   ├── camera_scanner/
│   │   ├── image_scanner/
│   │   └── gui/
│   │
│   ├── src/
│   │   ├── generation/
│   │   ├── camera_scanner/
│   │   ├── image_scanner/
│   │   └── gui/
│   │
│   ├── tests/
│   ├── docs/
│   ├── main.cpp
│   ├── vcpkg.json
│   └── vcpkg-configuration.json
│
├── .gitignore
└── README.md
```

## 8. Development Team

| Team Member | Assigned Module |
|---|---|
| Iraj | Barcode Generation |
| Izzah | Image Upload and Decoding |
| Wajiha | Camera Scanning |
| Hiba | Graphical User Interface |

## 9. Development Status

- [x] Project repository and module structure established.
- [x] ZXing-C++ dependency configured.
- [x] Six-digit input validation implemented.
- [x] Code 128 barcode generation implemented and tested.
- [x] Image upload and decoding module developed.
- [x] Camera scanning module.
- [x] SFML graphical user interface completion.
- [x] Integration of all modules.
- [x] Complete system testing.

## 10. Version Control and Collaboration

The project uses Git and GitHub for collaborative development. Each team member works on a separate development branch. Once a module is ready, it can be reviewed and merged into the main branch through a Pull Request.

## 11. Future Improvements

- Improve error handling and input validation.
- Enhance the graphical interface.
- Improve barcode scanning reliability.
- Perform complete integration and system testing.
- Prepare the final application for deployment.

---

**Course:** Introduction to Software Engineering

**Project:** ISE Barcode System

**Programming Language:** C++
