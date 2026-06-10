#include "expr.hpp"
#include "decl.hpp"

namespace mpl::ast
{
	lit_expr::lit_expr(const Token& val)
		: Expr(Node::LiteralExpr), m_value(val)
	{
	}
	const Token& lit_expr::value() const
	{
		return m_value;
	}

	id_expr::id_expr(decl& declaration)
		: Expr( Node::IdExpr),
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
		: Expr(Node::ParenExpr), m_expr(&internal)
	{
	}

	const Node& paren_expr::internal_expr() const
	{
		return *m_expr;
	}

	unary_expr::unary_expr(Node& operand, operation op)
		: Expr(Node::UnaryExpr), m_operand(&operand), m_op(op)
	{
	}

	const Node& unary_expr::operand() const
	{
		return *m_operand;
	}

	operation unary_expr::op() const
	{
		return m_op;
	}

	binary_expr::binary_expr(Node& lhs, Node& rhs, operation op)
		: Expr(Node::BinaryExpr), m_lhs(&lhs), m_rhs(&rhs),m_op(op)
	{
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
}