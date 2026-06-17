#include "evaluator.hpp"
#include "sema.hpp"

#include <charconv>
#include <string>

namespace mpl::eval
{
	namespace
	{
		value literal_value(const Token& token)
		{
			using enum tok_kind;
			switch (token.what())
			{
			case KwTrue:
				return value{ true };
			case KwFalse:
				return value{ false };
			case IntNumber:
			{
				int_type result{};
				auto text = token.value();
				std::from_chars(text.data(), text.data() + text.size(), result);
				return value{ result };
			}
			case FloatNumber:
				return value{ std::stod(std::string{ token.value() }) };
			default:
				return value{};
			}
		}
	}

	evaluator::evaluator()
	{
		m_callStack.emplace_back();
	}

	value evaluator::operator()(const ast::Node& root)
	{
		m_evalStack.clear();
		m_returned = false;
		if (root.what() == ast::node_kind::List)
			return eval_program(static_cast<const ast::list&>(root));

		return eval_node(root);
	}

	bool evaluator::preview(const ast::list& list)
	{
		exec_list(list);
		return false;
	}

	bool evaluator::preview(const ast::func_decl&)
	{
		return false;
	}

	bool evaluator::preview(const ast::param_decl&)
	{
		return false;
	}

	bool evaluator::preview(const ast::var_decl& decl)
	{
		auto val = eval_node(decl.initialiser()).cast_to(decl.type());
		set_value(decl, val);
		return false;
	}

	bool evaluator::preview(const ast::ret_stmt& stmt)
	{
		if (auto expr = stmt.ret_expr())
			push_value(eval_node(*expr));
		else
			push_value(value{});

		m_returned = true;
		return false;
	}

	bool evaluator::preview(const ast::if_stmt& stmt)
	{
		if (eval_node(stmt.condition()).truthy())
		{
			auto result = eval_node(stmt.true_branch());
			if (m_returned)
				push_value(result);
		}
		else if (auto falseBranch = stmt.false_branch())
		{
			auto result = eval_node(*falseBranch);
			if (m_returned)
				push_value(result);
		}
		return false;
	}

	bool evaluator::preview(const ast::while_stmt& stmt)
	{
		while (!m_returned && eval_node(stmt.condition()).truthy())
		{
			auto result = eval_node(stmt.body());
			if (m_returned)
				push_value(result);
		}
		return false;
	}

	bool evaluator::preview(const ast::id_expr& expr)
	{
		push_value(get_value(expr.declaration()));
		return false;
	}

	bool evaluator::preview(const ast::lit_expr& expr)
	{
		push_value(literal_value(expr.value()));
		return false;
	}

	bool evaluator::preview(const ast::binary_expr& expr)
	{
		if (expr.op() != ast::operation::Assign)
			return true;

		auto target = assign_target(expr.left());
		auto val = eval_node(expr.right());
		if (target)
		{
			val = val.cast_to(target->type());
			set_value(*target, val);
		}
		push_value(val);
		return false;
	}

	bool evaluator::preview(const ast::call_expr& expr)
	{
		auto func = sema::extract_function(expr.callee());
		if (!func)
		{
			push_value(value{});
			return false;
		}

		push_value(eval_function(*func, expr.args()));
		return false;
	}

	void evaluator::visit(const ast::unary_expr& expr)
	{
		push_value(pop_value().unary(expr.op()).cast_to(expr.type()));
	}

	void evaluator::visit(const ast::binary_expr& expr)
	{
		if (expr.op() == ast::operation::Assign)
			return;

		auto rhs = pop_value();
		auto lhs = pop_value();
		push_value(lhs.binary(expr.op(), rhs).cast_to(expr.type()));
	}

	void evaluator::visit(const ast::implicit_cast& expr)
	{
		push_value(pop_value().cast_to(expr.type()));
	}

	value evaluator::eval_node(const ast::Node& node)
	{
		const auto before = m_evalStack.size();
		base::operator()(&node);
		if (m_evalStack.size() > before)
			return pop_value();

		return value{};
	}

	value evaluator::eval_function(const ast::func_decl& decl, const ast::list& args)
	{
		std::vector<value> values;
		values.reserve(args.children().size());
		for (auto arg : args.children())
			values.push_back(eval_node(*arg));

		return eval_function(decl, values);
	}

	value evaluator::eval_function(const ast::func_decl& decl, std::span<const value> args)
	{
		m_callStack.emplace_back();

		auto&& params = decl.params();
		const auto paramCount = params.children().size();
		for (ast::list::size_type idx{}; idx < paramCount && idx < args.size(); ++idx)
		{
			auto param = static_cast<const ast::param_decl*>(params.children()[idx]);
			set_value(*param, args[idx].cast_to(param->type()));
		}

		const auto outerReturned = m_returned;
		m_returned = false;
		exec_list(decl.body());
		auto result = pop_value();
		m_returned = outerReturned;

		m_callStack.pop_back();
		return result.cast_to(decl.return_type());
	}

	value evaluator::eval_program(const ast::list& root)
	{
		for (auto node : root.children())
		{
			if (node->what() == ast::node_kind::VarDecl)
				eval_node(*node);
		}

		auto main = find_function(root, "_main");
		if (!main)
			return value{};

		return eval_function(*main, std::span<const value>{});
	}

	void evaluator::exec_list(const ast::list& list)
	{
		for (auto node : list.children())
		{
			if (m_returned)
				break;

			auto result = eval_node(*node);
			if (m_returned)
			{
				push_value(result);
				break;
			}
		}
	}

	const ast::func_decl* evaluator::find_function(const ast::list& list, std::string_view name) const
	{
		for (auto node : list.children())
		{
			if (node->what() != ast::node_kind::FunctionDecl)
				continue;

			auto func = static_cast<const ast::func_decl*>(node);
			if (func->name().value() == name)
				return func;
		}
		return {};
	}

	const ast::decl* evaluator::assign_target(const ast::Node& node) const
	{
		if (node.what() == ast::node_kind::IdExpr)
			return &static_cast<const ast::id_expr&>(node).declaration();

		if (node.what() == ast::node_kind::ImplicitCast)
			return assign_target(static_cast<const ast::implicit_cast&>(node).expr());

		if (node.what() == ast::node_kind::ParenExpr)
			return assign_target(static_cast<const ast::paren_expr&>(node).internal_expr());

		return {};
	}

	value evaluator::pop_value()
	{
		if (m_evalStack.empty())
			return value{};

		auto result = m_evalStack.back();
		m_evalStack.pop_back();
		return result;
	}

	void evaluator::push_value(value val)
	{
		m_evalStack.push_back(val);
	}

	value evaluator::get_value(const ast::decl& decl) const
	{
		for (auto it = m_callStack.rbegin(); it != m_callStack.rend(); ++it)
		{
			if (it->contains(decl))
				return it->get(decl);
		}
		return value{};
	}

	void evaluator::set_value(const ast::decl& decl, value val)
	{
		for (auto it = m_callStack.rbegin(); it != m_callStack.rend(); ++it)
		{
			if (it->contains(decl))
			{
				it->set(decl, val);
				return;
			}
		}
		m_callStack.back().set(decl, val);
	}
}
