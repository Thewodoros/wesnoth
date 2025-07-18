/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "formula/tokenizer.hpp"

#include <locale>
#include <sstream>

namespace wfl
{
namespace tokenizer
{

namespace {

[[noreturn]] void raise_exception(iterator& i1, iterator i2, const std::string& str) {
	std::ostringstream expr;
	while( (i1 != i2) && (*i1 != '\n') ) {
		if( (*i1 != '\t') )
			expr << *i1;
		++i1;
	}

	if( str.empty() )
		throw token_error("Unrecognized token", expr.str() );
	else
		throw token_error(str, expr.str() );
}

}

token get_token(iterator& i1, const iterator i2) {

	iterator it = i1;
	if( *i1 >= 'A' ) {
		//current character is >= 'A', limit search to the upper-half of the ASCII table

		// check if we parse now token_type::identifier or token_type::operator_token/keyword based on string
		if( *i1 <= 'Z' || ( *i1 >= 'a' && *it <= 'z' ) || *i1 == '_' ) {

			while(i1 != i2 && (std::isalpha(*i1, std::locale::classic()) || *i1 == '_'))
				++i1;

			int diff = i1 - it;
			token_type t = token_type::identifier;

			//check if this string matches any keyword or an operator
			//possible operators and keywords:
			// d, or, in, def, and, not, wfl, where, wflend, functions
			if( diff == 1 ) {
				if( *it == 'd' )
					t = token_type::operator_token;
			} else if( diff == 2 ) {
				if( *it == 'o' && *(it+1) == 'r' )
					t = token_type::operator_token;
				else if( *it == 'i' && *(it+1) == 'n' )
					t = token_type::operator_token;
			} else if( diff == 3 ) {
				if( *it == 'd' ) { //def
					if( *(it+1) == 'e' && *(it+2) == 'f' )
						t = token_type::keyword;
				} else if( *it == 'a' ) { //and
					if( *(it+1) == 'n' && *(it+2) == 'd' )
						t = token_type::operator_token;
				} else if( *it == 'n' ) { //not
					if( *(it+1) == 'o' && *(it+2) == 't' )
						t = token_type::operator_token;
				} else if( *it == 'w' ) { //wfl
					if( *(it+1) == 'f' && *(it+2) == 'l' )
						t = token_type::keyword;
				}
			} else if( diff == 5 ) {
				std::string s(it, i1);
				if( s == "where" )
					t = token_type::operator_token;
			} else if( diff == 6 ) {
				std::string s(it, i1);
				if( s == "wflend" )
					t = token_type::keyword;
			} else if( diff == 9 ) {
				std::string s(it, i1);
				if( s == "functions" )
					t = token_type::keyword;
			}

			return token( it, i1, t);
		} else {
			//at this point only 3 chars left to check:
			if( *i1 == '[' )
				return token( it, ++i1, token_type::lsquare );

			if( *i1 == ']' )
				return token( it, ++i1, token_type::rsquare );

			if( *i1 == '^' )
				return token( it, ++i1, token_type::operator_token );

			if( *i1 == '~' )
				return token( it, ++i1, token_type::operator_token );

			//unused characters in this range:
			// \ ` { | }
			// Note: {} should never be used since they play poorly with WML preprocessor
		}
	} else {
		//limit search to the lower-half of the ASCII table
		//start by checking for whitespaces/end of line char
		if( *i1 <= ' ' ) {
			if( *i1 == '\n' ) {
				return token( it, ++i1, token_type::eol);
			} else {

				while( i1 != i2 && *i1 <= ' ' && *i1 != '\n' )
					++i1;

				return token( it, i1, token_type::whitespace );
			}
		//try to further limit number of characters that we need to check:
		} else if ( *i1 >= '0' ){
			//current character is between '0' and '@'
			if( *i1 <= '9' ) {
				//we parse integer or decimal number
				++i1;
				bool dot = false;

				while( i1 != i2 ) {
					if( *i1 >= '0' && *i1 <= '9' ) {
						//do nothing
					} else {
						//look for '.' in case of decimal number
						if( *i1 == '.' ) {
							//allow only one dot in such expression
							if( !dot )
								dot = true;
							else
								raise_exception(it, i2, "Multiple dots near decimal expression");
						} else
							break;
					}
					++i1;
				}

				if( dot )
					return token( it, i1, token_type::decimal );
				else
					return token( it, i1, token_type::integer );

			} else {
				//current character is between ':' and '@'
				//possible tokens at this point that we are interested in:
				// ; < = > <= >=
				//unused characters in this range:
				// : ? @

				if( *i1 == ';' ) {
					return token( it, ++i1, token_type::semicolon);
				} else if( *i1 == '=' ) {
					return token( it, ++i1, token_type::operator_token);
				} else if( *i1 == '<' ) {
					++i1;
					if( i1 != i2 ) {
						if( *i1 == '=' )
							return token( it, ++i1, token_type::operator_token);
						else
							return token( it, i1, token_type::operator_token);
					} else
						return token( it, i1, token_type::operator_token);
				} else if( *i1 == '>' ) {
					++i1;
					if( i1 != i2 ) {
						if( *i1 == '=' )
							return token( it, ++i1, token_type::operator_token);
						else
							return token( it, i1, token_type::operator_token);
					} else
						return token( it, i1, token_type::operator_token);
				}
			}
		//current character is between '!' and '/'
		//possible tokens:
		// , . .+ .- .* ./ .. ( ) ' # + - -> * / % !=
		//unused characters:
		// ! " $ &
		// ! is used only as part of !=
		// Note: " should never be used since it plays poorly with WML
		} else if ( *i1 == ',' ) {
			return token( it, ++i1, token_type::comma);

		} else if ( *i1 == '.' ) {
			++i1;

			if( i1 != i2 ) {
				if( *i1 == '+' || *i1 == '-' || *i1 == '*' || *i1 == '/' || *i1 == '.')
					return token( it, ++i1, token_type::operator_token );
				else
					return token( it, i1, token_type::operator_token );
			} else {
				return token( it, i1, token_type::operator_token);
			}

		} else if ( *i1 == '(' ) {
			return token( it, ++i1, token_type::lparens);

		} else if ( *i1 == ')' ) {
			return token( it, ++i1, token_type::rparens);

		} else if ( *i1 == '\'' ) {
			int bracket_depth = 0;
			++i1;
			while (i1 != i2) {
				if (*i1 == '[') {
					bracket_depth++;
				} else if(bracket_depth > 0 && *i1 == ']') {
					bracket_depth--;
				} else if(bracket_depth == 0 && *i1 == '\'') {
					break;
				}
				++i1;
			}

			if( i1 != i2 ) {
				return token( it, ++i1, token_type::string_literal );
			} else {
				raise_exception(it, i2, "Missing closing ' for formula string");
			}

		} else if ( *i1 == '#' ) {
			++i1;
			while( i1 != i2 && *i1 != '#' )
				++i1;

			if( i1 != i2 ) {
				return token( it, ++i1, token_type::comment );
			} else {
				raise_exception(it, i2, "Missing closing # for formula comment");
			}

		} else if ( *i1 == '+' ) {
			return token( it, ++i1, token_type::operator_token);

		} else if ( *i1 == '-' ) {
			++i1;

			if( i1 != i2 ) {
				if( *i1 == '>' )
					return token( it, ++i1, token_type::pointer );
				else
					return token( it, i1, token_type::operator_token );
			} else {
				return token( it, i1, token_type::operator_token);
			}

		} else if ( *i1 == '*' ) {
			return token( it, ++i1, token_type::operator_token);

		} else if ( *i1 == '/' ) {
			return token( it, ++i1, token_type::operator_token);

		} else if ( *i1 == '%' ) {
			return token( it, ++i1, token_type::operator_token);

		} else if ( *i1 == '!' ) {
			++i1;
			if( *i1 == '=' )
				return token( it, ++i1, token_type::operator_token);
			else
				raise_exception(it, i2, std::string() );
		}
	}
	raise_exception(it, i2, std::string());
}

}

}
