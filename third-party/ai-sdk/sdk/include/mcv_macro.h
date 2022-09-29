/*!
 *  @file: mcv_macro.h
 *  @brief: mcv_macro
 *  @version: 1.0.0
 *  @author:
 *  @date:
 */

#pragma once

#ifndef IN
#define IN
#endif    // !IN

#ifndef OUT
#define OUT
#endif    // !OUT

#ifdef __cplusplus
#define MCV_API_BEGIN extern "C" {
#define MCV_API_END }
#else
#define MCV_API_BEGIN
#define MCV_API_END
#endif

#if MCV_LIBRARY
#if defined(_WIN32)
#if MCV_DLLEXPORT
#define MCV_API __declspec(dllexport)
#else
#define MCV_API __declspec(dllimport)
#endif
#else
#if MCV_DLLEXPORT
#define MCV_API __attribute__((visibility("default")))
#else
#define MCV_API
#endif
#endif
#else
#define MCV_API
#endif

#ifdef __GNUC__
#define MCV_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define MCV_DEPRECATED __declspec(deprecated)
#else
#define MCV_DEPRECATED
#endif
