#pragma once
#include <map>
#include <string>
#include "Selector.h"
#include "../Painter.h"

namespace yui {
	class Selector;

	#define STYLESHEET_PROPERTIES_ENUMERATOR \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(Display, display, display, StylesheetValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(Cursor, cursor, cursor, StylesheetValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(BackgroundColor, backgroundColor, background-color, StylesheetColorValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(TextColor, textColor, text-color, StylesheetColorValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(FontName, fontName, font-name, StylesheetValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(FontSize, fontSize, font-size, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(BorderColor, borderColor, border-color, StylesheetColorValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(BorderSize, borderSize, border-size, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(TextAlign, textAlign, text-align, StylesheetValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(Layout, layout, layout, StylesheetValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MarginTop, marginTop, margin-top, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MarginBottom, marginBottom, margin-bottom, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MarginLeft, marginLeft, margin-left, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MarginRight, marginRight, margin-right, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(PaddingX, paddingX, padding-x, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(PaddingY, paddingY, padding-y, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(Width, width, width, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(Height, height, height, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MaxWidth, maxWidth, max-width, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MaxHeight, maxHeight, max-height, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MinWidth, minWidth, min-width, StylesheetUnitSizeValue) \
		_STYLESHEET_PROPERTIES_ENUMERATOR_(MinHeight, minHeight, min-height, StylesheetUnitSizeValue)
	
	
	// Standardized properties
	enum class StylesheetPropertyId
	{
		Unknown,
#define _STYLESHEET_PROPERTIES_ENUMERATOR_(en, ...) en,
		STYLESHEET_PROPERTIES_ENUMERATOR
#undef _STYLESHEET_PROPERTIES_ENUMERATOR_
	};

	class StylesheetValue
	{
	public:
		StylesheetValue() = default;
		StylesheetValue(std::string);
		virtual ~StylesheetValue() = default;

		const std::string& string() const;
		virtual void set_string(std::string);

		virtual StylesheetValue* copy() const ;
	protected:
		std::string m_string{};
	};

	class StylesheetColorValue : public StylesheetValue
	{
	public:
		StylesheetColorValue(std::string);

		Color color() const { return m_color; }
		
		StylesheetValue* copy() const override;
		static Color to_color(const std::string&);
		static std::string to_string(Color);
	private:
		Color m_color{};
	};

	class StylesheetUnitSizeValue : public StylesheetValue
	{
	public:
		enum class Unit
		{
			Pixels,
			Percentage,
		};
		
	public:
		StylesheetUnitSizeValue(std::string);
		StylesheetUnitSizeValue(int, Unit);
		void set_string(std::string) override;

		Unit unit() const { return m_unit; }
		int scalar() const { return m_scalar; }
		void set_unit(Unit);
		void set_scalar(int);

		static std::string to_string(int, Unit);
		static std::pair<int, Unit> to_pair(std::string);

		StylesheetValue* copy() const override;
	private:
		int m_scalar{};
		Unit m_unit{Unit::Pixels};
	};
	
	class StylesheetDeclaration
	{
	public:
		using PropertyMap = std::map<StylesheetPropertyId, StylesheetValue*>;
		using CustomPropertyMap = std::map<std::string, StylesheetValue*>;
		
	public:
		StylesheetDeclaration(std::vector<Selector> selectors);
		StylesheetDeclaration() = default;
		StylesheetDeclaration(const StylesheetDeclaration&);
		StylesheetDeclaration(StylesheetDeclaration&&) noexcept;
		~StylesheetDeclaration();

		const PropertyMap& properties() const { return m_properties; }
		const CustomPropertyMap& custom_properties() const { return m_custom_properties; }

		const std::vector<Selector>& selectors() const { return m_selectors; }
		void set_selectors(std::vector<Selector> selector) { m_selectors = std::move(selector); }

		bool match(Node& dom_node) const;
		bool match(Node& dom_node, const Selector** first_matching_selector) const;

		uint32_t weight() const;
		
		bool has_property(StylesheetPropertyId) const;
		bool has_property(const std::string&) const;
		const StylesheetValue& property(StylesheetPropertyId) const;
		const StylesheetValue& property(const std::string&) const;

		void set_property(StylesheetPropertyId, std::string);
		void set_property(const std::string&, std::string);
		void set_property_ptr(StylesheetPropertyId, StylesheetValue*);
		void set_property_ptr(const std::string&, StylesheetValue*);
		void unset_property(StylesheetPropertyId);
		void unset_property(const std::string&);
		
		static StylesheetPropertyId property_id_from_string(const std::string&);
		static StylesheetValue* create_stylesheet_value(StylesheetPropertyId, std::string);
	private:
		std::vector<Selector> m_selectors;
		PropertyMap m_properties{}; // Properties, standardized.
		CustomPropertyMap m_custom_properties{}; // Custom properties, unknown.
	};
	
}
