# Product Requirements Document (PRD): JSON-Based ESL Display Update System

## 1. Introduction

### 1.1 Purpose
This document specifies the requirements for implementing a JSON-based display update system for Electronic Shelf Labels (ESLs). The system aims to provide efficient updates to ESL displays using structured JSON data rather than full binary images.

### 1.2 Scope
This PRD covers the JSON format definition, rendering specifications, memory requirements, and implementation guidelines for both the ESL tag firmware and the transmission system.

### 1.3 Objectives
- Reduce data transmission size compared to binary image updates
- Enable partial updates of specific display elements
- Standardize display layouts while allowing flexibility
- Maximize battery life through efficient updates
- Support multiple font styles, sizes, and colors

## 2. JSON Format Specification

### 2.1 Basic Structure
```json
{
  "prod": {"text": "Product Name", "pos": [100,100], "format": [bold, size, style], "color": 0},
  "brnd": {"text": "Brand Name", "pos": [100,100], "format": [bold, size, style], "color": 0},
  "price": {"text": "999.99", "pos": [100,100], "format": [bold, size, style], "color": 2},
  "unit": {"text": "99.99 / 999ml", "pos": [100,100], "format": [bold, size, style], "color": 0},
  "exp": {"text": "12/31/99", "pos": [100,100], "format": [bold, size, style], "color": 0},
  "barcode": {"value": "012345678901", "pos": [100,100], "type": "EAN-13", "color": 0}
}
```

### 2.2 Field Definitions
| Field | Description | Max Length | Required |
|-------|-------------|------------|----------|
| prod | Product name | 40 chars | Yes |
| brnd | Brand name | 30 chars | No |
| price | Price value | 6 chars | Yes |
| unit | Unit price | 20 chars | No |
| exp | Expiration date | 8 chars | No |
| barcode | Barcode value | 12-13 chars | No |

### 2.3 Format Array Definition
The `format` array contains three integers:
- Index 0: Bold (0 = normal, 1 = bold)
- Index 1: Size (0 = small, 1 = normal, 2 = large)
- Index 2: Style (0 = normal, 1 = strikethrough)

### 2.4 Position Definition
The `pos` array defines [x,y] coordinates in pixels from the top-left corner of the display.

### 2.5 Color Definition
The `color` value defines the text color:
- 0: Black (default)
- 1: White
- 2: Red (for displays with red capability)

### 2.6 Update Mode
Partial updates are supported implicitly:
- If a field is included in the JSON, it will be updated
- If a field is omitted, its current display state will be preserved

## 3. Font Specifications

### 3.1 Font Types
The system will support a single monospace font type to simplify rendering and positioning.

### 3.2 Font Sizes
| Size Value | Height (pixels) | Width per character (pixels) |
|------------|-----------------|------------------------------|
| 0 (Small) | 12 | 6 |
| 1 (Normal) | 16 | 8 |
| 2 (Large) | 24 | 12 |

### 3.3 Character Set
The system will support:
- ASCII characters (32-126)
- Basic Latin-1 Supplement characters for currency symbols
- Total characters: approximately 100

### 3.4 Font Memory Requirements
- Small font: ~1KB
- Normal font: ~2KB
- Large font: ~4KB
- Bold variants: additional ~7KB
- Total font storage: ~14KB

## 4. Font Library Implementation

### 4.1 Font Library Selection
- The system shall use a lightweight bitmap font library suitable for embedded systems
- Recommended libraries:
  - Adafruit GFX Font Library (minimal memory footprint)
  - u8g2 Monochrome Graphics Library (comprehensive but larger)
  - Custom bitmap font implementation (optimized for ESL needs)

### 4.2 Font Storage Format
- Fonts shall be stored as bitmap arrays in flash memory
- Each character shall be represented as a bitmap with 1 bit per pixel
- Font data shall be arranged to optimize rendering speed on ESL displays
- Font data structure:
  ```c
  typedef struct {
      uint8_t width;       // Character width in pixels
      uint8_t height;      // Character height in pixels
      uint8_t first_char;  // ASCII code of first character in set
      uint8_t last_char;   // ASCII code of last character in set
      const uint8_t *data; // Bitmap data
  } font_info_t;
  ```

### 4.3 Font Rendering
- The font library shall provide efficient functions for:
  - Rendering a single character at specified coordinates
  - Rendering a string with proper character spacing
  - Calculating text width for positioning
- Rendering shall support all defined format options (bold, sizes, strikethrough)
- Font rendering functions shall handle display boundaries correctly
- Optimization for common ESL display controllers (SSD1306, IL3820, etc.)

