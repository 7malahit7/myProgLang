#pragma once

#include "token.hpp"
#include "base.hpp"

namespace mpl::ast
{
	class decl;
	class Expr : public Node
	{
	public:
		Expr() = delete;
		~Expr() = default;

		Expr(const Expr&) = delete;
		Expr& operator= (const  Expr&) = delete;
		Expr(Expr&&) = delete;
		Expr& operator= (Expr&&) = delete;

		explicit Expr(node_kind kind) noexcept :
			Node(kind)
		{
		}
	};


	class lit_expr : public Expr
	{
	public:
		lit_expr() = delete;
		~lit_expr() = default;

		lit_expr(const lit_expr&) = delete;
		lit_expr& operator= (const  lit_expr&) = delete;
		lit_expr(lit_expr&&) = delete;
		lit_expr& operator= (lit_expr&&) = delete;

		explicit lit_expr(const Token& val);
		
	public:
		const Token& value() const;

	private:
		Token m_value;

	};

	class id_expr : public Expr
	{
	public:
		id_expr() = delete;
		~id_expr() = default;

		id_expr(const id_expr&) = delete;
		id_expr& operator= (const  id_expr&) = delete;
		id_expr(id_expr&&) = delete;
		id_expr& operator= (id_expr&&) = delete;

		explicit id_expr(decl& declaration);

	public:
		const decl& declaration() const;
		const Token& name() const;
	private:
		decl* m_decl;
	};

	class paren_expr : public Expr
	{
	public:
		paren_expr() = delete;
		~paren_expr() = default;

		paren_expr(const paren_expr&) = delete;
		paren_expr& operator= (const  paren_expr&) = delete;
		paren_expr(paren_expr&&) = delete;
		paren_expr& operator= (paren_expr&&) = delete;

		explicit paren_expr(Node& internal);

	public:
		const Node& internal_expr() const;

	private:
		Node* m_expr;
	};

	enum class operation : std::uint8_t
	{
		Unknown,
		UnaryNeg,
		UnaryPos,
		Addition,
		Subtraction,
		Multiplication,
		Division,
		UnaryNot,
		Assign,
		Equal,
		NEqual,
		Less,
		Greater,
		LessEq,
		GreaterEq,
		LogAnd,
		LogOr
	};

	class unary_expr : public Expr
	{
	public:
		unary_expr() = delete;
		~unary_expr() = default;

		unary_expr(const unary_expr&) = delete;
		unary_expr& operator= (const  unary_expr&) = delete;
		unary_expr(unary_expr&&) = delete;
		unary_expr& operator= (unary_expr&&) = delete;

		unary_expr(Node& operand, operation op);

	public:
		const Node& operand() const;
		operation op() const;
	private:
		Node* m_operand;
		operation m_op;
	};

	class binary_expr : public Expr
	{
	public:
		binary_expr() = delete;
		~binary_expr() = default;

		binary_expr(const binary_expr&) = delete;
		binary_expr& operator= (const  binary_expr&) = delete;
		binary_expr(binary_expr&&) = delete;
		binary_expr& operator= (binary_expr&&) = delete;

		binary_expr(Node& lhs, Node& rhs, operation op );

	public:
		const Node& left() const;
		const Node& right() const;

		operation op() const;
	private:
		Node* m_lhs;
		Node* m_rhs;
		operation m_op;
	};
}
