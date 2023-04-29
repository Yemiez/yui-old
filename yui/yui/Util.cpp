#include "Util.h"
#include <cstdio>

yui::Utf8String yui::read_utf8_file(const std::string &filename) {
    auto *handle = fopen(filename.c_str(), "r");

    if (!handle) {
        return { };
    }

    auto start = ftell(handle);
    fseek(handle, 0, SEEK_END);
    auto end = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    auto *buffer = new char[end - start];
    memset(buffer, 0, end - start);
    fread(buffer, 1, end - start, handle);
    Utf8String str{ buffer };
    delete[] buffer;
    return str;
}
