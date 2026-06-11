#pragma once
#include <cstdint>
#include <vector>

namespace mpl::ast
{
	enum class node_kind : std::uint8_t 
	{
		List,

		LiteralExpr,
		IdExpr,
		ParenExpr,
		UnaryExpr,
		BinaryExpr,

		VarDecl,
		ParamDecl,
		FunctionDecl,

		RetStmt,
		IfStmt
	};

	class Node
	{
	public:
		using enum node_kind;
	public:
		Node() = delete;
		virtual ~Node() = default;

		Node(const Node&) = delete;
		Node& operator= (const  Node&) = delete;
		Node(Node&&) = delete;
		Node& operator= (Node&&) = delete;
		
		explicit Node(node_kind kind) 
			: m_kind {kind}
		{
		}
	public:
		node_kind what() const
		{
			return m_kind;
		}
	private:
		node_kind m_kind;
	};

	class list final : public Node
	{
	public:
		using data_type = std::vector<const Node*>;
		using size_type = data_type::size_type;
	public:
		list() = delete;
		virtual ~list() = default;

		list(const list&) = delete;
		list& operator= (const  list&) = delete;
		list(list&&) = delete;
		list& operator= (list&&) = delete;

		explicit list(data_type items)
			: Node(Node::List), m_data(std::move(items))
		{
		}
	public:
		void attach(Node& n)
		{
			m_data.push_back(&n);
		}
		const data_type& children() const
		{
			return m_data;
		}
	private:
		data_type m_data;
	};
}