### 4.4 Font Customization
- Support for adding custom symbols (currency, units, etc.)
- Support for generating font data from standard font formats (TTF)
- Tools for converting and optimizing fonts for ESL use

## 5. Rendering Requirements

### 5.1 Text Rendering
- Text shall be rendered left-to-right, top-to-bottom
- The `pos` coordinates indicate the top-left corner of the first character
- Overflow text that would exceed display boundaries shall be truncated

### 5.2 Barcode Rendering
- Supported types: EAN-13, UPC-A, CODE-128
- Barcode shall be rendered with appropriate quiet zones
- Barcode height shall be standardized at 50 pixels

### 5.3 Display Update Strategy
- Display shall be updated in a single refresh cycle when possible
- For displays supporting partial refresh, only modified regions shall be updated
- Full display refreshes shall be performed every 10 partial updates to clear artifacts

## 6. Memory Requirements

### 6.1 RAM Requirements
- JSON parsing buffer: 512 bytes
- Display buffer: Based on display resolution (e.g., 4KB for 296×128 pixels)
- Rendering work buffer: 1KB
- Total minimum RAM: ~6KB plus display buffer

### 6.2 Flash Requirements
- Font storage: ~14KB
- Barcode generation code: ~3KB
- JSON parser: ~2KB
- Rendering engine: ~4KB
- State storage: ~1KB
- Total additional flash required: ~24KB

## 7. Performance Requirements

### 7.1 Parse Time
- JSON parsing shall complete within 100ms
- Overall update processing shall complete within 200ms (excluding display refresh time)

### 7.2 Power Consumption
- Processing a JSON update shall consume <5mA for <500ms
- System shall support low-power sleep between updates

## 8. Error Handling

### 8.1 JSON Parsing Errors
- Malformed JSON shall be rejected entirely
- Error code shall be logged for diagnostics

### 8.2 Field Validation
- Text exceeding maximum length shall be truncated
- Invalid format values shall default to [0,1,0]
- Invalid positions shall be clamped to display boundaries
- Invalid colors shall default to black (0)

### 8.3 Missing Required Fields
- If required fields are missing, the update shall be rejected
- Current display contents shall be preserved

## 9. Implementation Guidelines

### 9.1 JSON Parser Selection
- Use a lightweight JSON parser designed for embedded systems
- Consider jsmn, cJSON, or PJSON for minimal overhead

### 9.2 Font Implementation
- Store font data as bitmap arrays in flash memory
- Implement character lookup tables for efficient rendering

### 9.3 State Management
- Store current state of all fields in non-volatile memory
- Implement power-fail protection for partial updates

## 10. Future Considerations

### 10.1 Format Extensions
- Consider adding template support in future versions
- Evaluate text alignment options based on usage feedback
- Consider adding a version field for future protocol updates

### 10.2 Additional Features
- Investigate support for simple vector graphics
- Consider QR code support in addition to barcodes
- Evaluate options for animated transitions between updates

## 11. Testing Requirements

### 11.1 Validation Tests
- Test suite shall verify correct rendering of all font sizes and styles
- Test suite shall verify barcode generation accuracy
- Test suite shall verify correct handling of all error conditions

### 11.2 Performance Tests
- Measure and verify parsing and rendering performance
- Measure power consumption during updates
- Test partial update functionality

## 12. Implementation Roadmap

### 12.1 Phase 1: Core Implementation
- Implement JSON parser integration
- Implement basic text rendering for all font sizes
- Support for black pixels only

### 12.2 Phase 2: Enhanced Features
- Add barcode generation
- Add color support (red highlights)
- Implement partial update optimization

### 12.3 Phase 3: Optimization
- Memory usage optimization
- Power consumption optimization
- Performance tuning

## 13. ToDo
- [ ] Research and select appropriate JSON parser for embedded systems
- [ ] Design font bitmap storage format
- [ ] Select or develop appropriate font library
- [ ] Create font conversion tools if needed
- [ ] Implement text rendering function for all font sizes
- [ ] Implement barcode generation algorithm
- [ ] Design state storage format for non-volatile memory
- [ ] Implement JSON validation and error handling
- [ ] Create test suite for validation testing
- [ ] Optimize memory usage
- [ ] Implement partial update mechanism
- [ ] Develop power consumption tests

## 14. Done
- [x] Define JSON format specification
- [x] Document font requirements and specifications
- [x] Create initial PRD document 

| Version | Date | Description |
|---------|------|-------------|
| 0.1 | 2024-06-17 | Initial draft |
