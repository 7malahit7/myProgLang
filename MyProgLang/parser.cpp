#include "parser.hpp"
#include <array>
namespace mpl 
{
	constexpr auto unary_op(tok_kind tk)
	{
		switch (tk)
		{
		case tok_kind::Plus:  return ast::operation::UnaryPos;
		case tok_kind::Minus: return ast::operation::UnaryNeg;
		case tok_kind::Exclamation: return ast::operation::UnaryNot;
			
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

		case tok_kind::EqSign: return ast::operation::Assign;
		case tok_kind::Eq: return ast::operation::Equal;
		case tok_kind::NotEq: return ast::operation::NEqual;
		case tok_kind::Less: return ast::operation::Less;
		case tok_kind::Greater: return ast::operation::Greater;
		case tok_kind::LessEq: return ast::operation::LessEq;
		case tok_kind::GreaterEq: return ast::operation::GreaterEq;
		case tok_kind::LAnd: return ast::operation::LogAnd;
		case tok_kind::LOr: return ast::operation::LogOr;

		default: return ast::operation::Unknown;
		}
	}
	constexpr auto is_assign(ast::operation op)
	{
		return op == ast::operation::Assign;
	}
	constexpr auto is_type_name(tok_kind tk)
	{
		return tk == Token::KwInt ||
				tk == Token::KwFloat ||
				tk == Token::KwBool;
	}
	constexpr auto is_literal(tok_kind tk)
	{
		return tk == Token::IntNumber ||
			tk == Token::FloatNumber ||
			tk == Token::KwFalse ||
			tk == Token::KwTrue;
	}
	enum class precedence : std::uint8_t
	{
		Additive,
		Multiplicative,
		Relational,
		Equality,
		LogicalAnd,
		LogicalOr
	};

