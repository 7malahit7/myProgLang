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

	void Builder::make_id(decl& d)
	{
		make<id_expr>(d);
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

	decl* Builder::make_var(const Token& name)
	{
		auto init = extract();
		if (init)
		{
			make<var_decl>(name, *init);
		}
		else
		{
			return {};
		}
		return static_cast<decl*>(m_root);
	}

	decl* Builder::make_param(const Token& name)
	{
		make<param_decl>(name);
		return static_cast<decl*>(m_root);
	}

	func_decl* Builder::prepare_func(const Token& name)
	{
		make<func_decl>(name);
		return static_cast<func_decl*>(m_root);
	}

	void Builder::complete_func(func_decl& decl)
	{
		auto body = extract();
		auto params = extract();
		if (params && body)
		{
			decl.complete(static_cast<const list&>(*params), static_cast<const list&>(*body));
		}
	}

	void Builder::make_empty_ret()
	{
		make<ret_stmt>(nullptr);
	}

	void Builder::make_ret()
	{
		make<ret_stmt>(extract());
	}

	void Builder::make_if(bool hasElse)
	{
		auto falseBranch = hasElse ? extract() : nullptr;
		auto trueBranch = extract();
		auto condition = extract();
		if (condition && trueBranch)
		{
			make<if_stmt>(*condition, *trueBranch, falseBranch);
		}
	}

	void Builder::make_call()
	{
	}

	void Builder::make_list(list::size_type count)
	{
		const auto availableSz = static_cast<list::size_type>(m_state.size());
		if (availableSz < count)
		{
			//error
			return;
		}
		const auto startPos = availableSz - count;
		auto beg = std::next(m_state.begin(), startPos);
		list::data_type items;
		items.reserve(count);
		for (auto it = beg; it != m_state.end(); ++it)
		{
			items.push_back(*it);
		}
		m_state.erase(beg, m_state.end());
		make<list>(std::move(items));
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
