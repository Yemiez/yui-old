#pragma once
#include "EditorEngine.h"
#include "LayoutNode.h"

namespace yui::layout {
	
	class Textarea : public LayoutNode, public EditorEngine, public TextDocument::Client
	{
	public:
		Textarea();
		virtual ~Textarea() override = default;
		void paint(yui::Painter&) override;
		void update(float dt) override;
		void compute() override;
		bool is_inline() const override { return false; }

		bool on_input(int code_point) override;
		bool on_key_down(int key, int scan, int mods) override;
		bool on_key_up(int key, int scan, int mods) override;

		// size of a single mono-spaced character.
		glm::ivec2 character_size() const { return m_character_size; }

		const char* layout_name() const override { return "textarea"; }

		// Client events
		glm::ivec2 document_request_client_text_size(const Utf8String&) override;

		// Editor events
		void editor_caret_did_change() override;
		
		// general functions
		glm::ivec2 position_to_screen(const TextPosition& caret) const;
		TextPosition screen_to_position(glm::ivec2) const;
		void on_click(glm::ivec2 mouse_position) override;

		
	private:
		uint32_t m_columns{80};
		uint32_t m_rows{6};
		float m_caret_beam{0.f};
		glm::ivec2 m_character_size{};

		int m_scroll_x{0};
		int m_scroll_y{0};
	};
	
}
