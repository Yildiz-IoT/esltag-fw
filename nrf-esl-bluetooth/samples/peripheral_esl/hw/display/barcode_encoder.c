/**
 * @file barcode_encoder.c
 * @brief Implementation of EAN-13 and UPC-A barcode encoder
 * @author ESL Project Team
 *
 * This implementation is optimized for memory usage on constrained devices.
 * The barcode patterns are stored in compact arrays, and bitmap operations
 * are optimized to minimize processing requirements.
 */

#include "barcode_encoder.h"

/******************************************************************************
 * Constants and Pattern Tables
 ******************************************************************************/

/**
 * EAN/UPC Encoding patterns
 *
 * L (Left) patterns encode the left side digits in EAN/UPC
 * G patterns are inverted versions of the L patterns, used for EAN-13
 * R (Right) patterns encode the right side digits in EAN/UPC
 *
 * Each pattern is 7 modules (bits) long:
 * - 1 represents a black bar
 * - 0 represents a white space
 *
 * We store them as bytes where only the lowest 7 bits are used.
 */

/** Left pattern table (odd parity) */
static const uint8_t PATTERN_L[10] = {
	0b0001101, // 0
	0b0011001, // 1
	0b0010011, // 2
	0b0111101, // 3
	0b0100011, // 4
	0b0110001, // 5
	0b0101111, // 6
	0b0111011, // 7
	0b0110111, // 8
	0b0001011  // 9
};

/** G pattern table (even parity - inverted L patterns) */
static const uint8_t PATTERN_G[10] = {
	0b0100111, // 0
	0b0110011, // 1
	0b0011011, // 2
	0b0100001, // 3
	0b0011101, // 4
	0b0111001, // 5
	0b0000101, // 6
	0b0010001, // 7
	0b0001001, // 8
	0b0010111  // 9
};

/** Right pattern table (all digits use R patterns on the right side) */
static const uint8_t PATTERN_R[10] = {
	0b1110010, // 0
	0b1100110, // 1
	0b1101100, // 2
	0b1000010, // 3
	0b1011100, // 4
	0b1001110, // 5
	0b1010000, // 6
	0b1000100, // 7
	0b1001000, // 8
	0b1110100  // 9
};

/**
 * EAN-13 first digit encoding table
 * Defines which pattern (L or G) to use for the first 6 digits
 * 0 = pattern L, 1 = pattern G
 */
static const uint8_t EAN13_FIRST_DIGIT_PARITY[10][6] = {
	{0, 0, 0, 0, 0, 0}, // 0
	{0, 0, 1, 0, 1, 1}, // 1
	{0, 0, 1, 1, 0, 1}, // 2
	{0, 0, 1, 1, 1, 0}, // 3
	{0, 1, 0, 0, 1, 1}, // 4
	{0, 1, 1, 0, 0, 1}, // 5
	{0, 1, 1, 1, 0, 0}, // 6
	{0, 1, 0, 1, 0, 1}, // 7
	{0, 1, 0, 1, 1, 0}, // 8
	{0, 1, 1, 0, 1, 0}  // 9
};

/** Guard patterns */
#define GUARD_PATTERN_NORMAL 0b101   // Start and end guard
#define GUARD_PATTERN_CENTER 0b01010 // Center guard

/** Pattern widths */
#define DIGIT_WIDTH	   7 // Width of each digit pattern in modules
#define GUARD_WIDTH_NORMAL 3 // Width of normal guard in modules
#define GUARD_WIDTH_CENTER 5 // Width of center guard in modules

/** UPC-A quiet zone (recommended minimum) */
#define QUIET_ZONE_MODULES 9 // Minimum quiet zone width in modules

/** Character to module conversion (for validation) */
#define CHAR_TO_DIGIT(c) ((c) - '0')

/** Error message strings */
static const char *ERROR_MESSAGES[] = {
	"Success",			     // BARCODE_SUCCESS
	"NULL pointer provided as input",    // BARCODE_ERROR_NULL_INPUT
	"Invalid barcode type",		     // BARCODE_ERROR_INVALID_TYPE
	"Invalid digit string length",	     // BARCODE_ERROR_INVALID_LENGTH
	"Non-digit character in input",	     // BARCODE_ERROR_NON_DIGIT
	"Invalid checksum",		     // BARCODE_ERROR_INVALID_CHECKSUM
	"Bitmap dimensions too small",	     // BARCODE_ERROR_BITMAP_TOO_SMALL
	"Memory allocation or access error", // BARCODE_ERROR_MEMORY
	"Unknown error"			     // BARCODE_ERROR_UNKNOWN
};

