/*
 * doc.cc
 */

#include "doc.h"
#include "w/scanner.h"
#include <sstream>
#include <assert.h>

namespace wediter {

	EditerDoc::EditerDoc() : seleting_(false) {
		Line* l = new Line();
		this->ll_.push_back(l);
		this->sel_pos_.sel_1 = this->sel_pos_.sel_2 = this->cpos_;
		this->found_pos_.sel_1.id = -1;
		line_max_ = 0;
	}

	EditerDoc::EditerDoc(const tstring& txt) : saved_(txt), seleting_(false) {
		w::parse_txt(ll_, txt);
		this->found_pos_.sel_1.id = -1;
		line_max_ = 0;
	}

	EditerDoc::~EditerDoc() {
		for (LIT it = this->ll_.begin(); it != this->ll_.end(); ++it) {
			delete *it;
		}
	}

	int EditerDoc::Save() {
		w::parse_linelist(this->saved_, this->ll_);
		return 0;
	}

	//int EditerDoc::SaveAs(const tstring& file_name) {
	// return 0;
	//}

	int EditerDoc::Close() {
		return 0;
	}

	tstring EditerDoc::GetText(Pos pos1, Pos pos2) {
		if (pos1 == pos2) {
			return tstring();
		}
		if (pos2 < pos1) {
			std::swap(pos1, pos2);
		}
		LIT it1 = this->pos2it(pos1.id);
		LIT it2 = it1;
		std::advance(it2, pos2.id - pos1.id);
		if (it1 == it2) { // same line
			return (*it1)->txt.substr(pos1.pos, pos2.pos - pos1.pos);
		}

		tstring txt((*it1)->txt.substr(pos1.pos));
		txt += T("\r\n");
		for (++it1; it1 != it2; ++it1) {
			txt += (*it1)->txt;
			txt += T("\r\n");
		}
		txt += (*it2)->txt.substr(0, pos2.pos);
		return txt;
	}

	void EditerDoc::Move() {
		if (this->sel_pos_.sel_2 < this->sel_pos_.sel_1) {
			std::swap(this->sel_pos_.sel_2, this->sel_pos_.sel_1);
		}
		if (this->cpos_ <= this->sel_pos_.sel_2 
			&& this->sel_pos_.sel_1 <= this->cpos_) {
			return;
		}

		tstring txt(this->GetText(this->sel_pos_.sel_1, this->sel_pos_.sel_2));
		this->del(this->sel_pos_.sel_1, this->sel_pos_.sel_2);
		this->sel_pos_.sel_1 = this->cpos_;
		this->Paste(txt);
		this->sel_pos_.sel_2 = this->cpos_;
	}

	void EditerDoc::del(Pos& pos1, Pos& pos2) {
		if (pos1 == pos2) {
			return;
		}
		if (pos2 < pos1) {
			std::swap(pos1, pos2);
		}
		LIT it1 = this->pos2it(pos1.id);
		LIT it2 = this->pos2it(pos2.id);
		Line* l = *it1;
		tstring s1(l->txt.substr(0, pos1.pos));
		for (; it1 != it2;) {
			l = *it1;
			delete l;
			it1 = this->ll_.erase(it1++);
		}
		l = *it1;
		tstring s2(l->txt.substr(pos2.pos));
		l->txt = s1 + s2;
		if (pos2 < this->cpos_) {
			if (pos2.id < this->cpos_.id) {
				this->cpos_.id -= pos2.id - pos1.id;
			} else {
				this->cpos_.pos -= pos2.pos - pos1.pos;
			}
		} else if (pos2 == this->cpos_) {
			this->cpos_ = pos1;
		}
		pos2 = pos1;
		this->update_index();
		this->set_ident(l);
	}

	tstring EditerDoc::DeleteSel() {
		tstring txt(this->GetText(this->sel_pos_.sel_1, this->sel_pos_.sel_2));
		this->del(this->sel_pos_.sel_1, this->sel_pos_.sel_2);
		this->cpos_ = this->sel_pos_.sel_1;
		return txt;
	}