	constexpr auto match_precedence(tok_kind tk, precedence pr)
	{
		switch (pr)
		{
		case precedence::Additive:
			return tk == Token::Plus || tk == Token::Minus;
		case precedence::Multiplicative:
			return tk == Token::Asterisk || tk == Token::Slash;
		case precedence::Relational:
			return tk == Token::Less ||
				tk == Token::LessEq ||
				tk == Token::Greater ||
				tk == Token::GreaterEq;
		case precedence::Equality:
			return tk == Token::Eq || tk == Token::NotEq;
		case precedence::LogicalAnd:
			return tk == Token::LAnd;
		case precedence::LogicalOr:
			return tk == Token::LOr;
		}
		return false;
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
	bool Parser::good() 
	{
		return m_lexer.peek().what() != Token::Eof;
	}
	void Parser::program()
	{
		list([&]
			{
				if (m_lexer.peek().what() == Token::Semicolon)
				{
					m_lexer.next();
					return false;
				}
				declaration();
				return true;
			}, Token::Eof);
		m_builder->clear_state();
	}
	void Parser::declaration()
	{
		auto&& next = m_lexer.peek();
		if (next.what() == Token::KwVar)
		{
			var_decl();
			if (m_lexer.peek().what() == Token::Semicolon)
			{
				m_lexer.next();
			}
			else
			{
				//error
			}
		}
		else if (next.what() == Token::KwFn)
		{
			func_decl();
		}
	}

	void Parser::func_decl()
	{
		//KwFn
		m_lexer.next();
		auto name = m_lexer.next();
		if (name.what() != Token::Identifier)
		{
			// error
		}

		auto decl = m_builder->prepare_func(name);
		m_symTab[name.value()] = decl;
		param_list();
		compound_statement();
		m_builder->complete_func(*decl);
	}

	void Parser::param_list()
	{
		if (m_lexer.peek().what() != Token::ParenOpen)
		{
			//error
			return;
		}
		m_lexer.next();
		list([&]
			{
				param_decl();
				if (m_lexer.peek().what() == Token::Comma)
				{
					m_lexer.next();
				}
				return true;
			}, Token::ParenClose);
		// )
		m_lexer.next();
	}

	void Parser::param_decl()
	{
		auto typeName = m_lexer.next();
		if (!is_type_name(typeName.what()))
		{
			// error
		}
		if (m_lexer.peek().what() != Token::Identifier)
		{
			// error
		}
		auto name = m_lexer.next();
		auto param = m_builder->make_param(name);
		m_symTab[name.value()] = param;
	}

	void Parser::compound_statement()
	{
		if (m_lexer.peek().what() != Token::CurlyOpen)
		{
			// error
			return;
		}
		m_lexer.next();
		list([&]
			{
				if (m_lexer.peek().what() == Token::Semicolon)
				{
					m_lexer.next();
					return false;
				}
				statement(true);
				return true;
			}, Token::CurlyClose);
		m_lexer.next();
	}

	void Parser::statement(bool requireSemi)
	{
		bool needsSemi{};
		switch (m_lexer.peek().what())
		{
		case Token::KwRet:
			return_statement();
			needsSemi = true;
			break;
		case Token::KwIf:
			if_statement();
			break;
		case Token::CurlyOpen:
			compound_statement();
			break;
		default:
			expr();
			needsSemi = true;
			break;
		}

		if (needsSemi)
		{
			if (m_lexer.peek().what() == Token::Semicolon)
			{
				m_lexer.next();
			}
			else
			{
				// error
			}
		}
	}

	void Parser::return_statement()
	{
		m_lexer.next();
		if (m_lexer.peek().what() == Token::Semicolon)
		{
			m_builder->make_empty_ret();
			return;
		}
		expr();
		m_builder->make_ret();
	}

	void Parser::condition()
	{
		if (m_lexer.peek().what() != Token::ParenOpen)
		{
			// error
		}
		else
		{
			// (
			m_lexer.next();
		}

		expr();

		if (m_lexer.peek().what() != Token::ParenClose)
		{
			// error
		}
		else
		{
			m_lexer.next();
		}
	}

	void Parser::if_statement()
	{
		//if
		m_lexer.next();
		condition();
		statement(false);
		bool hasElse{};
		if (m_lexer.peek().what() == Token::KwElse)
		{
			//else
			m_lexer.next();
			statement(false);
			hasElse = true;
		}
		m_builder->make_if(hasElse);
	}

	void Parser::var_decl()
	{
		m_lexer.next();
		auto name = m_lexer.next();
		if (name.what() != Token::Identifier)
		{
			return;
		}
		auto eq = m_lexer.next();
		if (eq.what() != Token::EqSign)
		{
			return;
		}
		expr();
		auto var = m_builder->make_var(name);
		m_symTab[name.value()] = var;
	}
	void Parser::expr()
	{
		binary_expr(0);
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
		const auto unaryOp = unary_op(next.what());

		if (unaryOp == ast::operation::Unknown)
		{
			postfix_expr();
			return;
		}
		m_lexer.next();
		unary_expr();
		m_builder->make_unary(unaryOp);
	}

	void Parser::list(list_handler auto handler, tok_kind breakOn)
	{
		ast::list::size_type count{};
		while (good())
		{
			if (m_lexer.peek().what() == breakOn)
				break;
			if (handler())
				++count;
		}
		m_builder->make_list(count);
	}


	void Parser::postfix_expr()
	{
		primary_expr();
		while (good())
		{
			if (m_lexer.peek().what() != Token::ParenOpen)
				break;
			m_lexer.next();

			list([&]
				{
					expr();
					if (m_lexer.peek().what() != Token::Comma)
					{
						//error
					}
					return true;
				}, 
				Token::ParenClose);
			m_lexer.next();
			m_builder->make_call();
		}
	}
	void Parser::binary_expr(std::size_t precIdx)
	{
		static constexpr std::array precOrder
		{
			precedence::LogicalOr,
			precedence::LogicalAnd,
			precedence::Equality,
			precedence::Relational,
			precedence::Additive,
			precedence::Multiplicative
		};
		if (precIdx == precOrder.size())
		{
			unary_expr();
			return;
		}
		const auto nextIdx = precIdx + 1;
		binary_expr(nextIdx);
		while (good())
		{
			const auto opKind = m_lexer.peek().what();
			if (!match_precedence(opKind, precOrder[precIdx]))
				break;
			m_lexer.next();
			binary_expr(nextIdx);
			m_builder->make_binary(binary_op(opKind));
		}
	}

	void Parser::primary_expr()
	{
		auto&& next = m_lexer.peek();
		const auto kind = next.what();
		if (kind == Token::ParenOpen)
		{
			paren_expr();
		}
		else if (is_literal(kind))
		{
			literal_expr();
		}
		else if (kind == Token::Identifier)
		{
			return id_expr();
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
	void Parser::id_expr()
	{
		auto name = m_lexer.next();
		auto foundVar = m_symTab.find(name.value());
		if (foundVar == m_symTab.end())
		{
			//error
			return;
		}
		auto decl = foundVar->second;
		m_builder->make_id(*decl); 
	}
}
