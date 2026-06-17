#pragma once
#include "token.hpp"
#include "base.hpp"

namespace mpl::ast
{
	class ret_stmt : public Node
	{
	public:
		ret_stmt() = delete;
		~ret_stmt() = default;

		ret_stmt(const ret_stmt&) = delete;
		ret_stmt& operator=(const ret_stmt&) = delete;
		ret_stmt(ret_stmt&&) = delete;
		ret_stmt& operator=(ret_stmt&&) = delete;

		explicit ret_stmt(Node* retExpr);
	public:
		const Node* ret_expr() const;
	private:
		Node* m_retExpr{};
	};

	class if_stmt : public Node
	{
	public:
		if_stmt() = delete;
		~if_stmt() = default;

		if_stmt(const if_stmt&) = delete;
		if_stmt& operator=(const if_stmt&) = delete;
		if_stmt(if_stmt&&) = delete;
		if_stmt& operator=(if_stmt&&) = delete;

		if_stmt(Node& cond, Node& trueBranch, Node* falseBranch);
	public:
		const Node& condition() const;
		const Node& true_branch() const;
		const Node* false_branch() const;
	private:
		Node* m_condition{};
		Node* m_trueBranch{};
		Node* m_falseBranch{};
	};

	class while_stmt : public Node
	{
	public:
		while_stmt() = delete;
		~while_stmt() = default;

		while_stmt(const while_stmt&) = delete;
		while_stmt& operator=(const while_stmt&) = delete;
		while_stmt(while_stmt&&) = delete;
		while_stmt& operator=(while_stmt&&) = delete;

		while_stmt(Node& cond, Node& body);
	public:
		const Node& condition() const;
		const Node& body() const;
	private:
		Node* m_condition{};
		Node* m_body{};
	};

	inline ret_stmt::ret_stmt(Node* retExpr)
		: Node(Node::RetStmt), m_retExpr(retExpr)
	{
		validate(retExpr);
	}

	inline const Node* ret_stmt::ret_expr() const
	{
		return m_retExpr;
	}

	inline if_stmt::if_stmt(Node& cond, Node& trueBranch, Node* falseBranch)
		: Node(Node::IfStmt),
		  m_condition(&cond),
		  m_trueBranch(&trueBranch),
		  m_falseBranch(falseBranch)
	{
		validate(&cond);
		validate(&trueBranch);
		validate(falseBranch);
	}

	inline const Node& if_stmt::condition() const
	{
		return *m_condition;
	}

	inline const Node& if_stmt::true_branch() const
	{
		return *m_trueBranch;
	}

	inline const Node* if_stmt::false_branch() const
	{
		return m_falseBranch;
	}

	inline while_stmt::while_stmt(Node& cond, Node& body)
		: Node(Node::WhileStmt),
		  m_condition(&cond),
		  m_body(&body)
	{
		validate(&cond);
		validate(&body);
	}

	inline const Node& while_stmt::condition() const
	{
		return *m_condition;
	}

	inline const Node& while_stmt::body() const
	{
		return *m_body;
	}
}
