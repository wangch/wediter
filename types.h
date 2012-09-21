/*
  * types.h
  */
	
#ifndef EDITER_TYPES_H_
#define EDITER_TYPES_H_

#include "w/types.h"
#include <list>
#include <vector>


namespace wediter {

	using namespace w;

	struct Point {
		int x;
		int y;
	};

	struct Rect {
		int x, y, w, h;
	};

	struct Font {
		tstring name;
		int size;
		bool italic;
		bool underline;
		bool bold;
	};

	//struct Color {
	//	Color() {}
	//	Color(byte a, byte r, byte g, byte b) : a(a), r(r), g(g), b(b) { }
	//	Color(byte r, byte g, byte b) : a(0), r(r), g(g), b(b) { }
	//	byte a, r, g, b;
	//};


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
		int len() {
			int len = txt.length();
			//if (txt[len-1] == T('\n')) {
			//	--len;
			//}
			return len;
		}
		//std::list<Token*> tkl;
		std::list<int> founds;
	};
	
} // namespace wediter

#endif // 