/******************************************************************************
 * Static Helper Functions
 ******************************************************************************/

/**
 * @brief Set a pixel in a bitmap buffer
 *
 * @param bitmap Pointer to the bitmap buffer
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Total width of the bitmap in pixels
 */
static void set_pixel(uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t width)
{
	// // Calculate byte position and bit offset
	// size_t pos = y * width + x;
	// uint32_t byte_pos = pos / 8;
	// uint8_t bit_pos = 7 - (pos % 8); // MSB first layout (0 is leftmost bit)

	// // Set the bit
	// bitmap[byte_pos] |= (1 << bit_pos);

	uint16_t byte_row = y / 8;		    // Row of bytes (each byte is 8 vertical pixels)
	uint16_t byte_index = byte_row * width + x; // Byte index in the bitmap
	uint8_t bit_pos = 7 - (y % 8);		    // Bit position in the byte (MSB is top)

	bitmap[byte_index] |= (1 << bit_pos); // Set the bit to turn on the pixel
}

/**
 * @brief Draw a vertical bar in the bitmap
 *
 * @param bitmap Pointer to the bitmap buffer
 * @param x X coordinate (left edge of bar)
 * @param width Width of the bar in pixels
 * @param height Height of the bar in pixels
 * @param bitmap_width Total width of the bitmap
 */
static void draw_bar(uint8_t *bitmap, uint16_t x, uint16_t width, uint16_t height,
		     uint16_t bitmap_width)
{
	for (uint16_t i = 0; i < width; i++) {
		for (uint16_t j = 0; j < height; j++) {
			set_pixel(bitmap, x + i, j, bitmap_width);
		}
	}
}

/**
 * @brief Validate that a string contains only digits
 *
 * @param digits String to validate
 * @param len Expected length
 * @return barcode_error_t Error code
 */
static barcode_error_t validate_digits(const char *digits, uint8_t len)
{
	// Check for NULL pointer
	if (digits == NULL) {
		return BARCODE_ERROR_NULL_INPUT;
	}

	// Check string length
	size_t actual_len = strlen(digits);
	if (actual_len != len) {
		return BARCODE_ERROR_INVALID_LENGTH;
	}

	// Check that all characters are digits
	for (uint8_t i = 0; i < len; i++) {
		if (digits[i] < '0' || digits[i] > '9') {
			return BARCODE_ERROR_NON_DIGIT;
		}
	}

	return BARCODE_SUCCESS;
}

/**
 * @brief Draw a digit pattern to the bitmap
 *
 * @param bitmap Pointer to bitmap buffer
 * @param pattern Digit pattern (7 bits)
 * @param x X starting position
 * @param module_width Width of one module in pixels
 * @param height Height of the bar in pixels
 * @param bitmap_width Total width of the bitmap
 * @return Number of pixels drawn (width)
 */
static uint16_t draw_pattern(uint8_t *bitmap, uint8_t pattern, uint16_t x, uint16_t module_width,
			     uint16_t height, uint16_t bitmap_width)
{
	uint16_t pos = x;
	uint8_t mask = 0x40; // Start with MSB (bit 6)

	// Draw each bit in the pattern
	for (uint8_t i = 0; i < 7; i++) {
		if (pattern & mask) {
			draw_bar(bitmap, pos, module_width, height, bitmap_width);
		}
		pos += module_width;
		mask >>= 1;
	}

	return pos - x; // Return width drawn
}

/**
 * @brief Draw a guard pattern
 *
 * @param bitmap Pointer to the bitmap buffer
 * @param x X starting position
 * @param pattern Guard pattern (3 or 5 bits)
 * @param pattern_width Width of pattern in bits (3 or 5)
 * @param module_width Width of one module in pixels
 * @param height Height of the bars
 * @param bitmap_width Total width of the bitmap
 * @return Number of pixels drawn (width)
 */
static uint16_t draw_guard(uint8_t *bitmap, uint16_t x, uint8_t pattern, uint8_t pattern_width,
			   uint16_t module_width, uint16_t height, uint16_t bitmap_width)
{
	uint16_t pos = x;
	uint8_t mask = 1 << (pattern_width - 1); // Start with MSB

	// Draw each bit in the pattern
	for (uint8_t i = 0; i < pattern_width; i++) {
		if (pattern & mask) {
			draw_bar(bitmap, pos, module_width, height, bitmap_width);
		}
		pos += module_width;
		mask >>= 1;
	}

	return pos - x; // Return width drawn
}

