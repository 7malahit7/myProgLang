#include "decl.hpp"

namespace mpl::ast
{
	decl::decl(node_kind kind, const Token& name)
		: Node(kind), m_name(name)
	{
	}
	const Token& decl::name() const
	{
		return m_name;
	}

	
	var_decl::var_decl(const Token& name, Node& init)
		: decl(Node::VarDecl, name), m_init(&init)
	{
		validate(&init);
	}

	const Node& var_decl::initialiser() const
	{
		return *m_init;
	}

	param_decl::param_decl(const Token& name)
		: decl(Node::ParamDecl, name)
	{
	}

	func_decl::func_decl(const Token& name)
		: decl(Node::FunctionDecl, name)
	{
	}

	void func_decl::complete(const list& params, const list& body)
	{
		m_params = &params;
		m_body = &body;
		validate(&params);
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
}
