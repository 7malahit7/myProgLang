#pragma once
#include "symtab.hpp"

namespace mpl
{
	class sema final
	{
	public:
		sema();
		~sema() = default;
		
		sema(const sema&) = delete;
		sema& operator=(const sema&) = delete;
		sema(sema&&) = delete;
		sema& operator=(sema&&) = delete;
	public:
		void enter_new_scope();
		void exit_scope();
		void record(ast::decl& decl);

		ast::decl* lookup(symtab::name_type name);
		ast::decl* scoped_lookup(symtab::name_type name);
	private:
		symtab m_symtab;
		symtab::scope* m_global{};
		symtab::scope* m_cur{};

	};
}