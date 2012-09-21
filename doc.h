/*
 * doc.h
 */

#ifndef EDITER_DOC_H_
#define EDITER_DOC_H_

#include "w/util.h"
#include <list>

namespace wediter {

	class EditerView;

	class EditerDoc {
	public:
		EditerDoc();
		EditerDoc(const tstring& txt);
		~EditerDoc();

		int Open(const tstring& txt);

		int Save();
		int SaveAs(const tstring& file_name);
		int Close();

		std::list<Line*>* GetLineList() { return &ll_; }
		std::list<tstring>* GetTips() { 
			this->tips_.clear();
			this->tips_.push_back(T("abc"));
			this->tips_.push_back(T("abcd"));
			this->tips_.push_back(T("abcde"));
			this->tips_.push_back(T("abcdef"));
			return &tips_; 
		}
		tstring GetCurrentIndent() { return ident_; }

		int line_max() { return line_max_; }
		Pos pos() { 
			return cpos_; 
		}
		void set_pos(const Pos& p) {
			Pos pos(p);
			if (pos.id > ll_.back()->index) {
				pos.id = ll_.back()->index;
				pos.pos = ll_.back()->txt.length();
			} else if (pos.id < 0) {
				pos.id = 0;
			}

			Line* l = this->pos2ln(pos);
			if (pos.pos >= (int)l->txt.length()) {
				pos.pos = l->txt.length();
			} else if (pos.pos < 0) {
				pos.pos = 0;
			}
			cpos_ = pos;
			this->finding_ = false;
		}

		SelectPos sel_pos() { return sel_pos_; }
		void set_sel_pos(const SelectPos& sp) { sel_pos_ = sp; }
		bool seleting() { return seleting_; }
		void set_seleting(bool sel) { seleting_ = sel; }
		void SelectAll();
		void SelectLine(const Pos& pos);

		bool finding() { return finding_; }
		void set_finding(bool b) { finding_ = b; }
		SelectPos found_pos() {
			return found_pos_; 
		}
		void set_found_pos(const SelectPos& pos) {
			found_pos_ = pos;
		}

		tstring find_text() { return find_txt_; }
		void set_find_text(const tstring& txt) { 
			find_txt_ = txt; 
		}

		void FindAll(const tstring& txt, bool match_case, bool whole_word);
		void FindPrev(const tstring& txt, bool match_case, bool whole_word);
		void FindNext(const tstring& txt, bool match_case, bool whole_word);
		void ReplaceAll(const tstring& txt1, const tstring& txt2, bool match_case, bool whole_word);
		void Replace(const tstring& txt);

		tstring GetText(Pos pos1, Pos pos2);
		Line* GetLine(const Pos& pos) { return pos2ln(pos); }

		tstring DeleteSel();
		tstring Delete();
		tstring DeleteBack();
		void Paste(const tstring& txt);
		void Move();

		int Input(int c);
	private:
		void update_index();
		void del(Pos& pos1, Pos& pos2);
		void paste(const tstring& txt);
		Line* pos2ln(const Pos& pos) {
			return *this->pos2it(pos.id);
		}
		LIT pos2it(int id) {
			LIT it = ll_.begin();
			if (id >= (int)ll_.size()) {
				return --ll_.end();
			}
			std::advance(it, id);
			return it;
		}
		void set_ident(Line* l);
	private:
		tstring fname_;
		tstring saved_; // 		
		LineList ll_; // all lines in the doc
		
		Pos cpos_; // current pos(caret position) in doc

		bool seleting_; // current selecting state
		SelectPos sel_pos_; // current selecting pos

		EditerView* view_; // notify and update? no user

		bool finding_; // current finding state
		int found_index_; // current found index in founds_
		SelectPos found_pos_; // current found pos
		tstring find_txt_; // current find text
		bool case_;
		bool ww_;

		std::list<tstring> tips_; // auto complete tips
		tstring ident_;
		int line_max_;
	};

}

#endif // EDITER_DOC_H_