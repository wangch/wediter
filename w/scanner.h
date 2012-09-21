/*
  * scanner.h
  */

#ifndef EDITER_SCAN_H_
#define EDITER_SCAN_H_

#include "types.h"
#include <list>

namespace w {

	class Scanner {
	public:
		Scanner(const tstring& txt);
		~Scanner();

		int Scan(const tstring& txt, std::list<Token*>& l);
		int Scan(std::list<Token*>& l);

	private:
		Token* scan();
		void next();
		tstring scanComment();
		tstring scanIdent();
		tstring scanNumber();
		tstring scanString();
		tstring scanRawString();
		tstring scanWhitespace();
		tstring scanVar();
		void scanCtrl(Token* tk);

	private:
		const tstring& txt_;
		int ch_;
		Token* last_tk_;
		int size_;
		int offset_;
		int line_offset_;
	};

}

#endif // EDITER_SCAN_H_
