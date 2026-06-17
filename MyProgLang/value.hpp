#pragma once

#include "expr.hpp"
#include "types.hpp"

#include <cstdint>
#include <variant>

namespace mpl::eval
{
	using int_type = std::int64_t;
	using float_type = double;
	using bool_type = bool;

	struct invalid_type {};

	class value final
	{
	public:
		using underlying_type = std::variant<invalid_type, int_type, float_type, bool_type>;

	public:
		value() noexcept;
		value(int_type val) noexcept;
		value(float_type val) noexcept;
		value(bool_type val) noexcept;

	public:
		bool valid() const noexcept;
		bool truthy() const noexcept;
		const underlying_type& raw() const noexcept;

		value cast_to(types::type type) const noexcept;
		value unary(ast::operation op) const noexcept;
		value binary(ast::operation op, const value& rhs) const noexcept;

	private:
		underlying_type m_underlying;
	};
}
