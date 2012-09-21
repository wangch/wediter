/*
 * 
 */

#ifndef EDITER_EVENT_H_
#define EDITER_EVENT_H_

#include "w/singleton.h"
#include "w/util.h"
#include <list>
#include <map>

namespace wediter {

	class EventMgr;
	class EditerDoc;

	class EditEvent {
	public:
		EditEvent(EditerDoc* doc) : doc_(doc) {}
		virtual ~EditEvent() {}
		virtual void Do() = 0;
	protected:
		EditerDoc* doc_;
	};

	class RUEvent : public EditEvent  {
	public:
		RUEvent(EditerDoc* doc) : EditEvent(doc), undo_(false) {}
		virtual ~RUEvent() {}
		virtual void Do() = 0;
		virtual void Redo() = 0;
		virtual void Undo() = 0;
		bool undo() { return undo_; }
		void set_undo(bool un) { undo_ = un; }
	protected:
		bool undo_;
	};

	class Copy : public EditEvent {
	public:
		Copy(EditerDoc* doc);
		virtual ~Copy();
		virtual void Do();
	private:
	};

	class Cut : public RUEvent {
	public:
		Cut(EditerDoc* doc);
		virtual ~Cut();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		tstring txt_;
		SelectPos sel_;
	};

	class Paste : public RUEvent {
	public:
		Paste(EditerDoc* doc, const tchar* txt);
		virtual ~Paste();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		bool selecting_;
		tstring txt_;
		SelectPos sel_;
		SelectPos last_;
	};

	class SelectStart : public EditEvent {
	public:
		SelectStart(EditerDoc* doc, const Pos& pos);
		virtual ~SelectStart() {}
		virtual void Do();
	private:
		Pos pos_;
	};
	
	class SelectEnd : public EditEvent {
	public:
		SelectEnd(EditerDoc* doc, const Pos& to);
		virtual ~SelectEnd() {}
		virtual void Do();
	private:
		Pos pos_;
	};

	class SelectAll : public EditEvent {
	public:
		SelectAll(EditerDoc* doc) : EditEvent(doc) {}
		virtual ~SelectAll() {}
		virtual void Do();
	};

	class SelectLine : public EditEvent {
	public:
		SelectLine(EditerDoc* doc, const Pos& pos) : EditEvent(doc), pos_(pos) {}
		virtual ~SelectLine() {}
		virtual void Do();
	private:
		Pos pos_;
	};

	class FindEvent : public EditEvent {
	public:
		FindEvent(EditerDoc* doc) : EditEvent(doc) {}
		virtual ~FindEvent() {}
		virtual void Do() {}
		virtual void Find(const tstring& str, bool match_case, bool whole_word) {}
	};

	class FindPrevEvent : public FindEvent {
	public:
		FindPrevEvent(EditerDoc* doc) : FindEvent(doc) {}
		virtual ~FindPrevEvent() {}
		virtual void Find(const tstring& str, bool match_case, bool whole_word);
	};

	class FindNextEvent : public FindEvent {
	public:
		FindNextEvent(EditerDoc* doc) : FindEvent(doc) {}
		virtual ~FindNextEvent() {}
		virtual void Find(const tstring& str, bool match_case, bool whole_word);
	};

	class FindAllEvent : public FindEvent {
	public:
		FindAllEvent(EditerDoc* doc) : FindEvent(doc) {}
		virtual ~FindAllEvent() {}
		virtual void Find(const tstring& str, bool match_case, bool whole_word);
	};

	class Replace : public RUEvent {
	public:
		Replace(EditerDoc* doc, 
			const tstring& txt1, 
			const tstring& txt2);
		virtual ~Replace();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		tstring txt1_;
		tstring txt2_;
		SelectPos sel_;
	};

	class ReplaceAll : public RUEvent {
	public:
		ReplaceAll(EditerDoc* doc, 
			const tstring& txt1, 
			const tstring& txt2, 
			bool match_case, 
			bool whole_word);
		virtual ~ReplaceAll();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		tstring txt1_;
		tstring txt2_;
		bool case_;
		bool ww_;
	};

	class Input : public RUEvent {
	public:
		Input(EditerDoc* doc, int c);
		virtual ~Input();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		int ch_;
		Pos pos_;
	};

	class Delete : public RUEvent {
	public:
		Delete(EditerDoc* doc);
		virtual ~Delete();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	protected:
		Pos pos_;
		tstring txt_;
	};

	class DeleteBack : public Delete {
	public:
		DeleteBack(EditerDoc* doc);
		virtual ~DeleteBack();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	};

	class DeleteSel : public RUEvent {
	public:
		DeleteSel(EditerDoc* doc);
		virtual ~DeleteSel();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		SelectPos sel_;
		tstring txt_;
	};

	class Move : public RUEvent {
	public:
		Move(EditerDoc* doc);
		virtual ~Move();
		virtual void Do();
		virtual void Redo();
		virtual void Undo();
	private:
		SelectPos sel_1_;
		SelectPos sel_2_;
	};

	//class Comment : public EditEvent {
	//public:
	//	Comment(EditerDoc* doc);
	//	virtual ~Comment();
	//	virtual int Do();
	//	virtual void Redo();
	//	virtual void Undo();
	//};
	typedef std::list<RUEvent*> EventList;
	typedef std::list<RUEvent*>::iterator ELI;

	class CMDList {
	public:
		CMDList();
		~CMDList();

		void AddView(int id);
		void Do(int id, RUEvent* e);
		void Redo(int id);
		void Undo(int id);

	private:
		std::map<int, EventList*> el_map_;
		std::map<int, ELI> ce_map_; // current event
	};

	typedef w::singleton<CMDList> Commander;
}

#endif // EDITER_EDITEREVENT_H_