#pragma once

#include "decl.hpp"
#include "value.hpp"

#include <unordered_map>

namespace mpl::eval
{
	class value_map final
	{
	public:
		value_map() = default;
		~value_map() = default;

		value_map(const value_map&) = delete;
		value_map& operator=(const value_map&) = delete;
		value_map(value_map&&) = default;
		value_map& operator=(value_map&&) = default;

	public:
		bool contains(const ast::decl& decl) const;
		value get(const ast::decl& decl) const;
		void set(const ast::decl& decl, value val);

	private:
		std::unordered_map<const ast::decl*, value> m_values;
	};
}
