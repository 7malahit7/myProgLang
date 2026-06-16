#include "expr.hpp"
#include "decl.hpp"

namespace mpl::ast
{
	lit_expr::lit_expr(const Token& val, types::type t)
		: Expr(Node::LiteralExpr, t), m_value(val)
	{
	}
	const Token& lit_expr::value() const
	{
		return m_value;
	}

	id_expr::id_expr(decl& declaration)
		: Expr( Node::IdExpr, declaration.type()),
		  m_decl(& declaration)
	{
	}

	const decl& id_expr::declaration() const
	{
		return *m_decl;
	}

	const Token& id_expr::name() const
	{
		return m_decl->name();
	}

	paren_expr::paren_expr(Node& internal)
		: Expr(Node::ParenExpr)
	{
		m_children.push_back(&internal);
		validate(&internal);
	}

	paren_expr::paren_expr(data_type items)
		: Expr(Node::ParenExpr), m_children(std::move(items))
	{
		for (auto item : m_children)
			validate(item);
	}

	const Node& paren_expr::internal_expr() const
	{
		return *m_children.front();
	}

	const paren_expr::data_type& paren_expr::children() const
	{
		return m_children;
	}

	unary_expr::unary_expr(Node& operand, operation op, types::type t)
		: Expr(Node::UnaryExpr, t), m_operand(&operand), m_op(op)
	{
		validate(&operand);
	}

	const Node& unary_expr::operand() const
	{
		return *m_operand;
	}

	operation unary_expr::op() const
	{
		return m_op;
	}

	binary_expr::binary_expr(Node& lhs, Node& rhs, operation op, types::type t)
		: Expr(Node::BinaryExpr, t), m_lhs(&lhs), m_rhs(&rhs),m_op(op)
	{
		validate(&lhs);
		validate(&rhs);
	}

	const Node& binary_expr::left() const
	{
		return *m_lhs;
	}

	const Node& binary_expr::right() const
	{
		return *m_rhs;
	}

	operation binary_expr::op() const
	{
		return m_op;
	}

	call_expr::call_expr(Node& callee, const list& args, types::type t)
		: Expr(Node::CallExpr, t), m_callee(&callee), m_args(&args)
	{
		validate(&callee);
		validate(&args);
	}

	const Node& call_expr::callee() const
	{
		return *m_callee;
	}

	const list& call_expr::args() const
	{
		return *m_args;
	}

	implicit_cast::implicit_cast(Node& expr, types::type t)
		: Expr(Node::ImplicitCast, t), m_expr(&expr)
	{
		validate(&expr);
	}

	const Node& implicit_cast::expr() const
	{
		return *m_expr;
	}
}
