// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <stdarg.h>
#include <string.h>
#include <string>

#include "p/base/log.h"
#include "p/base/utils.h"
//#include "p/base/fixed_buffer.h"

namespace p {
namespace base {

class LogStream {
public:
    struct LogEntry;

    LogStream()
        : sentry_(nullptr), log_(nullptr), cur_(nullptr), end_(nullptr),
          source_file_(__FILE__, __LINE__) {}

    LogStream& operator<<(short v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(unsigned short v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(int v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(unsigned int v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(long v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& HexLogger(uint64_t v) {
        if (avial() >= kMaxNumericSize) {
            cur_ += ConvertHexInteger(cur_, v);
        }
        return *this;
    }

    LogStream& HexLogger(uint32_t v) {
        if (avial() >= kMaxNumericSize) {
            cur_ += ConvertHexInteger(cur_, v);
        }
        return *this;
    }

    LogStream& operator<<(unsigned long v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(long long v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(unsigned long long v) {
        _AppendInteger(v);
        return *this;
    }

    LogStream& operator<<(float v) {
        *this << static_cast<double>(v);
        return *this;
    }

    LogStream& operator<<(long double v) {
        appendf("%LF", v);
        return *this;
    }

    LogStream& operator<<(double v) {
        appendf("%F", v);
        return *this;
    }

    LogStream& operator<<(const char* str) {
        if (str) {
            append(str);
        } else {
            append("null");
        }
        return *this;
    }

    LogStream& operator<<(const std::string& str) {
        append(str.data(), str.size());
        return *this;
    }

    LogStream& operator<<(const void* v) {
        constexpr int kMaxPointerSize = 2 * sizeof(const void*) + 2;
        if (avial() >= kMaxPointerSize) {
            cur_ += ConvertPointer(cur_, v);
        }
        return *this;
    }

    LogStream& operator<<(bool v) {
        append(v ? '1' : '0');
        return *this;
    }

    LogStream& operator<<(char v) {
        append(v);
        return *this;
    }

    template<typename T>
    LogStream& operator <<(const T* v) {
        constexpr int kMaxPointerSize = 2 * sizeof(const void*) + 2;
        if (avial() >= kMaxPointerSize) {
            cur_ += ConvertPointer(cur_, v);
        }
        return *this;
    }

    LogStream& operator<<(LogStream& (*func)(LogStream&)) { return (*func)(*this); }

    LogStream& operator()(const char* data, int len) {
        append(data,len);
        return *this;
    }

    LogStream& noflush() {
        auto_flush_ = false;
        return *this;
    }

    int append(const char* buf, int len) {
        if (UNLIKELY(len > avial())) {
            len = avial();
        }

        if (UNLIKELY(len <= 0)) {
            return 0;
        }
        return just_append(buf, len);
    }

private:
    // check buffer is enaugh for a log and check buffer is using by a log
    // return -1, buffer is using, write log is not finished;
    // return >=0, buffer is ready and empty
    int check_log_buffer();

    void Sink();

    int just_append(const char* buf, int len) {
        ::memcpy(cur_, buf, len);
        cur_ += len;
        return len;
    }

    int append(char ch) {
        if (avial() > 0) {
            *cur_++ = ch;
            return 1;
        }
        return 0;
    }

    int append(const char* str) { return append(str, ::strlen(str)); }

    int appendf(const char* fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        const int ret = appendf(fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int appendf(const char* fmt, va_list argptr) {
        int n = avial();
        if (UNLIKELY(n <= 0)) {
            return 0;
        }
        int ret = ::vsnprintf(cur_, n + 1, fmt, argptr);
        if (UNLIKELY(ret < 0)) {
            return 0;
        }
        if (ret > n) {
            ret = n;
        }
        cur_ += ret;
        return ret;
    }

    const int avial() const { return static_cast<int>(end_ - cur_); }

    template <typename T> void _AppendInteger(T v) {
        if (avial() >= kMaxNumericSize) {
            cur_ += ConvertInteger(cur_, v);
        }
    }

    friend class LogMessage;

private:
    LogEntry*  sentry_;
    LogEntry*  log_;
    char*      cur_;
    char*      end_;
    bool       auto_flush_ = true;
    LogLevel   log_level_;
    SourceFile source_file_;
    P_DISALLOW_COPY(LogStream);
};

typedef LogStream LogStream;

class LogMessage {
public:
    static bool set_wf_log_min_level(LogLevel wf_log_min_level);

    typedef void (*OutputFunc)(const char* log_msg, int len);

    static void set_output_func(OutputFunc output_func);

    static void set_wf_output_func(OutputFunc output_func);

public:
    LogMessage() {}

    // LogStream &log_stream(LogLevel log_level, const char *file, int line);

    LogStream& log_stream(LogLevel log_level, const SourceFile& source_file);

    ~LogMessage();

private:
    P_DISALLOW_COPY(LogMessage);
};

template<typename T>
LogStream& operator <<(LogStream& ls, const T& t) {
    return t.Logger(ls);
}

template<typename T>
LogStream& operator <<(LogStream& ls, const T* t) {
    return ls.operator<<((const void*)t);
}

template<typename T>
LogStream& operator <<(LogStream& ls, T* t) {
    return ls.operator<<((void*)t);
}

struct noflush {
    LogStream& Logger(LogStream& ls) const {
        return ls.noflush();
    }

private:
    P_DISALLOW_COPY(noflush);
};

struct HexUint64 {
    HexUint64(uint64_t v) : value(v) {}
    uint64_t value;
    LogStream& Logger(LogStream& ls) const {
        return ls.HexLogger(value);
    }
};

struct HexUint32 {
    HexUint32(uint32_t v) : value(v) {}
    HexUint32(int32_t v) : value(v) {}
    uint32_t value;
    LogStream& Logger(LogStream& ls) const {
        return ls.HexLogger(value);
    }
};

} // end namespace base
} // end namespace p

