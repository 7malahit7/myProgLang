#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include "token.hpp"

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
		IfStmt,
		Error
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
		
		explicit Node(node_kind kind, bool valid = true) 
			: m_kind {kind}, m_valid(valid)
		{
		}
	protected:
		void validate(const Node* child) noexcept
		{
			if (!child)
				return;

			if (!child->is_valid())
				m_valid = false;
		}
	public:
		node_kind what() const
		{
			return m_kind;
		}
		bool is_valid() const noexcept
		{
			return m_valid;
		}
	private:
		node_kind m_kind;
		bool m_valid{};
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
			for (auto item : m_data)
				validate(item);
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

	class error : public Node
	{
	public:
		using msg_type = std::string_view;
	public:
		error() = delete;
		virtual ~error() = default;

		error(const error&) = delete;
		error& operator= (const  error&) = delete;
		error(error&&) = delete;
		error& operator= (error&&) = delete;

		explicit error(const Token& tok, msg_type msg)
			: Node{ Node::Error, false }, m_at(tok), m_message(msg)
		{
		}
	public:
		const Token& at() const
		{
			return m_at;
		}
		msg_type message() const
		{
			return m_message;
		}
	private:
		Token m_at;
		msg_type m_message;
	};
}
