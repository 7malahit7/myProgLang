#include "parser.hpp"
#include "visitor.hpp"

#include <iostream>

class AstPrinter
	: public mpl::ast::Visitor<AstPrinter>	
{
public:
	using base = mpl::ast::Visitor<AstPrinter>;
	using child_count = std::size_t;
	using child_tracker = std::vector<child_count>;
	using size_type = child_tracker::size_type;
public:
	AstPrinter() = default;
	~AstPrinter() = default;

	AstPrinter(const AstPrinter&) = delete;
	AstPrinter& operator= (const  AstPrinter&) = delete;
	AstPrinter(AstPrinter&&) = delete;
	AstPrinter& operator= (AstPrinter&&) = delete;

	void operator()(const mpl::ast::Node* node)
	{
		m_indetations.clear();
		base::operator()(node);
	}
public:
	bool preview(const mpl::ast::lit_expr& expr)
	{
		indent();
		std::cout << "literal '" << expr.value().value() << "'\n";
		return true;
	}
	bool preview(const mpl::ast::paren_expr& expr)
	{
		indent();
		std::cout << "paren\n";
		m_indetations.push_back(1u);
		return true;
	}
	bool preview(const mpl::ast::unary_expr& expr)
	{
		indent();
		std::cout << "unary '";
		switch (expr.op())
		{
		case mpl::ast::operation::UnaryPos: std::cout << '+'; break;
		case mpl::ast::operation::UnaryNeg: std::cout << '-'; break;
		default: std::cout << "unknown"; break;
		}

		std::cout << "'\n";
		m_indetations.push_back(1u);
		return true;
	}
	bool preview(const mpl::ast::binary_expr& expr)
	{
		indent();
		std::cout << "binary '";
		switch (expr.op())
		{
		case mpl::ast::operation::Addition: std::cout << '+'; break;
		case mpl::ast::operation::Subtraction: std::cout << '-'; break;
		case mpl::ast::operation::Multiplication: std::cout << '*'; break;
		case mpl::ast::operation::Division: std::cout << '/'; break;
		default: std::cout << "unknown"; break;
		}

		std::cout << "'\n";
		m_indetations.push_back(2u);
		return true;
	}
private:
	void indent()
	{
		while (!m_indetations.empty())
		{
			if (m_indetations.back())
				break;
			m_indetations.pop_back();
		}
		if (m_indetations.empty())
			return;
		auto&& last = m_indetations.back();
		--last;
		for (auto&& cur : m_indetations)
		{
			if (&cur != &last)
			{
				print_parent(cur);
				continue;
			}
			print_child(last);
		}
	}

	void print_parent(child_count count)
	{
		using namespace std::literals;
		static constexpr auto child = "| "sv;
		static constexpr auto blank = "  "sv;

		if (!count)
			std::cout << blank;
		else
			std::cout << child;
	}
	void print_child(child_count count)
	{
		using namespace std::literals;
		static constexpr auto prefix = "|-"sv;
		static constexpr auto lastPrefix = "`-"sv;

		if (!count)
			std::cout << lastPrefix;
		else
			std::cout << prefix;
	}

private:
	child_tracker m_indetations;
};


void echo(std::string_view input)
{
	std::cout << "input: " << input << "\n\n";
}

void test_lex(std::string_view input)
{
	echo(input);
	mpl::Lexer lex;
	lex(input);
	while (true)
	{
		auto tok = lex.next();
		switch (tok.what())
		{
		case mpl::Token::Eof:
			std::cout << "eof\n"; return;
		case mpl::Token::Number:
			std::cout << "number"; break;
		case mpl::Token::Plus:
		case mpl::Token::Minus:
		case mpl::Token::Asterisk:
		case mpl::Token::Slash:
			std::cout << "operator"; break;
		case mpl::Token::ParenOpen:
		case mpl::Token::ParenClose:
			std::cout << "paren"; break;
		default:
			std::cout << "error"; break;
		}
		std::cout << ": '" << tok.value() << "'\n";
	}
}

void test_parser(std::string_view input)
{
	echo(input);
	mpl::ast::Builder builder;
	mpl::Parser parser(builder);
	parser(input);
	AstPrinter{}(builder.root());
}

int main()
{
	test_parser("42 * ( 2 + 3 ) + -6");
	return 0;
}