	tstring EditerDoc::Delete() {
		Pos pos = this->cpos_;
		Line* l = this->pos2ln(pos);
		if (pos.pos < (int)l->txt.length()) {
			++pos.pos;
		} else {
			++pos.id;
			if (pos.id == this->ll_.size()) {
				--pos.id;
			} else {
				pos.pos = 0;
			}
		}
		tstring txt(this->GetText(this->cpos_, pos));
		this->del(this->cpos_, pos);
		return txt;
	}

	tstring EditerDoc::DeleteBack() {
		Pos pos = this->cpos_;
		if (pos.pos > 0) {
			--pos.pos;
		} else if (pos.id != 0) {
			--pos.id;
			Line* l = this->pos2ln(pos);
			pos.pos = l->txt.length();
		}
		tstring txt(this->GetText(pos, this->cpos_));
		this->del(pos, this->cpos_);
		return txt;
	}

	void EditerDoc::Paste(const tstring& txt) {
		this->paste(txt);
	}

	void EditerDoc::paste(const tstring& txt) {
		LineList ll;
		w::parse_txt(ll, txt);
		if (ll.empty()) {
			return;
		}
		Line* l1 = ll.front();
		Line* l2 = ll.back();
		LIT it = this->pos2it(this->cpos_.id);
		Line* cl = *it;
		tstring s1 = cl->txt.substr(0, this->cpos_.pos);
		tstring s2 = cl->txt.substr(this->cpos_.pos);
		l1->txt = s1 + l1->txt;
		int pos = l2->txt.length();
		l2->txt = l2->txt + s2;
		it = this->ll_.erase(it);
		this->ll_.insert(it, ll.begin(), ll.end());
		this->update_index();
		this->cpos_.id = l2->index;
		this->cpos_.pos = pos;
	}
	
	void EditerDoc::update_index() {
		int n = 0;
		int max = 0;
		for (LIT it = ll_.begin(); it != ll_.end(); ++it, ++n) {
			(*it)->index = n;
			if (max < (int)(*it)->txt.length()) {
				max = (*it)->txt.length();
			}
		}
		this->finding_ = false;
		this->line_max_ = max + 8;
	}

	static bool is_ident(int pos, int c) {
		bool b1 = w::is_letter(c) || c == T('%');
		bool b2 = w::is_letter(c) || w::is_digit(c) || c == T('%');
		return pos == 0 ? b1 : b2;
	}

	void EditerDoc::set_ident(Line* l) {
		int n = this->cpos_.pos - 1;
		if (n < 0 || l->txt.empty()) {
			this->ident_ = T("");
			return;
		}
		int c = l->txt[n];
		if (is_ident(n, c)) {
			int i = n;
			for (; i >= 0; --i) {
				if (!is_ident(i, l->txt[i]) && l->txt[i] != T('"')) {
					break;
				}
			}
			this->ident_ = l->txt.substr(++i, n+1);		
		} else {
			this->ident_ = T("");
		}
	}

	int EditerDoc::Input(int c) {
		LIT it = this->pos2it(this->cpos_.id);
		Line* l = *it;
		tstring s1(l->txt.substr(0, this->cpos_.pos));
		tstring s2(l->txt.substr(this->cpos_.pos));
		if (c == T('\n')) {
			Line* nl = new Line;
			nl->txt = s2;
			l->txt = s1;
			this->ll_.insert(++it, nl);
			++this->cpos_.id;
			this->cpos_.pos = 0;
		} else if (c == T('\r')) {
			return 0;
		}else {
			s1.push_back((tchar)c);
			l->txt = s1 + s2;
			++this->cpos_.pos;
		}

		this->seleting_ = false;
		this->finding_ = false;
		this->update_index();

		this->set_ident(l);
		return 0;
	}

