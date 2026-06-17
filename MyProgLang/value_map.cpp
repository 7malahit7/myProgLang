#include "value_map.hpp"

namespace mpl::eval
{
	bool value_map::contains(const ast::decl& decl) const
	{
		return m_values.contains(&decl);
	}

	value value_map::get(const ast::decl& decl) const
	{
		auto found = m_values.find(&decl);
		if (found == m_values.end())
			return value{};

		return found->second;
	}

	void value_map::set(const ast::decl& decl, value val)
	{
		m_values[&decl] = val;
	}
}
