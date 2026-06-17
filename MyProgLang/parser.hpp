#pragma once

#include "lex.hpp"
#include "builder.hpp"
#include <unordered_map>
#include "sema.hpp"

namespace mpl::ast
{
	class decl;
}


namespace mpl
{
	template <typename F>
	concept list_handler = std::is_invocable_r_v<bool, F>;

	class Parser final
	{
	public:
		using input_t = Lexer::value_type;
	
	public:
		Parser() = delete;
		~Parser() = default;

		Parser(const Parser&) = delete;
		Parser& operator=(const Parser&) = delete;
		Parser(Parser&&) = delete;
		Parser& operator=(Parser&&) = delete;

		explicit Parser(ast::Builder& builder);
	public:
		void operator()(input_t input);
	private:
		bool good();
		void program();
		void declaration();
		void func_decl();
		void param_list();
		void param_decl();
		void compound_statement();
		void statement(bool requireSemi);
		void return_statement();
		void condition();
		void if_statement();
		void while_statement();
		void var_decl();
		void expr();

		void add_expr();
		void mul_expr();

		void postfix_expr();
		void unary_expr();
		void list(list_handler auto handler, tok_kind breakOn);
		void binary_expr(std::size_t precIdx);
		void error(const Token& at, ast::Builder::error_msg msg);
		void primary_expr();
		void paren_expr();
		void literal_expr();
		void id_expr();

	private:
		Lexer m_lexer;
		sema m_sema;
		ast::Builder* m_builder;
		std::unordered_map<input_t, ast::decl*> m_symTab;
	};
}
