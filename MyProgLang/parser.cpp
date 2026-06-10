#include "parser.hpp"

namespace mpl 
{
	constexpr auto unary_op(tok_kind tk)
	{
		switch (tk)
		{
		case tok_kind::Plus:  return ast::operation::UnaryPos;
		case tok_kind::Minus: return ast::operation::UnaryNeg;
			
		default: return ast::operation::Unknown;
		}
	}

	constexpr auto binary_op(tok_kind tk)
	{
		switch (tk)
		{
		case tok_kind::Asterisk:  return ast::operation::Multiplication;
		case tok_kind::Slash: return ast::operation::Division;
		case tok_kind::Plus:  return ast::operation::Addition;
		case tok_kind::Minus: return ast::operation::Subtraction;

		default: return ast::operation::Unknown;
		}
	}
}

namespace mpl
{
	Parser::Parser(ast::Builder& builder)
		:m_builder(&builder)
	{
	}
	void Parser::operator()(input_t input)
	{
		m_lexer(input);
		program();
	}
	void Parser::program()
	{
		expr();
		m_builder->clear_state();
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
			m_builder->make_binary(binary_op(kind));
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
			m_builder->make_binary(binary_op(kind));
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
		m_builder->make_unary(unary_op(kind));
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
			// error 
		}
	}
	void Parser::paren_expr()
	{
		m_lexer.next();
		expr();
		auto closeParen = m_lexer.next();
		if (closeParen.what() != Token::ParenClose)
		{
			// error 
		}
		else
		{
			m_builder->make_paren();
		}
	}
	void Parser::literal_expr()
	{
		auto num = m_lexer.next();
		m_builder->make_literal(num);
	}
}