#include "decl.hpp"

namespace mpl::ast
{
	decl::decl(node_kind kind, const Token& name, types::type t)
		: Node(kind), typed(t), m_name(name)
	{
	}
	const Token& decl::name() const
	{
		return m_name;
	}

	
	var_decl::var_decl(const Token& name, Node& init, types::type t)
		: decl(Node::VarDecl, name, t), m_init(&init)
	{
		validate(&init);
	}

	const Node& var_decl::initialiser() const
	{
		return *m_init;
	}

	param_decl::param_decl(const Token& name, types::type t)
		: decl(Node::ParamDecl, name, t)
	{
	}

	func_decl::func_decl(const Token& name)
		: decl(Node::FunctionDecl, name, types::type::Function)
	{
	}

	void func_decl::specify_params(const list& params)
	{
		m_params = &params;
		validate(&params);
	}

	void func_decl::complete(const list& params, const list& body)
	{
		specify_params(params);
		m_body = &body;
		validate(&body);
	}

	const list& func_decl::params() const
	{
		return *m_params;
	}

	const list& func_decl::body() const
	{
		return *m_body;
	}

	types::type func_decl::return_type() const noexcept
	{
		return m_retType;
	}

	void func_decl::specify_ret(types::type t) noexcept
	{
		m_retType = t;
	}
}
