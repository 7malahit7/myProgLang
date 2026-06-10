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
	}

	const Node& var_decl::initialiser() const
	{
		return *m_init;
	}

}