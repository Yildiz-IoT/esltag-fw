/**
 * @file barcode_example.c
 * @brief Example usage of the barcode encoder library
 * @author ESL Project Team
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "barcode_encoder.h"

// Default display dimensions if not specified by user
#define DEFAULT_WIDTH 296
#define DEFAULT_HEIGHT 128

// Function to print bitmap for debug purposes
void print_bitmap(uint8_t* bitmap, uint16_t width, uint16_t height) {
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            uint32_t pos = y * width + x;
            uint32_t byte_pos = pos / 8;
            uint8_t bit_pos = 7 - (pos % 8);
            
            if (bitmap[byte_pos] & (1 << bit_pos)) {
                printf("█"); // Black pixel
            } else {
                printf(" "); // White pixel
            }
        }
        printf("\n");
    }
}

/**
 * @brief Debug function to print string details
 * 
 * @param label Label for debug output
 * @param str String to inspect
 */
void debug_string(const char* label, const char* str) {
    printf("DEBUG - %s:\n", label);
    printf("  Address: %p\n", (void*)str);
    printf("  Content: \"%s\"\n", str ? str : "NULL");
    
    if (str) {
        printf("  Strlen: %zu\n", strlen(str));
        printf("  Bytes: [");
        for (size_t i = 0; i < strlen(str); i++) {
            printf("%02X", (unsigned char)str[i]);
            if (i < strlen(str) - 1) printf(" ");
        }
        printf("]\n");
        
        // Check for hidden characters or null terminators
        printf("  Char dump: [");
        for (size_t i = 0; i < strlen(str) + 3 && i < 20; i++) {
            if (str[i] >= 32 && str[i] <= 126) {
                printf("'%c'", str[i]);
            } else {
                printf("\\x%02X", (unsigned char)str[i]);
            }
            
            if (i < strlen(str) + 2) printf(" ");
        }
        printf("]\n");
    }
}

/**
 * @brief Save bitmap as BMP file
 * 
 * @param filename Output filename
 * @param bitmap Bitmap buffer
 * @param width Width of the bitmap in pixels
 * @param height Height of the bitmap in pixels
 * @return true if successful, false if failed
 */