	void EditerDoc::FindPrev(const tstring& txt, bool match_case, bool whole_word) {
		this->find_txt_ = txt;
		this->case_ = match_case;
		this->ww_ = whole_word;
		this->finding_ = true;

		LIT it = this->pos2it(this->cpos_.id);
		std::list<Line*>::reverse_iterator rit(++it);
		for (; rit != this->ll_.rend(); ++rit) {
			Line* l = *rit;
			l->founds.clear();
			w::line_find(l, txt, match_case, whole_word);
			if (l->founds.empty()) {
				continue;
			}
			std::list<int>::reverse_iterator riit = l->founds.rbegin();
			for (; riit != l->founds.rend(); ++riit) {
				Pos p;
				p.id = l->index;
				p.pos = *riit;
				if (this->found_pos_.sel_1 <= p) {
					continue;
				}
				this->found_pos_.sel_1 = p;
				this->found_pos_.sel_2 = p;
				this->found_pos_.sel_2.pos += txt.length();
				this->cpos_ = this->found_pos_.sel_2;
				return;
			}		
		}
	}

	void EditerDoc::FindNext(const tstring& txt, bool match_case, bool whole_word) {
		this->find_txt_ = txt;
		this->case_ = match_case;
		this->ww_ = whole_word;
		this->finding_ = true;

		LIT it = this->pos2it(this->cpos_.id);
		for (; it != this->ll_.end(); ++it) {
			Line* l = *it;
			l->founds.clear();
			w::line_find(l, txt, match_case, whole_word);
			if (l->founds.empty()) {
				continue;
			}
			std::list<int>::iterator iit = l->founds.begin();
			for (; iit != l->founds.end(); ++iit) {
				Pos p;
				p.id = l->index;
				p.pos = *iit;
				if (p <= this->cpos_) {
					continue;
				}
				this->found_pos_.sel_1 = p;
				this->found_pos_.sel_2 = p;
				this->found_pos_.sel_2.pos += txt.length();
				this->cpos_ = this->found_pos_.sel_2;
				return;
			}		
		}
	}

	void EditerDoc::FindAll(const tstring& txt, bool match_case, bool whole_word) {
		if (txt.empty()) {
			return;
		}
		this->find_txt_ = txt;
		this->case_ = match_case;
		this->ww_ = whole_word;

		for (LIT it = this->ll_.begin(); it != this->ll_.end(); ++it) {
			Line* l = *it;
			l->founds.clear();
			w::line_find(l, txt, match_case, whole_word);
		}
	}

	void EditerDoc::ReplaceAll(const tstring& txt1, 
		const tstring& txt2, 
		bool match_case, 
		bool whole_word) {
		for (LIT it = this->ll_.begin(); it != this->ll_.end(); ++it) {
			w::line_replace(*it, txt1, txt2, match_case, whole_word);
		}
	}

	void EditerDoc::Replace(const tstring& txt) {
		this->del(this->found_pos_.sel_1, this->found_pos_.sel_2);
		this->cpos_ = this->found_pos_.sel_1;
		this->paste(txt);
		this->found_pos_.sel_2 = this->cpos_;
	}

	void EditerDoc::SelectAll() {
		SelectPos sp;
		this->sel_pos_.sel_1.id = 0;
		this->sel_pos_.sel_1.pos = 0;
		this->sel_pos_.sel_2.id = this->ll_.back()->index;
		this->sel_pos_.sel_2.pos = this->ll_.back()->txt.length();
		this->cpos_ = this->sel_pos_.sel_2;
		this->seleting_ = true;
	}

	void EditerDoc::SelectLine(const Pos& pos) {
		this->sel_pos_.sel_1 = this->sel_pos_.sel_2 = pos;
		if (pos.id < this->ll_.back()->index) {
			++this->sel_pos_.sel_2.id;
		} else {
			this->sel_pos_.sel_2.pos = this->ll_.back()->txt.length();
		}
		this->cpos_ = this->sel_pos_.sel_2;
		this->seleting_ = true;
	}

} // namespace wediter
