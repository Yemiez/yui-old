#pragma once
#include "Utf8String.h"

namespace yui {
class Window;

class Clipboard {
public:
    explicit Clipboard(Window &parent);
    Utf8String content() const;
    void set_content(const Utf8String &content);
    void set_content(const char *content);
    void set_content(const std::string &content);
private:
    Window &m_window;
};

}
