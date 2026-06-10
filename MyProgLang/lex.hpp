#pragma once

#include "token.hpp"
#include <optional>

namespace mpl
{
	class Lexer final
	{
	public:
		using value_type = Token::value_type;
		using iter = value_type::iterator;
		using char_t = value_type::value_type;
		using token_opt = std::optional<Token>;
	public:
		Lexer();
		~Lexer() = default;

		Lexer(const Lexer&) = default;
		Lexer& operator= (const Lexer&) = default;

		Lexer(Lexer&&) = default;
		Lexer& operator=(Lexer&&) = default;

	public:
		void operator()(value_type input);
		Token next();
		const Token& peek();
	private:
		bool good() const;

		char_t peek_char() const;

		void advance();

		value_type read_str() const;

		void skip_spaces();

		const Token& consume(tok_kind kind);

		const Token& punctuation();
		const Token& op();
		const Token& number();


	private:
		value_type m_buffer{};
		iter m_start;
		iter m_end;
		token_opt m_preview{};
	};
}