/**
 * @brief Apply scan mode 6 transformation (transpose and flip)
 *
 * @param bitmap Pointer to bitmap buffer
 * @param temp_buffer Temporary buffer for transformation
 * @param width Width of original bitmap
 * @param height Height of original bitmap
 * @return barcode_error_t Error code
 */
static barcode_error_t apply_scan_mode_6(uint8_t *bitmap, uint8_t *temp_buffer, uint16_t width,
					 uint16_t height)
{
	// Clear the temporary buffer
	memset(temp_buffer, 0, (height * width + 7) / 8);

	// For scan mode 6: transpose (swap x and y) and flip horizontally
	for (uint16_t y = 0; y < height; y++) {
		for (uint16_t x = 0; x < width; x++) {
			// Calculate position in source bitmap
			size_t src_pos = y * width + x;
			uint32_t src_byte = src_pos / 8;
			uint8_t src_bit = 7 - (src_pos % 8);

			// Get the pixel value
			uint8_t pixel = (bitmap[src_byte] >> src_bit) & 0x01;

			if (pixel) {
				// Calculate new position (transposed and flipped)
				uint16_t new_x =
					height - 1 - y; // Flip horizontally after transpose
				uint16_t new_y = x;

				// Set pixel in temp buffer
				size_t dst_pos = new_y * height + new_x;
				uint32_t dst_byte = dst_pos / 8;
				uint8_t dst_bit = 7 - (dst_pos % 8);

				temp_buffer[dst_byte] |= (1 << dst_bit);
			}
		}
	}

	// Copy back to original buffer - note that dimensions are now swapped
	memset(bitmap, 0, (width * height + 7) / 8);
	memcpy(bitmap, temp_buffer, (height * width + 7) / 8);

	return BARCODE_SUCCESS;
}

/******************************************************************************
 * Public Functions Implementation
 ******************************************************************************/

const char *barcode_get_error_message(barcode_error_t error)
{
	// Convert negative error codes to positive array indices
	int index = -error;

	if (error == BARCODE_SUCCESS) {
		return ERROR_MESSAGES[0];
	} else if (index >= 1 && index <= 7) {
		return ERROR_MESSAGES[index];
	} else {
		// Unknown error code
		return ERROR_MESSAGES[8]; // "Unknown error"
	}
}

barcode_error_t barcode_calculate_checksum(const char *digits, uint8_t len, uint8_t *checksum)
{
	// Validate input parameters
	if (digits == NULL || checksum == NULL) {
		return BARCODE_ERROR_NULL_INPUT;
	}

	// UPC-A and EAN-13 use the same checksum algorithm
	// but start at different positions
	uint8_t sum = 0;
	bool is_odd = true;

	for (int i = len - 1; i >= 0; i--) {
		// Validate that this is a digit
		if (digits[i] < '0' || digits[i] > '9') {
			return BARCODE_ERROR_NON_DIGIT;
		}

		uint8_t weight = is_odd ? 3 : 1;
		sum += weight * CHAR_TO_DIGIT(digits[i]);
		is_odd = !is_odd;
	}

	*checksum = (10 - (sum % 10)) % 10;
	return BARCODE_SUCCESS;
}

barcode_error_t barcode_verify(const char *digits, uint8_t len)
{
	// Check input validity first
	barcode_error_t error = validate_digits(digits, len);
	if (error != BARCODE_SUCCESS) {
		return error;
	}

	// Ensure we have at least 2 digits for the check
	if (len < 2) {
		return BARCODE_ERROR_INVALID_LENGTH;
	}

	// Calculate expected checksum
	uint8_t calculated;

	// Get all digits except the last one (which is the checksum)
	error = barcode_calculate_checksum(digits, len - 1, &calculated);
	if (error != BARCODE_SUCCESS) {
		return error;
	}

	// Compare with provided checksum (last digit)
	if (CHAR_TO_DIGIT(digits[len - 1]) != calculated) {
		return BARCODE_ERROR_INVALID_CHECKSUM;
	}

	return BARCODE_SUCCESS;
}

