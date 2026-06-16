#include "sema.hpp"
#include "decl.hpp"
#include "expr.hpp"

namespace
{
	class type_extractor : public mpl::ast::Visitor<type_extractor>
	{
	public:
		using base = mpl::ast::Visitor<type_extractor>;

	public:
		auto operator()(const mpl::ast::Node& node)
		{
			base::operator()(&node);
			return m_type;
		}

		bool preview(const mpl::ast::typed& typed)
		{
			m_type = typed.type();
			return false;
		}

	private:
		mpl::types::type m_type{ mpl::types::type::Error };
	};

	constexpr bool is_logical(mpl::ast::operation op) noexcept
	{
		return op == mpl::ast::operation::LogOr || op == mpl::ast::operation::LogAnd;
	}

	constexpr bool is_relational(mpl::ast::operation op) noexcept
	{
		using enum mpl::ast::operation;
		switch (op)
		{
		case Equal:
		case NEqual:
		case Less:
		case LessEq:
		case Greater:
		case GreaterEq:
			return true;
		}
		return false;
	}
}

namespace mpl
{
	bool sema::check_type_compatibility(types::type t1, types::type t2) noexcept
	{
		return types::calc_ct(t1, t2) != types::type::Error;
	}

	types::type sema::extract_type(const ast::Node& node) noexcept
	{
		return type_extractor{}(node);
	}

	types::type sema::literal_type(const Token& lit) noexcept
	{
		using enum types::type;
		switch (lit.what())
		{
		case Token::IntNumber:
			return Int;

		case Token::FloatNumber:
			return Float;

		case Token::KwTrue:
		case Token::KwFalse:
			return Bool;

		default:
			return Error;
		}
	}

	types::type sema::keyword_type(const Token& lit) noexcept
	{
		using enum types::type;
		switch (lit.what())
		{
		case Token::KwInt:   return Int;
		case Token::KwFloat: return Float;
		case Token::KwBool:  return Bool;
		default:             return Error;
		}
	}

	unary_cast sema::unary_type(const ast::Node& operand, ast::operation op) noexcept
	{
		const auto operandType = extract_type(operand);
		if (op == ast::operation::UnaryNot)
		{
			return {
				.m_exprType = types::type::Bool,
				.m_opType = operandType,
				.m_opToType = types::type::Bool
			};
		}

		const auto resType = types::calc_ct(operandType, operandType);
		return {
			.m_exprType = resType,
			.m_opType = operandType,
			.m_opToType = resType
		};
	}

	binary_cast sema::binary_type(const ast::Node& lhs, const ast::Node& rhs, ast::operation op) noexcept
	{
		const auto lt = extract_type(lhs);
		const auto rt = extract_type(rhs);
		const auto common = is_logical(op) ? types::type::Bool : types::calc_ct(lt, rt);
		const auto exprType = is_logical(op) || is_relational(op) ? types::type::Bool : common;

		return {
			.m_exprType = exprType,
			.m_lhsType = lt,
			.m_lhsToType = common,
			.m_rhsType = rt,
			.m_rhsToType = common
		};
	}

	const ast::func_decl* sema::extract_function(const ast::Node& node)
	{
		if (node.what() != ast::Node::IdExpr)
			return {};

		auto&& id = static_cast<const ast::id_expr&>(node);
		auto&& decl = id.declaration();
		if (decl.what() != ast::Node::FunctionDecl)
			return {};

		return static_cast<const ast::func_decl*>(&decl);
	}

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

	void sema::begin_function(ast::func_decl& fd)
	{
		m_curFn = &fd;
	}

	void sema::end_function()
	{
		if (m_curFn && m_curFn->return_type() == types::type::Error)
		{
			m_curFn->specify_ret(types::type::Void);
		}
		m_curFn = {};
	}

	bool sema::handle_return(const ast::Node* retExpr)
	{
		if (!m_curFn)
			return false;

		const auto retType = retExpr ? extract_type(*retExpr) : types::type::Void;
		if (m_curFn->return_type() == types::type::Error)
		{
			m_curFn->specify_ret(retType);
			return true;
		}

		return check_type_compatibility(m_curFn->return_type(), retType);
	}
}
