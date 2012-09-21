/*
  * types.h
  */

#ifndef W_TYPES_H_
#define W_TYPES_H_

#include <string>
#include <sstream>

namespace w {

	#ifdef UNICODE
		typedef wchar_t tchar;
		typedef std::wstring tstring;
		typedef std::wistringstream tistrstream;
	#define T(X) L ## X
	#else 
		typedef char tchar;
		typedef std::string tstring;
		typedef std::istringstream tistrstream;
	#define T(X) X
	#endif

	typedef unsigned char byte;
	typedef unsigned int uint;
	typedef unsigned short ushort;

	#ifdef WINDOWS
		typedef __int64 int64;
		typedef unsigned __int64 uint64;
	#else
		typedef long long int64;
		typedef unsigned long long uint64;
	#endif

	enum TokenType {
		EofType,
		CommentType, 	// ## 
		IdentType, 		// _abc abc abc123 abc_123 abc_
		KewordType, 	// select, from, 
		FuncType, 		// built in 
		OperatorType, 	// + - * / . -> () [] {} 
		NumberType, 	// 123 0.123 "123" `123`   
		StringType,
		VariableType,	// @abc or $abc
		CtrlType,		// %did_data, %if
		WhitespaceType
	};

	struct Token {
		tstring txt;
		int pos;
		//int line;
		TokenType type;
	};
}

#endif // W_TYPES_H_