// Use a static buffer instead of a variable length array
static uint8_t temp_buffer[2048]; // Max buffer size for typical display dimensions
barcode_error_t barcode_render(const char *digits, barcode_type_t type, uint8_t *bitmap,
			       uint8_t scan_mode, uint16_t width, uint16_t height,
			       bool include_text)
{
	uint8_t expected_len;
	uint16_t x = 0;
	uint16_t total_modules;
	uint16_t module_width;

	// Validate input parameters
	if (digits == NULL || bitmap == NULL) {
		return BARCODE_ERROR_NULL_INPUT;
	}

	// Make sure the temp buffer is large enough
	if ((width * height + 7) / 8 > sizeof(temp_buffer)) {
		return BARCODE_ERROR_BITMAP_TOO_SMALL;
	}

	// Clear the bitmap buffer
	memset(bitmap, 0, (width * height + 7) / 8);

	// Determine expected length based on barcode type
	switch (type) {
	case BARCODE_EAN13:
		expected_len = 13;
		break;
	case BARCODE_UPCA:
		expected_len = 12;
		break;
	default:
		return BARCODE_ERROR_INVALID_TYPE;
	}

	// Validate input digits and verify checksum in one step
	barcode_error_t error = barcode_verify(digits, expected_len);
	if (error != BARCODE_SUCCESS) {
		return error;
	}

	// Calculate total modules and module width
	if (type == BARCODE_EAN13) {
		// EAN-13 = 12 digits (7 modules each) + 3 guard patterns (3+5+3 modules) + quiet
		// zones
		total_modules = 12 * DIGIT_WIDTH + GUARD_WIDTH_NORMAL * 2 + GUARD_WIDTH_CENTER;
	} else { // BARCODE_UPCA
		// UPC-A = 12 digits (7 modules each) + 3 guard patterns (3+5+3 modules) + quiet
		// zones
		total_modules = 12 * DIGIT_WIDTH + GUARD_WIDTH_NORMAL * 2 + GUARD_WIDTH_CENTER;
	}

	// Add quiet zones to total modules (standardized)
	total_modules += QUIET_ZONE_MODULES * 2;

	// Calculate module width (pixels per module)
	module_width = width / total_modules;
	if (module_width == 0) {
		module_width = 1;
	}

	// Ensure at least 1 pixel per module
	if (width < total_modules || height < 20) // Arbitrary minimum height
	{
		return BARCODE_ERROR_BITMAP_TOO_SMALL;
	}

	// Reserve space for text if needed (usually 20% of height)
	uint16_t bar_height = include_text ? (height * 4) / 5 : height;

	// Start drawing (skip left quiet zone)
	x += QUIET_ZONE_MODULES * module_width;

	// Draw left guard pattern
	x += draw_guard(bitmap, x, GUARD_PATTERN_NORMAL, GUARD_WIDTH_NORMAL, module_width,
			bar_height, width);

	// Draw left side digits
	if (type == BARCODE_EAN13) {
		// First digit of EAN-13 is encoded in the parity of the next 6 digits
		uint8_t first_digit = CHAR_TO_DIGIT(digits[0]);

		// Draw the 6 digits after the first, using parity pattern from first digit
		for (uint8_t i = 1; i <= 6; i++) {
			uint8_t digit = CHAR_TO_DIGIT(digits[i]);
			uint8_t pattern;

			// Select pattern based on parity for this position
			if (EAN13_FIRST_DIGIT_PARITY[first_digit][i - 1] == 0) {
				pattern = PATTERN_L[digit];
			} else {
				pattern = PATTERN_G[digit];
			}

			x += draw_pattern(bitmap, pattern, x, module_width, bar_height, width);
		}
	} else { // BARCODE_UPCA
		// Draw first 6 digits using L patterns
		for (uint8_t i = 0; i < 6; i++) {
			uint8_t digit = CHAR_TO_DIGIT(digits[i]);
			x += draw_pattern(bitmap, PATTERN_L[digit], x, module_width, bar_height,
					  width);
		}
	}

	// Draw center guard pattern
	x += draw_guard(bitmap, x, GUARD_PATTERN_CENTER, GUARD_WIDTH_CENTER, module_width,
			bar_height, width);

	// Draw right side digits (always use R patterns)
	uint8_t start_idx = (type == BARCODE_EAN13) ? 7 : 6;
	for (uint8_t i = 0; i < 6; i++) {
		uint8_t digit = CHAR_TO_DIGIT(digits[start_idx + i]);
		x += draw_pattern(bitmap, PATTERN_R[digit], x, module_width, bar_height, width);
	}

	// Draw right guard pattern
	x += draw_guard(bitmap, x, GUARD_PATTERN_NORMAL, GUARD_WIDTH_NORMAL, module_width,
			bar_height, width);

	// Apply scan mode transformations
	if (scan_mode == 6) {
		// Apply scan mode 6 transformation (transpose and flip horizontally)
		error = apply_scan_mode_6(bitmap, temp_buffer, width, height);
		if (error != BARCODE_SUCCESS) {
			return error;
		}
	}

	return BARCODE_SUCCESS;
}