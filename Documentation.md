# ISE Barcode Management System

A C++ desktop application that generates and scans six-digit **Code 128** barcodes. It has a simple graphical interface made with SFML.

This is **Increment 1** of our ISE project.

## Features

- **Generate Barcode:** enter a six-digit code and save its barcode as a `.bmp` image.
- **Image Scanner:** choose an image (`.bmp`, `.png`, `.jpg`, `.jpeg`) and read the code from the barcode in it.
- **Camera Scanner:** scan a barcode live using the laptop camera.
- **GUI:** a dashboard and sidebar to move between the three features.

## Technologies Used

| Purpose | Tool |
|---|---|
| Language | C++ |
| GUI | SFML |
| File open/save dialogs | Windows API (`windows.h`, `commdlg.h`) |
| Loading images for the image scanner | `stb_image.h` |
| Camera | OpenCV |
| Barcode reading from camera | ZXing-C++ |

The project works on **Windows** only.

## Project Structure

```
include/
  generation/BarcodeGenerator.h
  image_scanner/imageScanner.h
  image_scanner/stb_image.h
  camera_scanner/CameraScanner.h
  gui/MainWindow.h
src/
  generation/BarcodeGenerator.cpp
  image_scanner/imageScanner.cpp
  camera_scanner/CameraScanner.cpp
  gui/MainWindow.cpp
  main.cpp
README.md
```

## How It Works

The project has four modules. The GUI calls the other three modules, and they do not depend on each other.

```
              MainWindow (GUI)
        /          |            \
BarcodeGenerator  Image Scanner  CameraScanner
```

### 1. Barcode Generator (`BarcodeGenerator`)

Creates a Code 128 barcode image from a six-digit code.

1. Checks that the code has exactly 6 digits and does not start with 0.
2. Uses Code 128 **Code B**: starts with the Start B symbol (104) and converts each digit to a symbol value (`digit - 32`).
3. Calculates the checksum: `(104 + sum of symbol × position) % 103`.
4. Adds the Stop symbol (106).
5. Draws the black and white bars into a pixel array using the Code 128 pattern table.
6. Writes the pixels to a 24-bit `.bmp` file.

Function: `bool generateBarcode(const string& code, const string& outputPath)`. It returns `true` if the image was saved and `false` if something failed.

### 2. Image Scanner (`readBarcodeFromImage`)

Reads a Code 128 barcode from an image file. We wrote the decoder ourselves, without a barcode library.

1. Loads the image in grayscale using `stb_image`.
2. Takes up to 40 horizontal rows of pixels from the image.
3. Turns each row into black/white pixels and measures the width of every bar and space.
4. Compares the widths with the Code 128 patterns and picks the closest symbol.
5. Looks for Start symbol, data symbols, checksum and Stop symbol, and checks the checksum.
6. Also tries the row backwards, in case the barcode is upside down.
7. Accepts the result only if at least 2 rows give the same code and it has exactly 6 digits.

Function: `bool readBarcodeFromImage(string path, string& code, string& error)`. On success `code` has the six-digit code. On failure `error` has a message that is shown to the user.

### 3. Camera Scanner (`CameraScanner`)

Scans a barcode using the laptop camera.

1. Opens the default camera with OpenCV.
2. Takes frames one by one, converts them to grayscale and gives them to ZXing.
3. Accepts a barcode only if it is Code 128 and has exactly 6 digits (first digit not 0).
4. Shows the live video in a window. When a code is found, it is written on the video and the window closes.
5. Pressing **ESC** stops scanning.

Function: `string scanUsingCamera()`. It returns the code, or an empty string if nothing was scanned.

### 4. GUI (`MainWindow`)

A 1000 × 700 SFML window with these pages:

| Page | What it has |
|---|---|
| Dashboard | Buttons to open the three features |
| Generate Barcode | Input box, Generate and Save, New Barcode, barcode preview |
| Image Scanner | Choose Image, Scan Image, New Scan |
| Camera Scanner | Start / Scan Again, Clear Result |

Every page except the dashboard has a **Back to Dashboard** button. Success messages are shown in green and error messages in red.

Main functions:

- `run()` runs the main loop (handle events, then draw).
- `handleEvents()` checks for window close, mouse click and typing.
- `handleClick()` checks which button was clicked.
- `handleTyping()` lets the user type only digits (maximum 6) and use backspace.
- `draw()` and the `draw...()` functions draw the sidebar and the current page.
- `generateBarcode()`, `scanImage()`, `scanCamera()` call the other modules and show the result.

## Rules for the Code

- It must have **exactly 6 digits**.
- It must contain **only digits**.
- The **first digit cannot be 0**.

The generator and the camera scanner check all three rules. The GUI only allows digits in the input box and stops at 6 characters. The image scanner checks that the code has exactly 6 digits.

## Software Principles Used

