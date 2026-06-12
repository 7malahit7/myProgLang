#include "lex.hpp"
#include <array>
#include <unordered_map>




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
		constexpr std::array blanks{ '+','-', '*', '/', '=', '<', '>', '!', '&', '|'};
		return is_in_range(c, blanks.begin(), blanks.end());
	}
	constexpr auto is_comma(Lexer::char_t c)
	{
		return c == ',';
	}
	constexpr auto is_curly_open(Lexer::char_t c)
	{
		return c == '{';
	}
	constexpr auto is_curly_close(Lexer::char_t c)
	{
		return c == '}';
	}
	constexpr auto is_digit(Lexer::char_t c)
	{
		return c >= '0' && c <= '9';
	}
	constexpr auto is_id_start(Lexer::char_t c)
	{
		return	(c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				c == '_';
	}
	constexpr auto is_id_char(Lexer::char_t c)
	{
		return is_id_start(c) || is_digit(c);
	}
	constexpr auto is_paren_open(Lexer::char_t c)
	{
		return c == '(';
	}
	constexpr auto is_dot(Lexer::char_t c)
	{
		return c == '.';
	}
	constexpr auto is_paren_close(Lexer::char_t c)
	{
		return c == ')';
	}
	constexpr auto is_paren(Lexer::char_t c)
	{
		return is_paren_open(c) || is_paren_close(c);
	}
	constexpr auto is_semi(Lexer::char_t c)
	{
		return c == ';';
	}
	constexpr auto is_separator(Lexer::char_t c)
	{
		return is_paren(c) ||
			is_blank(c) ||
			is_operator(c) ||
			is_semi(c) ||
			is_comma(c) ||
			is_curly_open(c) ||
			is_curly_close(c);
	}

	static auto lookup_keywords(Token::value_type name)
	{
		using namespace std::literals;
		using kw_map = std::unordered_map < Token::value_type, tok_kind>;
		static kw_map keywords
		{
			{"var"sv, Token::KwVar},
			{"fn"sv,  Token::KwFn},
			{"int"sv, Token::KwInt},
			{"float"sv, Token::KwFloat},
			{"bool"sv, Token::KwBool},
			{"true"sv, Token::KwTrue},
			{"false"sv, Token::KwFalse},
			{"return"sv, Token::KwRet},
			{"if"sv, Token::KwIf},
			{"else"sv, Token::KwElse},
			{"while"sv, Token::KwWhile},
		};
		auto found = keywords.find(name);
		return found != keywords.end() ? found->second : Token::Identifier;
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

if (is_id_start(next))
return identifier();
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

		auto tokStr = read_str();
		if (kind == Token::Identifier)
			kind = lookup_keywords(tokStr);

		auto val = kind != Token::Eof ? tokStr : value_type{};
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
		case(';'): res = Token::Semicolon;	break;
		case(','): res = Token::Comma;		break;
		case('{'): res = Token::CurlyOpen;	break;
		case('}'): res = Token::CurlyClose;	break;
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
		case('='):
			if (peek_char() == '=')
			{
				advance();
				res = Token::Eq;
			}
			else
			{
				res = Token::EqSign;
			}
			break;
		case('<'):
			if (peek_char() == '=')
			{
				advance();
				res = Token::LessEq;
			}
			else
			{
				res = Token::Less;
			}
			break;
		case('>'):
			if (peek_char() == '=')
			{
				advance();
				res = Token::GreaterEq;
			}
			else
			{
				res = Token::Greater;
			}
			break;
		case('!'):
			if (peek_char() == '=')
			{
				advance();
				res = Token::NotEq;
			}
			else
			{
				res = Token::Exclamation;
			}
			break;
		case('&'):
			if (peek_char() == '&')
			{
				advance();
				res = Token::LAnd;
			}
			break;
		case('|'):
			if (peek_char() == '|')
			{
				advance();
				res = Token::LOr;
			}
			break;
		}
		return consume(res);
	}
	const Token& Lexer::number()
	{
		auto res = Token::IntNumber;
		while (good())
		{
			const auto next = peek_char();
			if (is_separator(next))
				break;
			if (is_digit(next))
			{
				advance();
				continue;
			}
			if (is_dot(next) && res == Token::IntNumber)
			{
				res = Token::FloatNumber;
				advance();
				continue;
			}
 
			advance();
			res = Token::Error;
		}
		return consume(res);
	}
	const Token& Lexer::identifier()
	{
		auto res = Token::Identifier;
		while (good())
		{
			const auto next = peek_char();
			if (is_separator(next))
				break;
			advance();
			if (is_id_char(next))
				continue;
			res = Token::Error;
		}
		return consume(res);
	}
}
