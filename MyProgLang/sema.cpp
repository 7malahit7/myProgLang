#include "sema.hpp"

namespace mpl
{
	sema::sema()
	{
		m_global = &m_symtab.make_scope(nullptr);
	}
	void sema::enter_new_scope()
	{
		if (!m_cur)
		{
			m_cur = m_global;
			return;
		}
		m_cur = &m_symtab.make_scope(m_cur);
	}

	void sema::exit_scope()
	{
		if (!m_cur)
			return;
		m_cur = m_cur->m_parent;
	}

	void sema::record(ast::decl& decl)
	{
		if (!m_cur)
			return;

		m_symtab.record(decl, *m_cur);
	}

	ast::decl* sema::lookup(symtab::name_type name)
	{
		if (!m_cur)
			m_cur = m_global;
		return m_symtab.lookup(name, *m_cur, lookup_mode::Unscoped);
	}
	ast::decl* sema::scoped_lookup(symtab::name_type name)
	{
		if (!m_cur)
			m_cur = m_global;
		return m_symtab.lookup(name, *m_cur, lookup_mode::Scoped);
	}
}