| Principle | Where we used it |
|---|---|
| **Modularity** | The project is divided into four modules (generator, image scanner, camera scanner, GUI). Each has its own `.h` and `.cpp` file. |
| **Separation of concerns** | Barcode logic is separate from the GUI. The GUI only shows results. |
| **Single responsibility** | Each class/function does one job, e.g. `matchSymbol` only matches one symbol, `drawButton` only draws a button, `writeU32` only writes a number to the file. |
| **Abstraction** | The GUI uses simple calls like `generateBarcode()` and `scanUsingCamera()` without knowing how they work inside. |
| **Encapsulation / information hiding** | Class variables and helper functions are `private`. Helper functions in `imageScanner.cpp` are `static` so other files cannot use them. |
| **Low coupling** | The three feature modules do not depend on each other. Only `MainWindow` uses them. |
| **High cohesion** | Everything inside a module is about one feature. |
| **Header / source separation** | Declarations are in `include/` and implementations are in `src/`. Headers use `#pragma once`. |
| **Data-driven design** | The Code 128 patterns are stored in a table, so we did not write separate code for each symbol. |
| **Event-driven programming** | The GUI waits for events (click, typing, close) and reacts to them. |
| **State-based navigation** | The variable `currentPage` decides which page is shown. |
| **KISS (keep it simple)** | Simple arrays, simple functions and a simple BMP writer. |
| **Input validation** | Length, digits and first-digit checks before generating or accepting a code. |
| **Fail fast (early return)** | Functions return as soon as the input is invalid. |
| **Error handling** | Functions return `true`/`false`, the image scanner gives an error message, and the camera code uses `try/catch`. |
| **Data integrity** | The modulo 103 checksum is created in the generator and checked in the image scanner. |
| **Resource management** | Files, the camera and the image memory are always released after use (`file.close()`, `camera.release()`, `stbi_image_free`, `delete[]`). |
| **Reuse of libraries** | SFML, OpenCV, ZXing and stb_image are used instead of writing everything from scratch. |
| **Const correctness / pass by reference** | Inputs are passed as `const string&` and outputs as references. |
| **Meaningful names and comments** | Clear variable names and a comment above each function and main step. |
| **User feedback** | Green/red result messages, preview of the barcode, and Clear/New/Back buttons. |

## How to Build and Run

### Requirements

- Windows with Visual Studio (C++)
- SFML 2.x
- OpenCV 4.x
- ZXing-C++
- `stb_image.h` in `include/image_scanner/`
- A webcam (only for the camera scanner)

### Steps

1. Install the libraries (for example with vcpkg):
   ```
   vcpkg install sfml:x64-windows opencv4:x64-windows zxing-cpp:x64-windows
   vcpkg integrate install
   ```
2. Open the project in Visual Studio and add all `.cpp` files from `src/`.
3. Make sure the `include/` folder is in the include path.
4. Build in **x64** mode and run.

`main.cpp` only needs to create the window and run it:

```cpp
#include "../include/gui/MainWindow.h"

int main()
{
    MainWindow app;
    app.run();
    return 0;
}
```

## How to Use

**Generate a barcode**
1. Open *Generate Barcode*.
2. Type a six-digit code (first digit not 0).
3. Click *Generate and Save* and choose where to save the `.bmp` file.
4. The barcode appears on the screen. Click *New Barcode* to start again.

**Scan an image**
1. Open *Image Scanner* and click *Choose Image*.
2. Select an image with a barcode.
3. Click *Scan Image* to see the code. Click *New Scan* to reset.

**Scan with the camera**
1. Open *Camera Scanner* and click *Start / Scan Again*.
2. Show a barcode to the camera. The code is shown when it is read.
3. Press **ESC** to stop.

## Testing

| ID | Module | Test Case | Input | Expected Output |
|---|---|---|---|---|
| T1 | Generator | Valid code | `123456` | Barcode is saved as `.bmp`, preview is shown, green success message |
| T2 | Generator | Too short | `12345` | Error: "Enter exactly six digits." |
| T3 | Generator | First digit is zero | `012345` | Error: "The first digit cannot be zero." |
| T4 | Generator | Save dialog cancelled | Click Cancel in the save window | Message: "Save cancelled." |
| T5 | GUI input | Letters typed in the input box | `abc123` | Letters are ignored, only `123` appears |
| T6 | GUI input | More than 6 digits typed | `12345678` | Only the first 6 digits (`123456`) are accepted |
| T7 | Image Scanner | Barcode image made by our generator | Image of `123456` | Scanned code: `123456` |
| T8 | Image Scanner | Upside-down barcode | Barcode image rotated 180° | Scanned code is read correctly |
| T9 | Image Scanner | Blurry or cropped barcode | Blurry image | Error: "Could not read a barcode..." |
| T10 | Image Scanner | No image selected | Click Scan Image without choosing a file | Message: "Please choose an image first." |
| T11 | Image Scanner | Image without a barcode | Any normal photo | Error: "Could not read a barcode..." |
| T12 | Camera Scanner | Valid barcode in front of camera | Barcode of `123456` | Scanned code: `123456` |
| T13 | Camera Scanner | ESC pressed before scanning | Press ESC | Message: "No valid barcode was scanned." |
| T14 | Camera Scanner | No camera available | Camera disconnected or in use | Error message is shown, program does not crash |
| T15 | GUI | Navigation | Click each sidebar item and Back to Dashboard | Correct page opens each time, old messages are cleared |
| T16 | Full system | Generate, then scan | Generate `123456`, then scan the saved image | Scanned code matches the input code `123456` |

## Team Members

| Name | Work |
|---|---|
| Iraj | Barcode generation |
| Izzah | Image scanning |
| Wajiha | Camera scanner |
| Hiba | GUI |
