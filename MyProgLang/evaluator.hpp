#pragma once

#include "ast.hpp"
#include "value.hpp"
#include "value_map.hpp"
#include "visitor.hpp"

#include <span>
#include <string_view>
#include <vector>

namespace mpl::eval
{
	class evaluator final : public ast::Visitor<evaluator>
	{
	public:
		using base = ast::Visitor<evaluator>;
		using eval_stack = std::vector<value>;
		using call_stack = std::vector<value_map>;

	public:
		evaluator();
		~evaluator() = default;

		evaluator(const evaluator&) = delete;
		evaluator& operator=(const evaluator&) = delete;
		evaluator(evaluator&&) = delete;
		evaluator& operator=(evaluator&&) = delete;

	public:
		value operator()(const ast::Node& root);

		bool preview(const ast::list& list);
		bool preview(const ast::func_decl& decl);
		bool preview(const ast::param_decl& decl);
		bool preview(const ast::var_decl& decl);
		bool preview(const ast::ret_stmt& stmt);
		bool preview(const ast::if_stmt& stmt);
		bool preview(const ast::while_stmt& stmt);
		bool preview(const ast::id_expr& expr);
		bool preview(const ast::lit_expr& expr);
		bool preview(const ast::binary_expr& expr);
		bool preview(const ast::call_expr& expr);

		void visit(const ast::unary_expr& expr);
		void visit(const ast::binary_expr& expr);
		void visit(const ast::implicit_cast& expr);

	private:
		value eval_node(const ast::Node& node);
		value eval_function(const ast::func_decl& decl, const ast::list& args);
		value eval_function(const ast::func_decl& decl, std::span<const value> args);
		value eval_program(const ast::list& root);
		void exec_list(const ast::list& list);

		const ast::func_decl* find_function(const ast::list& list, std::string_view name) const;
		const ast::decl* assign_target(const ast::Node& node) const;

		value pop_value();
		void push_value(value val);
		value get_value(const ast::decl& decl) const;
		void set_value(const ast::decl& decl, value val);

	private:
		eval_stack m_evalStack;
		call_stack m_callStack;
		bool m_returned{};
	};
}
