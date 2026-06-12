#pragma once
#include "ast.hpp"
#include "expr.hpp"
#include "stmt.hpp"

namespace mpl::ast
{
	template <typename N, typename Visitor>
	concept visitable = 
		ast_node<N> &&
		requires(Visitor v, const N * node)
		{
			v.visit(*node);
		};
	template <typename N, typename Visitor>
	concept previewable = 
		ast_node<N> &&
		requires(Visitor v, const N * node)
	{
		{ v.preview(*node) } -> std::same_as<bool>;
	};

	template <typename Derived>
	class Visitor
	{
	public:
		using derived_t = Derived;
		using node_ptr = const Node*;
		using node_ref = const Node&;
	public:
		Visitor() = default;
		~Visitor() = default;

		Visitor(const Visitor&) = default;
		Visitor& operator= (const  Visitor&) = default;
		Visitor(Visitor&&) = default;
		Visitor& operator= (Visitor&&) = default;

		void operator()(node_ptr n)
		{
			visit_root(n);
		}
	private:
		template <ast_node N>
		static auto to(node_ptr n)
		{
			return static_cast<const N*>(n);
		}
		derived_t& to_derived()
		{
			return static_cast<derived_t&>(*this);
		}

		void visit(const visitable<derived_t> auto* n)
		{
			to_derived().visit(*n);
		}
		void visit(auto)
		{
		}

		bool preview(const previewable<derived_t> auto* n)
		{
			return to_derived().preview(*n);
		}
		bool preview(auto)
		{
			return true;
		}
		void visit_impl(node_ptr)
		{
		}
		void visit_impl(const lit_expr* e)
		{
			preview(e);
			visit(e);
		}
		void visit_impl(const id_expr* e)
		{
			preview(e);
			visit(e);
		}

		void visit_impl(const paren_expr* e)
		{
			if (preview(e))
			{
				for (auto item : e->children())
				{
					visit_root(item);
				}
			}
			visit(e);
		}
		void visit_impl(const unary_expr* e)
		{
			if (preview(e))
				visit_root(&e->operand());
			visit(e);
		}
		void visit_impl(const binary_expr* e)
		{
			if (preview(e)) 
			{
				visit_root(&e->left());
				visit_root(&e->right());
			}
			visit(e);
		}

		void visit_impl(const var_decl* v)
		{
			if (preview(v))
				visit_root(&v->initialiser());
			visit(v);
		}

		void visit_impl(const param_decl* p)
		{
			preview(p);
			visit(p);
		}

		void visit_impl(const func_decl* f)
		{
			if (preview(f))
			{
				visit_root(&f->params());
				visit_root(&f->body());
			}
			visit(f);
		}

		void visit_impl(const ret_stmt* r)
		{
			if (preview(r))
				visit_root(r->ret_expr());
			visit(r);
		}

		void visit_impl(const if_stmt* i)
		{
			if (preview(i))
			{
				visit_root(&i->condition());
				visit_root(&i->true_branch());
				visit_root(i->false_branch());
			}
			visit(i);
		}

		void visit_impl(const list* l)
		{
			if (preview(l))
			{
				for (auto item : l->children())
				{
					visit_root(item);
				}
			}
			visit(l);
		}

		void visit_impl(const error* e)
		{
			preview(e);
			visit(e);
		}

	private:
		void visit_root(node_ptr n)
		{
			if (!n)
				return;
			using enum node_kind;
			switch (n->what())
			{
			case LiteralExpr:	visit_impl(to<lit_expr>(n));	break;
			case ParenExpr:		visit_impl(to<paren_expr>(n));	break;
			case UnaryExpr:		visit_impl(to<unary_expr>(n));	break;
			case BinaryExpr:	visit_impl(to<binary_expr>(n)); break;
			case IdExpr:		visit_impl(to<id_expr>(n));	break;
			case VarDecl:		visit_impl(to<var_decl>(n)); break;
			case ParamDecl:		visit_impl(to<param_decl>(n)); break;
			case FunctionDecl:	visit_impl(to<func_decl>(n)); break;
			case RetStmt:		visit_impl(to<ret_stmt>(n)); break;
			case IfStmt:		visit_impl(to<if_stmt>(n)); break;
			case Error:			visit_impl(to<error>(n)); break;
			case List:			visit_impl(to<list>(n));	break;

			}
		}
	};
}
