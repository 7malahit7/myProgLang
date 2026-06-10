#include "lex.hpp"
#include <iostream>

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


int main()
{
	test_lex("42+10");
	return 0;
}