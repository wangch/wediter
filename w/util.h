/*
 * util.h
 */

#ifndef W_UTIL_H_
#define W_UTIL_H_

#include "types.h"
#include <list>

namespace w {

	struct Point {
		int x;
		int y;
	};

	struct Pos {
		Pos() : id(0), pos(0) {}
		int id;
		int pos;
		bool operator==(const Pos& p) const {
			return this->id == p.id && pos == p.pos;
		}
		bool operator!=(const Pos& p) const {
			return !(*this == p);
		}
		bool operator<(const Pos& p) const {
			return this->id == p.id ? this->pos < p.pos : this->id < p.id;
		}
		bool operator<=(const Pos& p) const {
			return *this < p || *this == p;
		}
		bool operator>(const Pos& p) const {
			return !(*this <= p);
		}
	};

	struct SelectPos {
		Pos sel_1; // select pos begin
		Pos sel_2; // select pos end
	};

	struct Line {
		Line() : index(0) {}
		tstring txt;
		int index;
		std::list<int> founds;
	};

	typedef std::list<Line*> LineList;
	typedef LineList::iterator LIT;
	typedef LineList::const_iterator LCIT;

	tstring to_lower(const tstring& str);
	tstring to_upper(const tstring& str);
	bool is_prompt(int c);
	bool is_letter(int c);
	bool is_digit(int c);
	int parse_txt(LineList& ll, const tstring& txt);
	int parse_linelist(tstring& txt, const LineList& ll);
	void line_find(Line* l, const tstring& s, bool match_case, bool whole_word);
	void line_replace(Line* l, const tstring& s, const tstring& ss, bool match_case, bool whole_word);
}

using namespace w;

#endif // EDITER_UTIL_H_