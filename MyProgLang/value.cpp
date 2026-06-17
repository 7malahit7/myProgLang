#include "value.hpp"
#include "expr.hpp"

#include <charconv>
#include <cmath>
#include <type_traits>
#include <variant>

namespace mpl::eval
{
	namespace
	{
		template <typename T>
		constexpr bool is_invalid = std::same_as<T, invalid_type>;

		value make_number(auto val, types::type type) noexcept
		{
			using enum types::type;
			switch (type)
			{
			case Bool:  return value{ static_cast<bool_type>(val) };
			case Int:   return value{ static_cast<int_type>(val) };
			case Float: return value{ static_cast<float_type>(val) };
			default:    return value{};
			}
		}

		value make_arithmetic(auto val) noexcept
		{
			using val_t = decltype(val);
			if constexpr (std::same_as<val_t, float_type>)
				return value{ static_cast<float_type>(val) };
			else
				return value{ static_cast<int_type>(val) };
		}
	}

	value::value() noexcept
		: m_underlying{ invalid_type{} }
	{
	}

	value::value(int_type val) noexcept
		: m_underlying{ val }
	{
	}

	value::value(float_type val) noexcept
		: m_underlying{ val }
	{
	}

	value::value(bool_type val) noexcept
		: m_underlying{ val }
	{
	}

	bool value::valid() const noexcept
	{
		return !std::holds_alternative<invalid_type>(m_underlying);
	}

	bool value::truthy() const noexcept
	{
		return std::visit([](auto val) noexcept
			{
				using val_t = decltype(val);
				if constexpr (is_invalid<val_t>)
					return false;
				else
					return static_cast<bool>(val);
			}, m_underlying);
	}

	const value::underlying_type& value::raw() const noexcept
	{
		return m_underlying;
	}

	value value::cast_to(types::type type) const noexcept
	{
		return std::visit([type](auto val) noexcept -> value
			{
				using val_t = decltype(val);
				if constexpr (is_invalid<val_t>)
					return value{};
				else
					return make_number(val, type);
			}, m_underlying);
	}

	value value::unary(ast::operation op) const noexcept
	{
		return std::visit([op](auto val) noexcept -> value
			{
				using val_t = decltype(val);
				if constexpr (is_invalid<val_t>)
				{
					return value{};
				}
				else
				{
					using enum ast::operation;
					switch (op)
					{
					case UnaryPos: return value{ val };
					case UnaryNeg:
						if constexpr (std::same_as<val_t, bool_type>)
							return value{};
						else
							return make_arithmetic(-val);
					case UnaryNot: return value{ !static_cast<bool>(val) };
					default:       return value{};
					}
				}
			}, m_underlying);
	}

	value value::binary(ast::operation op, const value& rhs) const noexcept
	{
		return std::visit([op](auto lhs, auto rhs) noexcept -> value
			{
				using lhs_t = decltype(lhs);
				using rhs_t = decltype(rhs);

				if constexpr (is_invalid<lhs_t> || is_invalid<rhs_t>)
				{
					return value{};
				}
				else
				{
					constexpr auto useFloat =
						std::same_as<lhs_t, float_type> ||
						std::same_as<rhs_t, float_type>;
					const auto l = static_cast<std::conditional_t<useFloat, float_type, int_type>>(lhs);
					const auto r = static_cast<std::conditional_t<useFloat, float_type, int_type>>(rhs);

					using enum ast::operation;
					switch (op)
					{
					case Addition:       return make_arithmetic(l + r);
					case Subtraction:    return make_arithmetic(l - r);
					case Multiplication: return make_arithmetic(l * r);
					case Division:       return make_arithmetic(l / r);
					case Equal:          return value{ l == r };
					case NEqual:         return value{ l != r };
					case Less:           return value{ l < r };
					case Greater:        return value{ l > r };
					case LessEq:         return value{ l <= r };
					case GreaterEq:      return value{ l >= r };
					case LogAnd:         return value{ static_cast<bool>(lhs) && static_cast<bool>(rhs) };
					case LogOr:          return value{ static_cast<bool>(lhs) || static_cast<bool>(rhs) };
					default:             return value{};
					}
				}
			}, m_underlying, rhs.m_underlying);
	}
}
