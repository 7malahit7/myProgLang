#include "builder.hpp"

namespace mpl::ast
{
	template <ast_node T, typename ...Args>
		requires std::constructible_from<T, Args...>
	void Builder::make(Args&& ...args)
	{
		auto newNode = m_nodes.emplace_front(std::make_unique<T>(std::forward<Args>(args)...)).get();
		m_state.push_back(newNode);
		m_root = newNode;
	}

	const Node* Builder::root() const
	{
		return m_root;
	}

	void Builder::clear_state()
	{
		m_state.clear();
	}

	void Builder::make_paren()
	{
		if (auto internal = extract())
		{
			make<paren_expr>(*internal);
		}
	}

	void Builder::make_literal(const Token& value)
	{
		make<lit_expr>(value);
	}

	void Builder::make_unary(operation op)
	{
		if (auto operand = extract())
		{
			make<unary_expr>(*operand ,op);
		}
	}

	void Builder::make_binary(operation op)
	{
		auto rhs = extract();
		auto lhs = extract();
		if (lhs && rhs)
		{
			make<binary_expr>(*lhs, *rhs, op);
		}
	}

	Node* Builder::extract()
	{
		if (m_state.empty())
			return {};
		auto item = m_state.back();
		m_state.pop_back();
		return item;
	}
}