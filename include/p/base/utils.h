// Copyright (c) 2015, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <algorithm>
#include <stdint.h>

namespace p {
namespace base {

constexpr int kMaxNumericSize = 24;

// Efficient Integer to String Conversions, by Matthew Wilson.
// Copy code from muduo, https://github.com/chenshuo/muduo
template <typename T> size_t ConvertInteger(char *buf, T value) {
    static const char s_digits[] = "9876543210123456789";
    static const char *s_zero = s_digits + 9;
    T i = value;
    char *p = buf;
    int32_t lsd;

    do {
        lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = s_zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template<typename T>
size_t ConvertHexInteger(char *buf, T value) {
    static const char s_digits[] = "0123456789ABCDEFGHIGK";

    char *p = buf;
    uint64_t v = value;
    uint32_t lsd;
    do {
        lsd = v % 16;
        v /= 16;
        *p++ = s_digits[lsd];
    } while (v);

    *p++ = 'x';
    *p++ = '0';
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

size_t ConvertPointer(char *buf, const void *value);

} // end namespace base
} // end namespace p
