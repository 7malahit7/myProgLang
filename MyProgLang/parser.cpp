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
		Assignment,
		LogicalOr,
		LogicalAnd,
		Equality,
		Relational,
		Additive,
		Multiplicative
	};

	constexpr auto match_precedence(tok_kind tk, precedence pr)
	{
		switch (pr)
		{
		case precedence::Assignment:
			return tk == Token::EqSign;
		case precedence::LogicalOr:
			return tk == Token::LOr;
		case precedence::LogicalAnd:
			return tk == Token::LAnd;
		case precedence::Equality:
			return tk == Token::Eq || tk == Token::NotEq;
		case precedence::Relational:
			return tk == Token::Less ||
				tk == Token::LessEq ||
				tk == Token::Greater ||
				tk == Token::GreaterEq;
		case precedence::Additive:
			return tk == Token::Plus || tk == Token::Minus;
		case precedence::Multiplicative:
			return tk == Token::Asterisk || tk == Token::Slash;
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
		m_sema.enter_new_scope();
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
		m_sema.exit_scope();
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
				error(m_lexer.peek(), "Expected ;");
			}
		}
		else if (next.what() == Token::KwFn)
		{
			func_decl();
		}
		else
		{
			auto errPos = m_lexer.next();
			error(errPos, "Expected declaration");
		}
	}

	void Parser::func_decl()
	{
		//KwFn
		m_lexer.next();
		auto name = m_lexer.next();
		if (name.what() != Token::Identifier)
		{
			error(name, "Expected function name");
			return;
		}

		auto decl = m_builder->prepare_func(name);
		m_sema.record(*decl);
		m_sema.enter_new_scope();
		param_list();
		compound_statement();
		m_builder->complete_func(*decl);
		m_sema.exit_scope();
	}

	void Parser::param_list()
	{
		if (m_lexer.peek().what() != Token::ParenOpen)
		{
			error(m_lexer.peek(), "Expected '(' before parameter list");
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
				else if (m_lexer.peek().what() != Token::ParenClose)
				{
					error(m_lexer.peek(), "Expected ',' between parameters");
				}
				return true;
			}, Token::ParenClose);
		if (m_lexer.peek().what() != Token::ParenClose)
		{
			error(m_lexer.peek(), "Expected ')' after parameter list");
		}
		else
		{
			m_lexer.next();
		}
	}

	void Parser::param_decl()
	{
		auto typeName = m_lexer.next();
		if (!is_type_name(typeName.what()))
		{
			error(typeName, "Expected parameter type");
		}
		if (m_lexer.peek().what() != Token::Identifier)
		{
			error(m_lexer.peek(), "Expected parameter name");
			return;
		}
		auto name = m_lexer.next();
		auto param = m_builder->make_param(name);
		m_sema.record(*param);
	}

	void Parser::compound_statement()
	{
		if (m_lexer.peek().what() != Token::CurlyOpen)
		{
			error(m_lexer.peek(), "Expected '{' before compound statement");
			return;
		}
		m_lexer.next();
		m_sema.enter_new_scope();
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
		if (m_lexer.peek().what() != Token::CurlyClose)
		{
			error(m_lexer.peek(), "Expected '}' after compound statement");
		}
		else
		{
			m_lexer.next();
		}
		m_sema.exit_scope();
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
				error(m_lexer.peek(), "Expected ';' after statement");
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
			error(m_lexer.peek(), "Expected '(' before condition");
		}
		else
		{
			// (
			m_lexer.next();
		}

		expr();

		if (m_lexer.peek().what() != Token::ParenClose)
		{
			error(m_lexer.peek(), "Expected ')' after condition");
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
			error(name, "Expected variable name");
			return;
		}
		auto eq = m_lexer.next();
		if (eq.what() != Token::EqSign)
		{
			error(eq, "Expected '=' after variable name");
			return;
		}
		expr();
		auto var = m_builder->make_var(name);
		if (var)
			m_sema.record(*var);
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
			const auto before = m_builder->state_size();
			handler();
			const auto after = m_builder->state_size();
			if (after > before)
				count += after - before;
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
						if (m_lexer.peek().what() != Token::ParenClose)
							error(m_lexer.peek(), "Expected ',' between call arguments");
					}
					return true;
				}, 
				Token::ParenClose);
			if (m_lexer.peek().what() != Token::ParenClose)
			{
				error(m_lexer.peek(), "Expected ')' after call arguments");
			}
			else
			{
				m_lexer.next();
			}
			m_builder->make_call();
		}
	}
	void Parser::binary_expr(std::size_t precIdx)
	{
		static constexpr std::array precOrder
		{
			precedence::Assignment,
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
			auto op = m_lexer.next();
			binary_expr(nextIdx);
			if (is_assign(binary_op(opKind)))
				m_builder->make_assign(op);
			else
				m_builder->make_binary(binary_op(opKind));
		}
	}

	void Parser::error(const Token& at, ast::Builder::error_msg msg)
	{
		m_builder->make_error(at, msg);
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
			auto errPos = m_lexer.next();
			error(errPos, "Unexpected token");
		}
	}
	void Parser::paren_expr()
	{
		m_lexer.next();
		const auto stateSize = m_builder->state_size();
		expr();
		auto closeParen = m_lexer.peek();
		if (closeParen.what() != Token::ParenClose)
		{
			error(closeParen, "Expected )");
			const auto count = m_builder->state_size() - stateSize;
			m_builder->make_list(count);
			m_builder->make_paren();
		}
		else
		{
			m_lexer.next();
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
		auto foundVar = m_sema.lookup(name.value());
		if (!foundVar)
		{
			error(name, "Undeclared identifier");
			return;
		}
		m_builder->make_id(*foundVar);
	}
}
