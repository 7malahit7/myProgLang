#pragma once
#include "ast.hpp"
#include <memory>
#include <forward_list>
#include <vector>
#include "token.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "stmt.hpp"


namespace mpl::ast
{
	class Builder final
	{
	public:
		using node_ptr = std::unique_ptr<Node>;
		using node_storage = std::forward_list<node_ptr>;
		using node_container = std::vector<Node*>;
		using error_msg = std::string_view;

	public:
		Builder() = default;
		~Builder() = default;

		Builder(const Builder&) = delete;
		Builder& operator= (const  Builder&) = delete;
		Builder(Builder&&) = delete;
		Builder& operator= (Builder&&) = delete;

	public:
		const Node* root() const;
		const Node* last() const;
		node_container::size_type state_size() const;

		void clear_state();

		void make_paren();
		void make_literal(const Token& value);
		void make_id(decl& d);
		void make_unary(operation op);
		void make_binary(operation op);
		void make_assign(const Token& op);

		decl* make_var(const Token& name);
		decl* make_param(const Token& name, types::type t);
		func_decl* prepare_func(const Token& name);
		void specify_func_params(func_decl& decl);
		void complete_func(func_decl& decl);
		void make_empty_ret();
		void make_ret();
		void make_if(bool hasElse);
		void make_call();
		void make_list(list::size_type count);
		void make_error(const Token& at, error_msg message);
	private:
		template<ast_node T, typename ...Args> 
			requires std::constructible_from<T, Args...>
		void make(Args&& ...args);

		Node* extract();
		Node* wrap_in_cast(Node& node, types::type to);

	private:
		node_storage m_nodes;
		node_container m_state;
		Node* m_root{};

	};

}
