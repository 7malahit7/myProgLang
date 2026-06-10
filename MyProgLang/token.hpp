#pragma once
#include <cstdint>
#include <string_view>

namespace mpl
{
	enum class tok_kind : std::uint8_t
	{
		Error,
		Eof,

		Plus,
		Minus,
		Asterisk,
		Slash,
		ParenOpen,
		ParenClose,

		Number
	};

	class Token final
	{
	public:
		using value_type = std::string_view;
		using enum tok_kind;
	public:
		Token() = delete;
		~Token() = default;

		Token(const Token&) = default;
		Token& operator= (const Token&) = default;

		Token(Token&&) = default;
		Token& operator=(Token&&) = default;

		Token(value_type val, tok_kind tag) noexcept :
			m_value(val), m_tag(tag)
		{

		}
	public:
		value_type value() const
		{
			return m_value;
		}
		tok_kind what() const
		{
			return m_tag;
		}
	private:
		value_type m_value{};
		tok_kind   m_tag{};
	};
}