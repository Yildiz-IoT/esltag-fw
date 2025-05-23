#
# Copyright (c) 2022 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

# The application uses the conf/<board> scheme for configuration files.
set(APPLICATION_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/conf/\${BOARD}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_esl)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
  hw/esl_hw_impl.c
)

#Add ESLS source
zephyr_library_sources_ifdef(
  CONFIG_BT_ESL
  ../../service/esl.c
  ../../service/esl_settings.c
  ../../service/esl_common.c)

zephyr_library_sources_ifdef(
  CONFIG_ESL_SHELL
  ../../service/esl_shell.c)

zephyr_library_include_directories_ifdef(CONFIG_BT_ESL ../../service)

# Display HW implementation

if (CONFIG_SSD16XX)
  target_sources(app PRIVATE hw/display/epd_shield_display.c)
endif()

if (CONFIG_UC81XX)
  target_sources(app PRIVATE hw/display/epd_shield_uc81xx_display.c)
endif()

if (CONFIG_ESL_SIMULATE_DISPLAY)
  target_sources(app PRIVATE hw/display/led_display.c)
endif()

if (CONFIG_BT_ESL_DISPLAY_MAX EQUAL 0)
  target_sources(app PRIVATE hw/display/dummy_display.c)
endif()

if ((CONFIG_EPD_4IN2_V1) OR (CONFIG_EPD_4IN2_V2) OR (CONFIG_EPD_1IN54B_V2) OR (CONFIG_EPD_2IN9B_V3) OR (CONFIG_EPD_2IN13B_V3))
  add_subdirectory(driver/EPD)
  target_sources(app PRIVATE hw/display/epd_display.c)
endif()

if (CONFIG_BT_ESL_JF_PAINT_LIB)
  FILE(GLOB paint_lib_sources hw/display/paint_lib/*.c)
  target_sources(app PRIVATE hw/display/paint_lib/font8.c)
  target_sources(app PRIVATE hw/display/paint_lib/font12.c)
  target_sources(app PRIVATE hw/display/paint_lib/font16.c)
  target_sources(app PRIVATE hw/display/paint_lib/font20.c)
  target_sources(app PRIVATE hw/display/paint_lib/font24.c)
  target_sources(app PRIVATE hw/display/paint_lib/paint.c)
  zephyr_library_include_directories(hw/display/paint_lib/include)
endif()

# Sensor HW implementation
target_sources(app PRIVATE hw/sensor/sensor.c)

# LED HW implementation
if (CONFIG_BT_ESL_LED_MAX EQUAL 0)
  target_sources(app PRIVATE hw/led/dummy_led.c)
else()
  target_sources(app PRIVATE hw/led/led.c)
endif()

# Storage HW implementation
if (CONFIG_BT_ESL_IMAGE_AVAILABLE)
  if (CONFIG_ESL_OTS_NVS)
    target_sources(app PRIVATE hw/storage/nvs.c)
  elseif (CONFIG_ESL_OTS_LFS)
    target_sources(app PRIVATE hw/storage/lfs.c)
  endif()
else()
  target_sources(app PRIVATE hw/storage/dummy.c)
endif()

# Vendor Specific ECP command
if (CONFIG_BT_ESL_VENDOR_SPECIFIC_SUPPORT)
  target_sources(app PRIVATE hw/esl_vs_impl.c)
endif()

# NFC
if (CONFIG_ESL_NFC_SUPPORT)
  target_sources(app PRIVATE hw/esl_nfc_impl.c)
endif()

# Add barcode encoder library to the source files
target_sources(app PRIVATE
  hw/display/barcode_encoder.c
)

# NORDIC SDK APP END

target_include_directories(app PRIVATE hw)
