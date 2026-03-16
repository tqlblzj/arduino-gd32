/*
 * SPI.h - Wrapper for src/SPI.h
 *
 * This file is a wrapper that includes the actual SPI header from the src directory.
 * Arduino library convention requires main headers to be in the library root directory.
 *
 * IMPORTANT: This wrapper handles the SPI macro conflict issue.
 * The SPI macro is defined in gd32vw55x_spi.h as SPI_BASE and gets included
 * through Arduino.h's include chain. We undefine it here before including
 * the actual SPI library header to prevent conflicts with the SPI class instance.
 */

#ifndef SPI_H_WRAPPER
#define SPI_H_WRAPPER

/* Undefine SPI macro from system headers to avoid conflict with C++ class instance */
#ifdef SPI
    #undef SPI
#endif

/* Include the actual SPI library header */
#include "src/SPI.h"

#endif /* SPI_H_WRAPPER */
