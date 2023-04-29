#pragma once
#include "EditorEngine.h"
#include "Inline.h"

namespace yui::layout {

class Input : public LayoutNode, public EditorEngine, public TextDocument::Client {
public:
    Input();
    ~Input() override = default;

    void paint(yui::Painter &) override;
    void update(float dt) override;
    void compute() override;

    bool on_key_down(int key, int scan, int mods) override;
    bool on_key_up(int key, int scan, int mods) override;
    bool on_input(int code_point) override;

    // Client
    void text_document_did_change() override;

    [[nodiscard]] const char *layout_name() const override { return "input"; }

    // Client events events
    glm::ivec2 document_request_client_text_size(const Utf8String &) override;

    // Editor events
    void editor_caret_did_change() override;

    // general functions
    [[nodiscard]] glm::ivec2 position_to_screen(const TextPosition &caret) const;
    [[nodiscard]] TextPosition screen_to_position(glm::ivec2) const;
protected:
    void on_click(glm::ivec2 mouse_position) override;
private:
    TextRange m_view_range{ };
    float m_caret_beam{ 0.f };
    glm::ivec2 m_character_size{ };

    // Scroll in characters.
    u32 m_size_value{ 0 };
    u32 m_scroll{ 0 };
    Utf8String m_placeholder{ };
};

}
