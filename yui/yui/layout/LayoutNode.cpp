#include "LayoutNode.h"


#include "Box.h"
#include "DocumentWidget.h"
#include "Inline.h"
#include "../ymd/Node.h"
#include "../yss/StylesheetDeclaration.h"

static uint32_t g_global_id_counter = 0;

yui::layout::LayoutNode::LayoutNode()
	: m_id(++g_global_id_counter)
{
}

void yui::layout::LayoutNode::update(float dt)
{
	for (auto *child : m_children) {
		child->update(dt);
	}
}

void yui::layout::LayoutNode::compute()
{
	for (auto *child : m_children) {
		child->set_position({
			child->dom_node()->computed().margin().left,
			child->dom_node()->computed().margin().top
		});
		child->compute();
	}
}

void yui::layout::LayoutNode::set_size(glm::ivec2 size)
{
	m_size = size;
}

void yui::layout::LayoutNode::set_position(glm::ivec2 pos)
{
	m_position = pos;
}

glm::ivec2 yui::layout::LayoutNode::inner_position() const
{
	const auto padding = dom_node()->computed().padding();
	return m_position + glm::ivec2{ padding.x, padding.y };
}

glm::ivec2 yui::layout::LayoutNode::inner_size() const
{
	const auto padding = dom_node()->computed().padding();
	return m_size - glm::ivec2{ padding.x * 2, padding.y * 2 };
}

glm::ivec2 yui::layout::LayoutNode::size_with_padding() const
{
	const auto padding = dom_node()->computed().padding();
	return m_size + glm::ivec2{ padding.x * 2, padding.y * 2 };
}

void yui::layout::LayoutNode::set_dom_node(Node* dom_node)
{
	m_dom_node = dom_node;
}

void yui::layout::LayoutNode::set_parent(LayoutNode* node)
{
	m_parent = node;
}

void yui::layout::LayoutNode::set_previous(LayoutNode* node)
{
	m_previous = node;
}

void yui::layout::LayoutNode::set_next(LayoutNode* node)
{
	m_next= node;
}

void yui::layout::LayoutNode::insert_child(LayoutNode* node)
{
	auto exists = std::find_if(m_children.begin(), m_children.end(), [&node](auto child) { return node == child; });

	if (exists == m_children.end()) {
		m_children.emplace_back(node);
		update_siblings();
	}
	else {
		__debugbreak();
		printf("Tried to insert child twice!\n");
	}
}

yui::layout::LayoutNode::ChildIterator yui::layout::LayoutNode::remove_child(LayoutNode* layout_node)
{
	const auto it = std::find_if(m_children.begin(), m_children.end(), [&layout_node](auto *child) {
		return layout_node = child;
	});

	if (it != m_children.end()) {
		(*it)->set_parent(nullptr);
		(*it)->set_previous(nullptr);
		(*it)->set_next(nullptr);

		auto ret_it = m_children.erase(it);
		update_siblings();
		return ret_it;
	}
	return m_children.end();
}

void yui::layout::LayoutNode::replace_child(LayoutNode* old_child, LayoutNode* new_child)
{
	auto it = remove_child(old_child);

	if (it != m_children.end()) {
		m_children.insert(it, new_child);
	}
	else {
		m_children.emplace_back(new_child);
	}
}

bool yui::layout::LayoutNode::is_ancestor_of(LayoutNode* node) const
{
	for (const auto &child : m_children) {
		if (child == node) return true;
		if (child->is_ancestor_of(node)) return true;
	}

	return false;
}

bool yui::layout::LayoutNode::is_child_of(LayoutNode* parent) const
{
	for (const auto* node = m_parent; node != nullptr; node = node->m_parent) {
		if (node == parent) return true;
	}

	return false;
}

void yui::layout::LayoutNode::set_document_widget(DocumentWidget* widget)
{
	m_document_widget = widget;
}

bool yui::layout::LayoutNode::hit_test(int x, int y) const
{
	return x >= m_position.x && x <= m_position.x + size_with_padding().x &&
		y >= m_position.y && y <= m_position.y + size_with_padding().y;
}

bool yui::layout::LayoutNode::on_input(int code_point)
{
	for (auto *child : m_children) {
		if (child->on_input(code_point)) {
			return true;
		}
	}

	return false;
}

bool yui::layout::LayoutNode::on_key_down(int key, int scan, int mods)
{
	for (auto *child : m_children) {
		if (child->on_key_down(key, scan, mods)) {
			return true;
		}
	}

	return false;
}

bool yui::layout::LayoutNode::on_key_up(int key, int scan, int mods)
{
	for (auto *child : m_children) {
		if (child->on_key_up(key, scan, mods)) {
			return true;
		}
	}

	return false;
}

bool yui::layout::LayoutNode::on_mouse_move(glm::ivec2 pt)
{
	for (auto *child : m_children) {
		if (child->on_mouse_move(pt)) {
			return true;
		}
	}

	// This is the currently hovered element
	if (hit_test(pt.x, pt.y)) {
		dom_node()->set_hovered(true);
		document_widget()->invalidate_node(*this);
		on_mouse_enter();
		return true;
	}

	if (dom_node()->set_hovered(false)) {
		document_widget()->invalidate_node(*this);
		on_mouse_leave();
	}
	return false;
}

bool yui::layout::LayoutNode::on_left_mouse_down(glm::ivec2 pt)
{
	if (!hit_test(pt.x, pt.y)) {
		return false;
	}

	for (auto *child : m_children) {
		if (child->on_left_mouse_down(pt)) {
			return true;
		}
	}

	return true;
}

bool yui::layout::LayoutNode::on_left_mouse_up(glm::ivec2 pt)
{
	if (!hit_test(pt.x, pt.y)) {
		return false;
	}

	for (auto *child : m_children) {
		if (child->on_left_mouse_up(pt)) {
			return true;
		}
	}

	dom_node()->set_focused(true);
	document_widget()->invalidate_node(*this);
	on_click(pt);
	return true;
}

void yui::layout::LayoutNode::update_siblings()
{
	for (auto i = 0u; i < m_children.size(); ++i) {
		auto *prev = i == 0 ? nullptr : children().at(i - 1);
		auto *child = children().at(i);
		auto* next = i + 1 == m_children.size() ? nullptr : children().at(i + 1);
		child->set_next(next);
		child->set_previous(prev);
		child->set_parent(this);
	}
}
