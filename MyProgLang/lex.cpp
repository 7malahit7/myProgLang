#include "lex.hpp"
#include <array>

namespace mpl
{
	template <typename It>
	constexpr auto is_in_range(Lexer::char_t c, It beg, It end)
	{
		return std::find(beg, end, c) != end;
	}

	constexpr auto is_blank(Lexer::char_t c)
	{
		constexpr std::array blanks{'\n','\t', ' ', '\f', '\v', '\r', '\0'};
		return is_in_range(c, blanks.begin(), blanks.end());
	}

	constexpr auto is_operator(Lexer::char_t c)
	{
		constexpr std::array blanks{ '+','-', '*', '/'};
		return is_in_range(c, blanks.begin(), blanks.end());
	}
	constexpr auto is_digit(Lexer::char_t c)
	{
		return c >= '0' && c <= '9';
	}
	constexpr auto is_paren_open(Lexer::char_t c)
	{
		return c == '(';
	}
	constexpr auto is_paren_close(Lexer::char_t c)
	{
		return c == ')';
	}
	constexpr auto is_paren(Lexer::char_t c)
	{
		return is_paren_open(c) || is_paren_close(c);
	}
	constexpr auto is_separator(Lexer::char_t c)
	{
		return is_paren(c) || is_blank(c) || is_operator(c);
	}
}

namespace mpl
{
	Lexer::Lexer()
		: m_start(m_buffer.end()),
		  m_end(m_buffer.end())
	{

	}

	void Lexer::operator()(value_type input)
	{
		m_buffer = input;
		m_start = m_buffer.begin();
		m_end = m_start;
		m_preview.reset();
		skip_spaces();
	}

	Token Lexer::next()
	{
		auto token = peek();
		m_preview.reset();
		return token;
	}

	const Token& Lexer::peek()
	{
		if (m_preview)
			return *m_preview;

		if (!good())
			return consume(Token::Eof);

		const auto next = peek_char();

		if (is_digit(next))
			return number();
		if (is_operator(next))
			return op();
		if (is_separator(next))
			return punctuation();

		return consume(Token::Error);
	}

	bool Lexer::good() const
	{
		return m_end != m_buffer.end();
	}

	Lexer::char_t Lexer::peek_char() const
	{
		return good() ? *m_end : char_t{};
	}
	void Lexer::advance()
	{
		if (good())
			++m_end;
	}
	Lexer::value_type Lexer::read_str() const
	{
		return { m_start, m_end };
	}
	void Lexer::skip_spaces()
	{
		while (good() && is_blank(peek_char()))
			advance();
		m_start = m_end;
	}
	const Token& Lexer::consume(tok_kind kind)
	{
		if (kind == Token::Error)
		{
			while (!is_separator(peek_char()))
				advance();
		}

		auto val = kind != Token::Eof ? read_str() : value_type{};
		skip_spaces();
		m_preview.emplace(val, kind);
		return *m_preview;
	}
	const Token& Lexer::punctuation()
	{
		const auto next = peek_char();
		advance();
		auto res = Token::Error;
		switch (next)
		{
			case('('): res = Token::ParenOpen;	break;
			case(')'): res = Token::ParenClose; break;
		}
		return consume(res);
	}
	const Token& Lexer::op()
	{
		const auto next = peek_char();
		advance();
		auto res = Token::Error;
		switch (next)
		{
		case('+'): res = Token::Plus;		break;
		case('-'): res = Token::Minus;		break;
		case('*'): res = Token::Asterisk;	break;
		case('/'): res = Token::Slash;		break;
		}
		return consume(res);
	}
	const Token& Lexer::number()
	{
		auto res = Token::Number;
		while (good())
		{
			const auto next = peek_char();
			if (is_separator(next))
				break;
			advance();
			if (is_digit(next))
				continue;
			res = Token::Error;
		}
		return consume(res);
	}
}