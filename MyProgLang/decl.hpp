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

	class param_decl : public decl
	{
	public:
		param_decl() = delete;
		~param_decl() = default;

		param_decl(const param_decl&) = delete;
		param_decl& operator= (const  param_decl&) = delete;
		param_decl(param_decl&&) = delete;
		param_decl& operator= (param_decl&&) = delete;

		param_decl(const Token& name);
	};

	class func_decl : public decl
	{
	public:
		func_decl() = delete;
		~func_decl() = default;

		func_decl(const func_decl&) = delete;
		func_decl& operator= (const  func_decl&) = delete;
		func_decl(func_decl&&) = delete;
		func_decl& operator= (func_decl&&) = delete;

		func_decl(const Token& name);
	public:
		void complete(const list& params, const list& body);
		const list& params() const;
		const list& body() const;
	private:
		const list* m_params{};
		const list* m_body{};
	};
}
