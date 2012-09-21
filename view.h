/*
 * view.h
 */

#ifndef EDITER_VIEW_H_
#define EDITER_VIEW_H_

#include "w/util.h"
#include <Windows.h>
#include <GdiPlus.h>

namespace wediter {

	enum Cursors { 
		CursorInvalid, 
		CursorText, 
		CursorArrow, 
		CursorUp, 
		CursorWait, 
		CursorHoriz, 
		CursorVert, 
		CursorReverseArrow, 
		CursorHand 
	};

	struct Config {
		Gdiplus::Font* font;
		Gdiplus::Color bkgrd_color;
		Gdiplus::Color txt_color;
		Gdiplus::Color comment_color;
		Gdiplus::Color func_color;
		Gdiplus::Color var_color;
		Gdiplus::Color ctrl_color;
		Gdiplus::Color str_color;
		Gdiplus::Color num_color;
		Gdiplus::Color kw_color;
		Gdiplus::Color select_color;
		int tabstop;
	};

	class CtrlWindow {
	public:
		CtrlWindow() {}
		virtual ~CtrlWindow() {}
		virtual void Notify(int id) {}
	};

	class Button {
	public:
		Button();
		~Button();
		int Init(int id, CtrlWindow* cw, HWND pw, HINSTANCE hinst, Gdiplus::Font* f, bool checkbox = false, int idc = 0);
		void SetText(const tstring& txt, Gdiplus::Color color);
		void SetBkColor(Gdiplus::Color color);
		void Show(int x, int y, int w, int h);
		void Show(int x, int y);
		void SetRgn(HRGN rgn);
		bool Check() { return checked_; }

		LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam);
	private:
		void draw(HDC dc);
	private:
		HWND hw_;
		int id_;
		tstring txt_;
		Gdiplus::Color txt_color_;
		Gdiplus::Color bk_color_;
		int mod_;
		WNDPROC btn_proc_;
		Gdiplus::Font* font_;
		bool checkbox_;
		bool checked_;
		bool moving_;
		bool btn_down_;
		CtrlWindow* cw_;
	};

	class EditerDoc;
	class EditerView;

	class FindBox : public CtrlWindow {
	public:
		FindBox(EditerView* view);
		virtual ~FindBox();
		int Init(HWND pw, HINSTANCE hinst, HFONT hf);
		LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK EdtProc(UINT message, WPARAM wParam, LPARAM lParam);
		void Show(int x, int y, int w, int h);
		void Hide();
		virtual void Notify(int id);
	private:
		void draw(HDC dc);
	private:
		HWND hw_;
		HWND edit_;
		Button btns_[5];

		WNDPROC findbox_proc_;
		WNDPROC edit_proc_;

		EditerView* view_;
	};


	class RepBox : public CtrlWindow {
	public:
		RepBox(EditerView* view);
		virtual ~RepBox();
		int Init(HWND pw, HINSTANCE hinst, HFONT hf);
		LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam);
		//LRESULT CALLBACK FindEdtProc(UINT message, WPARAM wParam, LPARAM lParam);
		void Show(int x, int y, int w, int h);
		void Hide();
		virtual void Notify(int id);
	private:
		void draw(HDC dc);
	private:
		HWND hw_;
		HWND find_edit_;
		HWND rep_edit_;
		Button btns_[6];

		WNDPROC repbox_proc_;
		//WNDPROC edit_proc_;

		EditerView* view_;
	};
	
	class EditerView : public CtrlWindow {
	public:
		EditerView(EditerDoc* doc, int id);
		~EditerView();

		int Init(HINSTANCE hinst, HWND pw);
		int Update();
		HWND window() { return hw_; }
		LRESULT CALLBACK WndProc(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK TipboxProc(int msg, int wp, int lp);
		LRESULT CALLBACK GotoboxProc(int msg, int wp, int lp);

	private:
		friend class FindBox;
		friend class RepBox;

		void find_prev();
		void find_next();
		void find_all();
		void replace_find();
		void replace_all();

		void reDraw();
		void draw(HDC dc);
		void draw(Gdiplus::Graphics* g, int w, int h);
		void drawLine(Gdiplus::Graphics* g, Line* l);
		void drawSel(Gdiplus::Graphics* g, const Line* l);
		void drawFind(Gdiplus::Graphics* g, Line* l);
		void drawLnu(Gdiplus::Graphics* g, Line* l, Gdiplus::PointF& pt);

		void showTips();
		void input(int c, int b);
		void keyDown(int k, int b);

		void lbClickDown(int x, int y);
		void lbdClickDown(int x, int y);
		void lbClickUp(int x, int y);

		void mmove(int x, int y);
		void lbdMmove(int x, int y);
		
		void scrollV(int sb, int pos);
		void scrollH(int sb, int pos);
		void showScroolH(int h);
		void showScroolV(int h);

		void updatePos(int w, int h);
		void updatePos();

		void ctrlKey(int k);
		void copy();
		void paste();
		void cut();
		void find();
		void replace();
		void goto_ln();

		int createCaret();
		void setSize();
		void setCursor(Cursors curs);
		Gdiplus::Size getTxtSize(tstring txt);
		Pos pt2Pos(const Point& pt);
		Point pos2Pt(const Pos& pos);
		bool ptInSelRgn(const Point& pt);

		void tipboxProc(int wp, int lp);

	private:
		HWND pw_;
		HWND hw_;
		int id_;
		EditerDoc* doc_;

		HWND goto_box_;
		HWND goto_box_edit_;
		HFONT gotobox_font_;
		WNDPROC goto_proc_;
		
		HWND tipbox_;
		HFONT tipbox_font_;
		WNDPROC tipbox_proc_;

		FindBox* fb_;
		RepBox* rb_;
		bool fb_show_;
		bool rb_show_;
		bool finding_;
		bool replacing_;
		
		Gdiplus::GraphicsPath* find_gp_;

		int scroll_v_;
		int scroll_h_;
		int line_height_;
		int char_width_;
		
		Config cfg_;
		HBITMAP caret_bmp_;

		bool moving_;
		int max_line_w_;
		int lnu_width_;

		tstring find_txt_;
		tstring rep_txt_;
		bool match_case_;
		bool match_whole_word_;
	};
}

#endif // EDITER_VIEW_H_