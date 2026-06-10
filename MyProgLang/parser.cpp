#include "parser.hpp"
#include <iostream>

namespace mpl
{
	void Parser::operator()(input_t input)
	{
		m_lexer(input);
		program();
	}
	void Parser::program()
	{
		expr();
		std::cout << "done\n";
	}
	void Parser::expr()
	{
		add_expr();
	}
	void Parser::add_expr()
	{
		mul_expr();
		while (true)
		{
			auto&& next = m_lexer.peek();
			const auto kind = next.what();

			if (kind != Token::Plus && kind != Token::Minus)
				break;
			m_lexer.next();
			mul_expr();
			std::cout
				<< "additive-expr "
				<< (kind == Token::Plus ? '+' : '-')
				<< '\n';
		}
	}
	void Parser::mul_expr()
	{
		unary_expr();
		while (true)
		{
			auto&& next = m_lexer.peek();
			const auto kind = next.what();

			if (kind != Token::Asterisk && kind != Token::Slash)
				break;
			m_lexer.next();
			mul_expr();
			std::cout
				<< "multiplicative-expr "
				<< (kind == Token::Asterisk ? '*' : '/')
				<< '\n';
		}
	}
	void Parser::unary_expr()
	{
		auto&& next = m_lexer.peek();
		const auto kind = next.what();

		if (kind != Token::Plus && kind != Token::Minus)
		{
			primary_expr();
			return;
		}
		m_lexer.next();
		unary_expr();
		std::cout << "unary-expr "
			<< (kind == Token::Plus ? '+' : '-')
			<< '\n';
	}

	void Parser::primary_expr()
	{
		auto&& next = m_lexer.peek();
		const auto kind = next.what();
		if (kind == Token::ParenOpen)
		{
			paren_expr();
		}
		else if (kind == Token::Number)
		{
			literal_expr();
		}
		else
		{
			std::cout << "error in primary-expr: " << next.value() << '\n';
		}
	}
	void Parser::paren_expr()
	{
		m_lexer.next();
		expr();
		auto closeParen = m_lexer.next();
		if (closeParen.what() != Token::ParenClose)
		{
			std::cout << "error in paren-expr: " << closeParen.value() << '\n';
		}
		else
		{
			std::cout << "paren-expr\n";
		}
	}
	void Parser::literal_expr()
	{
		auto num = m_lexer.next();
		std::cout << "literal-expr: " << num.value() << '\n';
	}
}