bool save_bitmap_as_bmp(const char* filename, uint8_t* bitmap, uint16_t width, uint16_t height) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file for writing: %s\n", filename);
        return false;
    }
    
    // BMP file header (14 bytes)
    uint8_t file_header[14] = {
        'B', 'M',           // Signature
        0, 0, 0, 0,         // File size in bytes (filled in later)
        0, 0, 0, 0,         // Reserved
        54 + 8, 0, 0, 0     // Offset to pixel data (54 bytes for headers + 8 bytes for color table)
    };
    
    // DIB header (40 bytes) - BITMAPINFOHEADER
    uint8_t dib_header[40] = {
        40, 0, 0, 0,        // DIB header size
        0, 0, 0, 0,         // Width in pixels (filled in later)
        0, 0, 0, 0,         // Height in pixels (filled in later)
        1, 0,               // Number of color planes
        1, 0,               // Bits per pixel (1 for monochrome)
        0, 0, 0, 0,         // Compression method (0 for none)
        0, 0, 0, 0,         // Image size (can be 0 for uncompressed)
        0, 0, 0, 0,         // Horizontal resolution (pixels per meter)
        0, 0, 0, 0,         // Vertical resolution (pixels per meter)
        2, 0, 0, 0,         // Number of colors in palette (2 for monochrome)
        0, 0, 0, 0          // Number of important colors (0 means all)
    };
    
    // Calculate padded row size (BMP rows must be multiple of 4 bytes)
    uint32_t row_size_bits = width;
    uint32_t row_size_bytes = (row_size_bits + 7) / 8;
    uint32_t padded_row_size = (row_size_bytes + 3) & ~3; // Align to 4 bytes
    
    // Calculate total size
    uint32_t pixel_data_size = padded_row_size * height;
    uint32_t file_size = 14 + 40 + 8 + pixel_data_size; // Headers + color table + pixel data
    
    // Fill in file size in file header
    file_header[2] = (uint8_t)(file_size);
    file_header[3] = (uint8_t)(file_size >> 8);
    file_header[4] = (uint8_t)(file_size >> 16);
    file_header[5] = (uint8_t)(file_size >> 24);
    
    // Fill in width in DIB header
    dib_header[4] = (uint8_t)(width);
    dib_header[5] = (uint8_t)(width >> 8);
    dib_header[6] = (uint8_t)(width >> 16);
    dib_header[7] = (uint8_t)(width >> 24);
    
    // Fill in height in DIB header (negative for top-down image)
    int32_t signed_height = -((int32_t)height); // Negative for top-down image
    dib_header[8] = (uint8_t)(signed_height);
    dib_header[9] = (uint8_t)(signed_height >> 8);
    dib_header[10] = (uint8_t)(signed_height >> 16);
    dib_header[11] = (uint8_t)(signed_height >> 24);
    
    // Write headers
    fwrite(file_header, 1, 14, file);
    fwrite(dib_header, 1, 40, file);
    
    // Write color table (2 entries for monochrome: black and white)
    // White (0) - BGR0
    uint8_t color_table[8] = {
        255, 255, 255, 0,  // White (0 in bitmap)
        0, 0, 0, 0         // Black (1 in bitmap)
    };
    fwrite(color_table, 1, 8, file);
    
    // Prepare buffer for padded row
    uint8_t* row_buffer = (uint8_t*)malloc(padded_row_size);
    if (!row_buffer) {
        fclose(file);
        return false;
    }
    
    // Write pixel data - monochrome bitmap is 1 bit per pixel
    for (uint16_t y = 0; y < height; y++) {
        // Clear row buffer
        memset(row_buffer, 0, padded_row_size);
        
        // Fill row buffer with image data
        for (uint16_t x = 0; x < width; x++) {
            uint32_t pos = y * width + x;
            uint32_t byte_pos = pos / 8;
            uint8_t bit_pos = 7 - (pos % 8);
            
            // Check if pixel is set
            if (bitmap[byte_pos] & (1 << bit_pos)) {
                // Set corresponding bit in row buffer
                uint32_t row_byte = x / 8;
                uint8_t row_bit = 7 - (x % 8);
                row_buffer[row_byte] |= (1 << row_bit);
            }
        }
        
        // Write row to file
        fwrite(row_buffer, 1, padded_row_size, file);
    }
    
    // Clean up and close
    free(row_buffer);
    fclose(file);
    
    printf("BMP file saved: %s\n", filename);
    return true;
}

/**
 * @brief Print usage information
 */
void print_usage(const char* program_name) {
    printf("Usage: %s [width] [height]\n", program_name);
    printf("Generate EAN-13 and UPC-A barcodes and save as BMP files\n\n");
    printf("Options:\n");
    printf("  width   Width of the bitmap in pixels (default: %d)\n", DEFAULT_WIDTH);
    printf("  height  Height of the bitmap in pixels (default: %d)\n", DEFAULT_HEIGHT);
    printf("\nExamples:\n");
    printf("  %s                 # Use default resolution\n", program_name);
    printf("  %s 400 200         # Use 400x200 resolution\n", program_name);
    printf("  %s 250 100         # Use 250x100 resolution\n", program_name);
}

