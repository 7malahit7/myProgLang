#pragma once
#include <cstdint>

namespace mpl::ast
{
	enum class node_kind : std::uint8_t
	{
		LiteralExpr,
		ParenExpr,
		UnaryExpr,
		BinaryExpr
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
}