/*
 * util.cc
 */

#include "util.h"


namespace w {

	tstring to_lower(const tstring& str) {
		tstring s(str);
		int len = s.length();
		for (int i = 0; i < len; ++i) {
			s[i] = ::tolower(s[i]);
		}
		return s;
	}

	tstring to_upper(const tstring& str) {
		tstring s(str);
		int len = s.length();
		for (int i = 0; i < len; ++i) {
			s[i] = ::toupper(s[i]);
		}
		return s;
	}

	bool is_prompt(int c) {
		return c == T('%') || c == T('$') || c == T('@');
	}

	bool is_letter(int ch) {
		return T('a') <= ch && ch <= T('z') 
			|| T('A') <= ch && ch <= T('Z') 
			|| ch == T('_') || ch > 0x80 /*unicode*/;
	}

	bool is_digit(int ch) {
		return T('0') <= ch && ch <= T('9');
	}

	int parse_txt(LineList& ll, const tstring& txt) {
		tistrstream is(txt);
		tstring line_txt;
		while (std::getline(is, line_txt)) {
			Line* l = new Line;
			l->txt = line_txt;
			while (!l->txt.empty() && l->txt.back() == T('\r')) {
				l->txt.pop_back();
			}
			ll.push_back(l);
		}
		if (line_txt.empty()) {
			ll.push_back(new Line);
		}
		return 0;
	}

	int parse_linelist(tstring& txt, const LineList& ll) {
		for (LCIT it = ll.begin(); it != ll.end(); ++it) {
			Line* l = *it;
			txt += l->txt;
			txt += T("\r\n");
		}
		return 0;
	}
	
	static void update_line(Line* l, 
		const tstring& s, 
		const tstring& ss, 
		bool match_case, 
		bool whole_word, 
		bool rep) {
		int len1 = l->txt.length();
		int len2 = s.length();
		tstring s1(l->txt);
		tstring s2(s);
		if (!match_case) {
			s1 = to_lower(s1);
			s2 = to_lower(s2);
		}

		int off = 0;
		int off2 = off;
		for (;;) {
			off = s1.find(s2, off2);
			if (off == tstring::npos) {
				break;
			}
			off2 = off;
			off2 += len2;
			if (whole_word) {
				if (off == 0) {
					if (is_letter(s1[off + len2])) {
						continue;
					}
				} else if(off + len2 < len1) {
					if (is_letter(s1[off - 1])) {
						continue;
					}
					if (is_letter(s1[off + len2])) {
						continue;
					}
				}
			}
			if (rep) {
				tstring t1(l->txt.substr(0, off));
				tstring t2(l->txt.substr(off2));
				l->txt = t1 + ss + t2;
			} else {
				l->founds.push_back(off);
			}
		}
	}

	void line_replace(Line* l, 
		const tstring& s, 
		const tstring& ss, 
		bool match_case, 
		bool whole_word) {
		update_line(l, s, ss, match_case, whole_word, true);
	}

	void line_find(Line* l, 
						  const tstring& s, 
						  bool match_case, 
						  bool whole_word) {
		update_line(l, s, T(""), match_case, whole_word, false);
	}
} // namespace w