// Main example function
int main(int argc, char* argv[]) {
    uint16_t width = DEFAULT_WIDTH;
    uint16_t height = DEFAULT_HEIGHT;
    
    // Parse command line arguments
    if (argc > 1) {
        // Check if help is requested
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        
        // Try to parse width
        char* endptr;
        long parsed_width = strtol(argv[1], &endptr, 10);
        if (*endptr == '\0' && parsed_width > 0 && parsed_width <= 65535) {
            width = (uint16_t)parsed_width;
        } else {
            printf("Error: Invalid width '%s'\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (argc > 2) {
        // Try to parse height
        char* endptr;
        long parsed_height = strtol(argv[2], &endptr, 10);
        if (*endptr == '\0' && parsed_height > 0 && parsed_height <= 65535) {
            height = (uint16_t)parsed_height;
        } else {
            printf("Error: Invalid height '%s'\n", argv[2]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    printf("Using resolution: %dx%d pixels\n", width, height);
    
    // Allocate bitmap buffer (1 bit per pixel)
    uint32_t buffer_size = (width * height + 7) / 8;
    uint8_t* bitmap = (uint8_t*)malloc(buffer_size);
    
    if (!bitmap) {
        printf("Failed to allocate bitmap memory\n");
        return -1;
    }
    
    // Clear bitmap
    memset(bitmap, 0, buffer_size);
    
    // Example EAN-13 barcode
    const char* ean13_code = "59012341234570"; // Valid EAN-13 with checksum
    
    // Example UPC-A barcode
    const char* upca_code = "042100005264"; // Valid UPC-A with checksum
    
    // Also test with invalid barcode to demonstrate error handling
    const char* invalid_code = "12345"; // Too short, will generate error
    
    // Debug string information - this will help diagnose length issues
    printf("\n=== String Diagnostics ===\n");
    debug_string("EAN-13 code", ean13_code);
    debug_string("UPC-A code", upca_code);
    
    // Fix for Windows: Create explicit copies with guaranteed null termination
    char ean13_fixed[14];
    char upca_fixed[13];
    
    strncpy(ean13_fixed, ean13_code, 13);
    ean13_fixed[13] = '\0'; // Ensure null termination
    
    strncpy(upca_fixed, upca_code, 12);
    upca_fixed[12] = '\0'; // Ensure null termination
    
    debug_string("EAN-13 fixed", ean13_fixed);
    debug_string("UPC-A fixed", upca_fixed);
    
    // Render EAN-13 barcode (using fixed copy)
    printf("\n=== Testing EAN-13 barcode ===\n");
    printf("Code: %s\n", ean13_fixed);
    
    barcode_error_t result = barcode_render(ean13_fixed, BARCODE_EAN13, bitmap, 
                           width, height/2, false);
    
    if (result == BARCODE_SUCCESS) {
        printf("EAN-13 barcode rendered successfully\n");
        
        // Save EAN-13 barcode as BMP file
        char filename[32];
        sprintf(filename, "ean13_%dx%d.bmp", width, height/2);
        save_bitmap_as_bmp(filename, bitmap, width, height/2);
        
        // Only print to console if the bitmap is small enough
        if (width <= 100) {
            print_bitmap(bitmap, width, height/2);
        } else {
            printf("Bitmap too large to display in console\n");
        }
    } else {
        printf("Failed to render EAN-13 barcode: %s (code %d)\n", 
               barcode_get_error_message(result), result);
    }
    
    // Clear bitmap for next example
    memset(bitmap, 0, buffer_size);
    
    // Render UPC-A barcode (using fixed copy)
    printf("\n=== Testing UPC-A barcode ===\n");
    printf("Code: %s\n", upca_fixed);
    
    result = barcode_render(upca_fixed, BARCODE_UPCA, bitmap, 
                         width, height/2, false);
    
    if (result == BARCODE_SUCCESS) {
        printf("UPC-A barcode rendered successfully\n");
        
        // Save UPC-A barcode as BMP file
        char filename[32];
        sprintf(filename, "upca_%dx%d.bmp", width, height/2);
        save_bitmap_as_bmp(filename, bitmap, width, height/2);
        
        // Only print to console if the bitmap is small enough
        if (width <= 100) {
            print_bitmap(bitmap, width, height/2);
        } else {
            printf("Bitmap too large to display in console\n");
        }
    } else {
        printf("Failed to render UPC-A barcode: %s (code %d)\n", 
               barcode_get_error_message(result), result);
    }
    
    // Test with invalid barcode
    printf("\n=== Testing Invalid barcode ===\n");
    printf("Code: %s\n", invalid_code);
    
    result = barcode_render(invalid_code, BARCODE_UPCA, bitmap, 
                         width, height/2, false);
    
    if (result == BARCODE_SUCCESS) {
        printf("Barcode rendered successfully (unexpected!)\n");
    } else {
        printf("Failed to render barcode (expected): %s (code %d)\n", 
               barcode_get_error_message(result), result);
    }
    
    // Test error description function with all possible error codes
    printf("\n=== Error Code Table ===\n");
    printf("%-4s | %s\n", "Code", "Description");
    printf("-----|-------------\n");
    for (int i = 0; i >= -7; i--) {
        printf("%-4d | %s\n", i, barcode_get_error_message((barcode_error_t)i));
    }
    
    // Free bitmap memory
    free(bitmap);
    
    return 0;
} 