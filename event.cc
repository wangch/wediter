/*
 * event.cc
 */

#include "event.h"
#include "doc.h"
#include <assert.h>

namespace wediter {

	void FindAllEvent::Find(const tstring& txt, bool match_case, bool whole_word) {
		this->doc_->FindAll(txt, match_case, whole_word);
	}

	void FindPrevEvent::Find(const tstring& txt, bool match_case, bool whole_word) {
		this->doc_->FindPrev(txt, match_case, whole_word);
	}

	void FindNextEvent::Find(const tstring& txt, bool match_case, bool whole_word) {
		this->doc_->FindNext(txt, match_case, whole_word);
	}

	void SelectAll::Do() {
		this->doc_->SelectAll();
	}

	void SelectLine::Do() {
		this->doc_->SelectLine(this->pos_);
	}

	Replace::Replace(EditerDoc* doc, 
		const tstring& txt1, 
		const tstring& txt2) 
		: RUEvent(doc), txt1_(txt1), txt2_(txt2) {
	}

	Replace::~Replace() {
	}

	void Replace::Do() {
		this->doc_->Replace(this->txt2_);
		this->sel_ = this->doc_->found_pos();
	}

	void Replace::Redo() {
		this->Do();
	}

	void Replace::Undo() {
		this->doc_->set_found_pos(this->sel_);
		this->doc_->Replace(this->txt1_);
	}

	ReplaceAll::ReplaceAll(EditerDoc* doc, 
		const tstring& txt1, 
		const tstring& txt2, 
		bool match_case, 
		bool whole_word) 
		: RUEvent(doc), txt1_(txt1), txt2_(txt2), case_(match_case), ww_(whole_word) {
	}

	ReplaceAll::~ReplaceAll() {
	}

	void ReplaceAll::Do() {
		this->doc_->ReplaceAll(this->txt1_, this->txt2_, this->case_, this->ww_);
	}

	void ReplaceAll::Redo() {
		this->Do();
	}

	void ReplaceAll::Undo() {
		this->doc_->ReplaceAll(this->txt2_, this->txt1_, this->case_, this->ww_);
	}

	SelectStart::SelectStart(EditerDoc* doc, const Pos& pos) 
		: EditEvent(doc), pos_(pos) {
	}

	void SelectStart::Do() {
		this->doc_->set_pos(this->pos_);
		SelectPos sp;
		sp.sel_1 = sp.sel_2 = this->pos_;
		this->doc_->set_sel_pos(sp);
		this->doc_->set_seleting(false);
	}

	SelectEnd::SelectEnd(EditerDoc* doc, const Pos& to) 
		: EditEvent(doc), pos_(to) {
	}

	void SelectEnd::Do() {
		SelectPos sp = this->doc_->sel_pos();
		sp.sel_2 = this->pos_;
		this->doc_->set_sel_pos(sp);
		this->doc_->set_seleting(true);
	}
	
	Paste::Paste(EditerDoc* doc, const tchar* txt) 
		: RUEvent(doc), selecting_(false), txt_(txt) {
	}

	Paste::~Paste() {
	}

	void Paste::Do() {
		this->selecting_ = this->doc_->seleting();
		if (this->selecting_) {
			this->last_ = this->doc_->sel_pos();
			if (this->last_.sel_2 < this->last_.sel_1) {
				std::swap(this->sel_.sel_1, this->sel_.sel_2);
			}
			this->sel_.sel_1 = this->last_.sel_1;
		} else {
			this->sel_.sel_1 = this->doc_->pos();
		}
		this->doc_->Paste(this->txt_);
		this->sel_.sel_2 = this->doc_->pos();
		this->doc_->set_sel_pos(this->sel_);
		//this->doc_->set_seleting(true);
	}

	void Paste::Redo() {
		this->doc_->set_seleting(this->selecting_);
		this->doc_->set_sel_pos(this->last_);
		this->doc_->set_pos(this->sel_.sel_1);
		this->Do();
	}

	void Paste::Undo() {
		this->doc_->set_pos(this->sel_.sel_1);
		this->doc_->set_sel_pos(this->sel_);
		this->doc_->DeleteSel();
	}

	Delete::Delete(EditerDoc* doc) : RUEvent(doc){
	}

	Delete::~Delete() {
	}

	void Delete::Do() {
		this->txt_ = this->doc_->Delete();
		this->pos_ = this->doc_->pos();
	}

