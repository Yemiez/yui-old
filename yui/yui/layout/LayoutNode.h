#pragma once
#include <memory>
#include <vector>
#include "../Painter.h"

namespace yui {
	class Painter;
	class StylesheetDeclaration;
	class Node;
}

namespace yui::layout {
	class DocumentWidget;

	struct Rect
	{
		int top{0}, left{0}, bottom{0}, right{0};
	};

	class LayoutNode
	{
	public:
		using ChildIterator = std::vector<LayoutNode*>::iterator;
		
		LayoutNode();
		virtual ~LayoutNode() = default;

		virtual void paint(yui::Painter&) {}
		virtual void update(float dt);
		// Compute widths & heights
		virtual void compute();

		uint32_t id() const { return m_id; }

		void set_size(glm::ivec2 size);
		void set_position(glm::ivec2 pos);

		// The total absolute rect of this element including padding and border.
		Rect absolute_rect() const
		{
			return {
				.top = m_position.y,
				.left = m_position.x,
				.bottom = m_position.y + size_with_padding().y,
				.right = m_position.x + size_with_padding().x
			};
		}
		
		// The absolute top-left position of this node in window rect.
		glm::ivec2 position() const { return m_position; }

		glm::ivec2 inner_position() const;
		glm::ivec2 inner_size() const;
		
		// The size of this element (excluding padding).
		glm::ivec2 size() const { return m_size; }

		glm::ivec2 size_with_padding() const;
		
		void set_dom_node(Node* dom_node);
		Node* dom_node() const { return m_dom_node; }

		// TODO: Sync next/previous etc at every insert/erase/replace.
		LayoutNode* next() const { return m_next; }
		LayoutNode* previous() const { return m_previous; }
		LayoutNode* parent() const { return m_parent; }
		const std::vector<LayoutNode*>& children() const { return m_children; }

		void set_parent(LayoutNode*);
		void set_previous(LayoutNode*);
		void set_next(LayoutNode*);
		void insert_child(LayoutNode*);
		ChildIterator remove_child(LayoutNode* layout_node);
		void replace_child(LayoutNode *old_child, LayoutNode *new_child);

		// Check if given node is a child of this node.
		bool is_ancestor_of(LayoutNode*) const;

		// Check if given node is a parent of the current node.
		bool is_child_of(LayoutNode*) const;

		DocumentWidget* document_widget() const { return m_document_widget; }
		void set_document_widget(DocumentWidget*);

		virtual bool is_inline() const { return false; }
		virtual bool is_inline_box() const { return false; };
		virtual bool is_input() const { return false; }
		virtual bool is_box() const { return false; }
		virtual bool hit_test(int x, int y) const;
		virtual bool is_root() const { return false; }

		template<typename T>
		T* as() { return dynamic_cast<T*>(this); }

		template<typename T>
		const T* as() const { return dynamic_cast<const T*>(this); }

		virtual bool on_input(int code_point);
		virtual bool on_key_down(int key, int scan, int mods);
		virtual bool on_key_up(int key, int scan, int mods);

		// Return true to say "handled".
		virtual bool on_mouse_move(glm::ivec2);
		virtual bool on_left_mouse_down(glm::ivec2);
		virtual bool on_left_mouse_up(glm::ivec2);

		virtual const char* layout_name() const { return "LayoutNode"; }

	protected:
		virtual void on_mouse_enter() {}
		virtual void on_mouse_leave() {}
		virtual void on_click(glm::ivec2 mouse_position) {}

	protected:
		void update_siblings();
	protected:
		uint32_t m_id{};
		glm::ivec2 m_position{};
		glm::ivec2 m_size{};
		Node* m_dom_node{};
		
		// Tree
		LayoutNode* m_previous{nullptr};
		LayoutNode* m_parent{nullptr};
		LayoutNode* m_next{nullptr};
		std::vector<LayoutNode*> m_children{};

		// Document
		DocumentWidget* m_document_widget{nullptr};
	};

}
