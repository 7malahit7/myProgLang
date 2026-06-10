#pragma once

#include "lex.hpp"

namespace mpl
{
	class Parser final
	{
	public:
		using input_t = Lexer::value_type;
	
	public:
		Parser() = default;
		~Parser() = default;

		Parser(const Parser&) = delete;
		Parser& operator=(const Parser&) = delete;
		Parser(Parser&&) = delete;
		Parser& operator=(Parser&&) = delete;
	public:
		void operator()(input_t input);
	private:
		void program();
		void expr();

		void add_expr();
		void mul_expr();

		void unary_expr();
		void primary_expr();
		void paren_expr();
		void literal_expr();

	private:
		Lexer m_lexer;

	};
}