	void Delete::Redo() {
		this->doc_->set_pos(this->pos_);
		this->doc_->Delete();
	}

	void Delete::Undo() {
		this->doc_->set_pos(this->pos_);
		this->doc_->Paste(this->txt_);
	}

	DeleteBack::DeleteBack(EditerDoc* doc) 
		: Delete(doc) {
	}

	DeleteBack::~DeleteBack() {
	}

	void DeleteBack::Do() {
		this->txt_ = this->doc_->DeleteBack();
		this->pos_ = this->doc_->pos();
	}

	void DeleteBack::Redo() {
		this->Do();
	}

	void DeleteBack::Undo() {
		this->doc_->set_pos(this->pos_);
		this->doc_->Paste(this->txt_);
	}

	DeleteSel::DeleteSel(EditerDoc* doc) : RUEvent(doc){
	}

	DeleteSel::~DeleteSel() {
	}

	void DeleteSel::Do() {
		this->sel_ = this->doc_->sel_pos();
		this->txt_ = this->doc_->DeleteSel();
	}

	void DeleteSel::Redo() {
		this->doc_->set_sel_pos(this->sel_);
		this->Do();
		this->doc_->set_seleting(true);
	}

	void DeleteSel::Undo() {
		Pos pos = this->sel_.sel_1 < this->sel_.sel_2 ? this->sel_.sel_1 : this->sel_.sel_2;
		this->doc_->set_pos(pos);
		this->doc_->Paste(this->txt_);
		this->doc_->set_sel_pos(this->sel_);
		this->doc_->set_seleting(true);
	}

	Move::Move(EditerDoc* doc) : RUEvent(doc) {
	}

	Move::~Move() {
	}

	void Move::Do() {
		this->sel_1_ = this->doc_->sel_pos();
		this->doc_->Move();
		this->sel_2_ = this->doc_->sel_pos();
		this->doc_->set_seleting(true);
	}

	void Move::Redo() {
		this->doc_->set_pos(this->sel_2_.sel_1);
		this->doc_->set_sel_pos(this->sel_1_);
		this->doc_->Move();
		this->doc_->set_seleting(true);
	}

	void Move::Undo() {
		this->doc_->set_pos(this->sel_1_.sel_1);
		this->doc_->set_sel_pos(this->sel_2_);
		this->doc_->Move();
		this->doc_->set_seleting(true);
	}

	Input::Input(EditerDoc* doc, int c) : RUEvent(doc), ch_(c) {
	}

	Input::~Input() {
	}

	void Input::Do() {
		this->pos_ = this->doc_->pos();
		this->doc_->Input(this->ch_);
	}

	void Input::Redo() {
		this->doc_->set_pos(this->pos_);
		this->Do();
		this->doc_->set_seleting(false);
	}

	void Input::Undo() {
		this->doc_->set_pos(this->pos_);
		this->doc_->Delete();
		this->doc_->set_seleting(false);
	}


	CMDList::CMDList() {
	}

	CMDList::~CMDList() {
	}

	void CMDList::AddView(int id) {
		this->el_map_[id] = new EventList;
		this->ce_map_[id] = this->el_map_[id]->end();
	}

	void CMDList::Do(int id, RUEvent* e) {
		EventList* l = this->el_map_[id];
		ELI it = this->ce_map_[id];
		for (;it != l->end();) {
			if ((*it)->undo()) {
				EditEvent* e = *it;
				delete e;
				it = l->erase(it++);
			} else {
				break;
			}
		}
		l->push_back(e);
		it = l->end();
		this->ce_map_[id] = --it;
		e->Do();
	}

	void CMDList::Redo(int id) {
		EventList* l = this->el_map_[id];
		ELI it = this->ce_map_[id];
		for (;it != l->end(); ++it) {
			if ((*it)->undo()) {
				(*it)->Redo();
				(*it)->set_undo(false);
				this->ce_map_[id] = it;
				break;
			}
		}
	}

	void CMDList::Undo(int id) {
		EventList* l = this->el_map_[id];
		ELI it = this->ce_map_[id];
		for (;;) {
			if (!(*it)->undo()) {
				(*it)->Undo();
				(*it)->set_undo(true);
				this->ce_map_[id] = it;
				break;
			}
			if (it == l->begin()) {
				break;
			} else {
				--it;
			}
		}
	}
}
