/**
 * @file barcode_encoder.h
 * @brief Lightweight barcode encoder for EAN-13 and UPC-A
 * @author ESL Project Team
 *
 * This library provides functions to generate EAN-13 and UPC-A barcodes
 * directly to bitmap buffers for e-ink displays. Implementation is optimized
 * for low memory usage on nRF52 microcontrollers.
 */

#ifndef BARCODE_ENCODER_H
#define BARCODE_ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Barcode types supported by the encoder
 */
typedef enum {
	BARCODE_EAN13, /**< EAN-13 format (13 digits) */
	BARCODE_UPCA   /**< UPC-A format (12 digits) */
} barcode_type_t;

/**
 * @brief Error codes for barcode operations
 */
typedef enum {
	BARCODE_SUCCESS = 0,		     /**< Operation completed successfully */
	BARCODE_ERROR_NULL_INPUT = -1,	     /**< NULL pointer was provided as input */
	BARCODE_ERROR_INVALID_TYPE = -2,     /**< Invalid barcode type specified */
	BARCODE_ERROR_INVALID_LENGTH = -3,   /**< Invalid digit string length */
	BARCODE_ERROR_NON_DIGIT = -4,	     /**< Non-digit character in input string */
	BARCODE_ERROR_INVALID_CHECKSUM = -5, /**< Checksum validation failed */
	BARCODE_ERROR_BITMAP_TOO_SMALL = -6, /**< Bitmap dimensions too small for barcode */
	BARCODE_ERROR_MEMORY = -7,	     /**< Memory allocation or access error */
	BARCODE_ERROR_UNKNOWN = -99	     /**< Unknown error occurred */
} barcode_error_t;

/**
 * @brief Render a barcode to a bitmap buffer
 *
 * @param digits String containing digits to encode
 *               (12 digits for UPC-A, 13 digits for EAN-13)
 * @param type Barcode type (BARCODE_EAN13 or BARCODE_UPCA)
 * @param bitmap Buffer to receive the rendered bitmap
 * @param scan_mode Mode of barcode orientation (0=normal, 6=transposed & flipped)
 *        For scan_mode=6, the output is transposed (width and height swapped)
 *        and horizontally flipped, which is required for certain E-ink displays
 * @param width Width of the bitmap in pixels
 * @param height Height of the bitmap in pixels
 * @param include_text Set true to render digits below the barcode
 * @return barcode_error_t Error code (BARCODE_SUCCESS if successful)
 *
 * @note The function validates input parameters and barcode digits.
 *       The bitmap buffer must be at least (width * height / 8) bytes.
 *       For a monochrome display, each bit in the bitmap represents one pixel.
 *       When using scan_mode=6, the output dimensions are swapped (height,width).
 */
barcode_error_t barcode_render(const char *digits, barcode_type_t type, uint8_t *bitmap,
			       uint8_t scan_mode, uint16_t width, uint16_t height,
			       bool include_text);

/**
 * @brief Calculate the checksum digit for EAN-13 or UPC-A
 *
 * @param digits Pointer to the digits array (without checksum)
 * @param len Length of the digits array
 * @param checksum Pointer to receive the calculated checksum digit
 * @return barcode_error_t Error code (BARCODE_SUCCESS if successful)
 */
barcode_error_t barcode_calculate_checksum(const char *digits, uint8_t len, uint8_t *checksum);

/**
 * @brief Verify that a complete barcode including checksum is valid
 *
 * @param digits Pointer to the full digits array (including checksum)
 * @param len Length of the digits array
 * @return barcode_error_t Error code (BARCODE_SUCCESS if valid)
 */
barcode_error_t barcode_verify(const char *digits, uint8_t len);

/**
 * @brief Get an error message string for the given error code
 *
 * @param error Error code
 * @return const char* Error message string
 */
const char *barcode_get_error_message(barcode_error_t error);

#endif /* BARCODE_ENCODER_H */