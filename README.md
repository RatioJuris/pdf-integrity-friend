# PDF Integrity Friend

**Version:** 1.0-Beta  
**Publisher:** Ratio Juris  
**Repository:** https://github.com/RatioJuris/pdf-integrity-friend

---

## Overview

PDF Integrity Friend is a Windows desktop application for secure PDF signing, visible signature placement, document integrity verification, timestamping, and PDF permission management.

The application is designed for legal professionals, businesses, enterprises, government organizations, engineers, and compliance teams requiring a user-friendly PDF signing environment.

---

## Features

### PDF Viewer

- Open PDF documents
- Smooth navigation
- Large document support
- Page thumbnails
- Multi-page browsing
- High DPI support

### Visible Signatures

- Place signatures visually
- Multiple signatures per page
- Multiple pages supported
- Drag-and-drop placement
- Signature resize support
- Signature preview

### Signature Types

- Text signatures
- Image signatures
- Future handwritten signatures
- Future digital certificate signatures

### Timestamping

Planned RFC3161 support:

- DigiCert TSA
- Sectigo TSA
- GlobalSign TSA
- Entrust TSA
- Custom TSA endpoints

### Security Controls

Optional PDF restrictions:

- Disable Editing
- Disable Printing
- Disable Copy
- Disable Commenting
- Disable Annotation
- Disable Form Filling
- Disable Page Extraction

All security options are disabled by default.

### Certificates

Planned support:

- Windows Certificate Store
- PFX Files
- P12 Files

### Logging

- Application logs
- Signing logs
- TSA logs
- Error logs

### Themes

- Light Theme
- Dark Theme

---

## Supported Platforms

Current target:

- Windows 10 x86
- Windows 10 x64
- Windows 11 x64

---

## Technology Stack

### Framework

- Qt 6

### Libraries

- Qt Widgets
- Qt Network
- Qt PDF
- Qt PDF Widgets
- OpenSSL 3.x

### Build System

- CMake

### Compiler

- Microsoft Visual C++

---

## Repository Structure

```text
pdf-integrity-friend/

├── src/
├── res/
├── build/
├── release-latest/
├── .github/
│   └── workflows/
│
├── LICENSE.md
├── README.md
└── CMakeLists.txt
