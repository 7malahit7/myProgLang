#pragma once
#include "token.hpp"
#include "base.hpp"

namespace mpl::ast
{
	class decl : public Node
	{
	public:
		decl() = default;
		~decl() = default;

		decl(const decl&) = delete;
		decl& operator= (const  decl&) = delete;
		decl(decl&&) = delete;
		decl& operator= (decl&&) = delete;

		decl(node_kind kind, const Token& name);
	public:
		const Token& name() const;
	private:
		Token m_name;
	};

	class var_decl : public decl
	{
	public:
		var_decl() = delete;
		~var_decl() = default;

		var_decl(const var_decl&) = delete;
		var_decl& operator= (const  var_decl&) = delete;
		var_decl(var_decl&&) = delete;
		var_decl& operator= (var_decl&&) = delete;

		var_decl(const Token& name, Node& init);
	public:
		const Node& initialiser() const;

	private:
		Node* m_init;
	};
}