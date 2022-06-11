#pragma once
#include <memory>

#include "StylesheetDeclaration.h"

namespace yui {

	class StyleHelper
	{
	private:
		StyleHelper() = default;

	public:
		static std::unique_ptr<StylesheetDeclaration> merge(std::vector<StylesheetDeclaration*> declarations);
	};
	
}
