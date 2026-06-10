#pragma once

#include "lex.hpp"
#include "builder.hpp"
#include <unordered_map>

namespace mpl::ast
{
	class decl;
}


namespace mpl
{
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
		void var_decl();
		void expr();

		void add_expr();
		void mul_expr();

		void unary_expr();
		void primary_expr();
		void paren_expr();
		void literal_expr();
		void id_expr();

	private:
		Lexer m_lexer;
		ast::Builder* m_builder;
		std::unordered_map<input_t, ast::decl*> m_symTab;
	};
}