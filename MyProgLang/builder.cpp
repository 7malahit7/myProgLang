#include "builder.hpp"
#include "sema.hpp"


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

	const Node* Builder::last() const
	{
		if (m_state.empty())
			return {};
		return m_state.back();
	}

	Builder::node_container::size_type Builder::state_size() const
	{
		return m_state.size();
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
		make<lit_expr>(value, sema::literal_type(value));
	}

	void Builder::make_id(decl& d)
	{
		make<id_expr>(d);
	}

	void Builder::make_unary(operation op)
	{
		if (auto operand = extract())
		{
			const auto unaryType = sema::unary_type(*operand, op);
			if (unaryType.m_opType != unaryType.m_opToType)
				operand = wrap_in_cast(*operand, unaryType.m_opToType);

			make<unary_expr>(*operand ,op, unaryType.m_exprType);
		}
	}

	void Builder::make_binary(operation op)
	{
		auto rhs = extract();
		auto lhs = extract();
		if (lhs && rhs)
		{
			const auto binaryType = sema::binary_type(*lhs, *rhs, op);
			if (binaryType.m_lhsType != binaryType.m_lhsToType)
				lhs = wrap_in_cast(*lhs, binaryType.m_lhsToType);
			if (binaryType.m_rhsType != binaryType.m_rhsToType)
				rhs = wrap_in_cast(*rhs, binaryType.m_rhsToType);

			make<binary_expr>(*lhs, *rhs, op, binaryType.m_exprType);
		}
	}

	void Builder::make_assign(const Token& op)
	{
		auto rhs = extract();
		auto lhs = extract();
		if (!lhs || !rhs)
			return;

		if (lhs->what() != Node::IdExpr)
		{
			if (lhs->what() == Node::ParenExpr)
			{
				auto paren = static_cast<paren_expr*>(lhs);
				auto err = m_nodes.emplace_front(
					std::make_unique<error>(op, "Expected an identifier")).get();

				paren_expr::data_type items;
				items.push_back(&paren->internal_expr());
				items.push_back(err);

				auto replacement = m_nodes.emplace_front(std::make_unique<paren_expr>(std::move(items))).get();
				lhs = replacement;
			}
			else
			{
				m_state.push_back(lhs);
				m_root = lhs;
				make_error(op, "Expected an identifier");
				make_list(2);
				lhs = extract();
			}
		}
		const auto initType = sema::extract_type(*rhs);
		const auto varType = sema::extract_type(*lhs);
		if (initType != varType)
			rhs = wrap_in_cast(*rhs, varType);

		make<binary_expr>(*lhs, *rhs, operation::Assign, varType);
	}

	decl* Builder::make_var(const Token& name)
	{
		auto init = extract();
		if (init)
		{
			make<var_decl>(name, *init, sema::extract_type(*init));
		}
		else
		{
			return {};
		}
		return static_cast<decl*>(m_root);
	}

	decl* Builder::make_param(const Token& name, types::type t)
	{
		make<param_decl>(name, t);
		return static_cast<decl*>(m_root);
	}

	func_decl* Builder::prepare_func(const Token& name)
	{
		make<func_decl>(name);
		return static_cast<func_decl*>(m_root);
	}

	void Builder::specify_func_params(func_decl& decl)
	{
		auto params = last();
		if (params && params->what() == Node::List)
			decl.specify_params(static_cast<const list&>(*params));
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
		node_container errors;
		auto extract_branch = [&]() -> Node*
			{
				while (auto lastNode = last())
				{
					if (lastNode->what() != Node::Error)
						break;
					errors.push_back(extract());
				}
				return extract();
			};

		auto falseBranch = hasElse ? extract_branch() : nullptr;
		auto trueBranch = extract_branch();
		auto condition = extract_branch();
		if (condition && trueBranch)
		{
			make<if_stmt>(*condition, *trueBranch, falseBranch);
		}
		for (auto it = errors.rbegin(); it != errors.rend(); ++it)
		{
			m_state.push_back(*it);
			m_root = *it;
		}
	}

	void Builder::make_while()
	{
		auto body = extract();
		auto condition = extract();
		if (condition && body)
		{
			make<while_stmt>(*condition, *body);
		}
	}

	void Builder::make_call()
	{
		auto args = extract();
		auto callee = extract();
		if (!callee || !args)
			return;

		auto func = sema::extract_function(*callee);
		if (!func)
		{
			make<call_expr>(*callee, static_cast<const list&>(*args), types::type::Error);
			return;
		}

		list::data_type items;
		auto&& callArgs = static_cast<const list&>(*args);
		auto&& params = func->params();
		items.reserve(callArgs.children().size());
		const auto argCount = callArgs.children().size();
		const auto paramCount = params.children().size();
		for (list::size_type idx{}; idx < argCount; ++idx)
		{
			auto item = const_cast<Node*>(callArgs.children()[idx]);
			if (idx < paramCount)
			{
				auto param = static_cast<const param_decl*>(params.children()[idx]);
				const auto argType = sema::extract_type(*item);
				if (argType != param->type())
					item = wrap_in_cast(*item, param->type());
			}
			items.push_back(item);
		}

		auto convertedArgs = m_nodes.emplace_front(std::make_unique<list>(std::move(items))).get();
		make<call_expr>(*callee, static_cast<const list&>(*convertedArgs), func->return_type());
	}

	void Builder::make_list(list::size_type count)
	{
		const auto availableSz = static_cast<list::size_type>(m_state.size());
		if (availableSz < count)
		{
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

	Node* Builder::wrap_in_cast(Node& node, types::type to)
	{
		return m_nodes.emplace_front(std::make_unique<implicit_cast>(node, to)).get();
	}

	void Builder::make_error(const Token& at, error_msg message)
	{
		make<error>(at, message);
	}
}
