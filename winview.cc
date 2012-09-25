/*
 * winview.cc
 */

#include "stdafx.h"
#include "view.h"
#include "doc.h"
#include "w/scanner.h"
#include "event.h"
#include "Resource.h"

#include <math.h>

namespace wediter {
	
	static tchar* cls_name = T("wedit");
	static void show_last_error_box() {
		wchar_t* err;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, ::GetLastError(), 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&err, 0, NULL);
		::MessageBox(NULL, err, L"Error", 0);
	}

	/// class EditerView
	EditerView::EditerView(EditerDoc* doc, int id) 
		: doc_(doc), id_(id), scroll_v_(0), caret_bmp_(NULL),
		scroll_h_(0), max_line_w_(0), moving_(false), fb_show_(false), rb_show_(false),
		finding_(false), replacing_(false), match_case_(false), match_whole_word_(false), show_ln_(true) {
	}

	EditerView::~EditerView() {
		delete this->fb_;
		delete this->rb_;
		::DeleteObject(this->tipbox_font_);
		::DeleteObject(this->gotobox_font_);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		EditerView* pv = (EditerView*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pv == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return pv->WndProc(message, wParam, lParam);
	}


	static ATOM regwnd(HINSTANCE hinst, HWND pw) {
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= 0/*CS_HREDRAW | CS_VREDRAW*/;
		wcex.lpfnWndProc	= WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hinst;
		wcex.hIcon			= ::LoadIcon(hinst, MAKEINTRESOURCE(IDI_WEDITER));
		wcex.hCursor		= ::LoadCursor(NULL, IDC_IBEAM);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WEDITER);
		wcex.lpszClassName	= cls_name;
		wcex.hIconSm		= ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		return ::RegisterClassEx(&wcex);
	}

	static LRESULT CALLBACK _goto_boxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		EditerView* pv = (EditerView*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pv == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return pv->GotoboxProc(message, wParam, lParam);
	}

	static LRESULT CALLBACK _tipboxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		EditerView* pv = (EditerView*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (pv == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return pv->TipboxProc(message, wParam, lParam);
	}

	int EditerView::Init(HINSTANCE hinst, HWND pw) {
		this->cfg_.bkgrd_color	= Gdiplus::Color(39, 40, 34);
		this->cfg_.select_color	= Gdiplus::Color(173, 214, 255);
		this->cfg_.txt_color		= Gdiplus::Color(248, 248, 242);
		this->cfg_.kw_color		= Gdiplus::Color(249, 38, 76);
		this->cfg_.str_color		= Gdiplus::Color(230, 219, 90);
		this->cfg_.num_color		= Gdiplus::Color(174, 115, 154);
		this->cfg_.func_color		= Gdiplus::Color(102, 217, 239);
		this->cfg_.comment_color	= Gdiplus::Color(117, 113, 94);
		this->cfg_.var_color		= Gdiplus::Color(185, 217, 239);
		this->cfg_.ctrl_color		= Gdiplus::Color(166, 169, 38);
		//cfg_.ctrl_font = new Gdiplus::Color(249, 38, 114);
		this->cfg_.tabstop = 4;
		Gdiplus::FontFamily ff(T("Consolas"));
		this->cfg_.font = new Gdiplus::Font(&ff, 16, 
			Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

		regwnd(hinst, pw);

		HWND hw = ::CreateWindowEx(WS_EX_CLIENTEDGE, T("static"), T(""),
			WS_CHILD | WS_VISIBLE,
			80, 40, 800, 400,
			pw, (HMENU)IDC_WEDITER_P, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->pw_ = hw;

		hw = ::CreateWindowEx(0, 
			cls_name, 
			T(""),
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL,
			0, 0, 800, 400,
			this->pw_,
			NULL, 
			hinst,
			NULL);

		if (!hw) {
			return -1;
		}

		this->hw_ = hw;
		::SetWindowLongPtr(this->hw_, GWLP_USERDATA, (LONG)this);
		::ShowWindow(this->hw_, SW_SHOW);
		::UpdateWindow(hw);

		// gotobox
		/*
		hw = ::CreateWindowEx(WS_EX_CLIENTEDGE, T("static"), T(""),
			WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0,
			this->hw_, (HMENU)IDC_GOTOBOX, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->goto_box_ = hw;
		*/

		hw = ::CreateWindowEx(0, T("edit"), T(""),
			WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
			0, 0, 0, 0,
			this->hw_, (HMENU)IDC_GOTOBOX_EDIT, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->goto_box_edit_ = hw;
		::SetWindowLongPtr(this->goto_box_edit_, GWLP_USERDATA, (LONG)this);
		this->goto_proc_ = (WNDPROC)::SetWindowLongPtr(this->goto_box_edit_, GWLP_WNDPROC, (LONG)_goto_boxProc);

		// tipbox
		hw = ::CreateWindowEx(0, T("listbox"), T(""),
			WS_CHILD | WS_VISIBLE | LBS_STANDARD,
			0, 0, 0, 0,
			this->hw_, // 
			(HMENU)IDC_TIPBOX, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->tipbox_ = hw;
		::SetWindowLongPtr(this->tipbox_, GWLP_USERDATA, (LONG)this);
		this->tipbox_proc_ = (WNDPROC)::SetWindowLongPtr(this->tipbox_, GWLP_WNDPROC, (LONG)_tipboxProc);

		LOGFONT logfont;
		::memset(&logfont, 0, sizeof(logfont));
		logfont.lfHeight = -14;
		wcscpy_s(logfont.lfFaceName, L"Consolas");

		this->tipbox_font_ = ::CreateFontIndirect(&logfont);
		this->fb_ = new FindBox(this);
		this->fb_->Init(this->pw_, hinst, this->tipbox_font_);
		this->rb_ = new RepBox(this);
		this->rb_->Init(this->pw_, hinst, this->tipbox_font_);
		
		::SendMessage(this->tipbox_, WM_SETFONT, WPARAM(this->tipbox_font_),
			MAKELPARAM(TRUE, 0));

		logfont.lfHeight = -20;
		logfont.lfWeight = 700;
		this->gotobox_font_ = ::CreateFontIndirect(&logfont);
		::SendMessage(this->goto_box_edit_, WM_SETFONT, WPARAM(this->gotobox_font_),
			MAKELPARAM(TRUE, 0));

		this->setSize();
		this->createCaret();

		return 0;
	}

	void EditerView::setCursor(Cursors curs) {
		switch (curs) {
		case CursorText:
			::SetCursor(::LoadCursor(NULL,IDC_IBEAM));
			break;
		case CursorUp:
			::SetCursor(::LoadCursor(NULL,IDC_UPARROW));
			break;
		case CursorWait:
			::SetCursor(::LoadCursor(NULL,IDC_WAIT));
			break;
		case CursorHoriz:
			::SetCursor(::LoadCursor(NULL,IDC_SIZEWE));
			break;
		case CursorVert:
			::SetCursor(::LoadCursor(NULL,IDC_SIZENS));
			break;
		case CursorHand:
			::SetCursor(::LoadCursor(NULL,IDC_HAND));
			break;
		case CursorReverseArrow:
			::SetCursor(::LoadCursor(NULL,IDC_ARROW));
			break;
		case CursorArrow:
			::SetCursor(::LoadCursor(NULL,IDC_ARROW));
			break;
		case CursorInvalid:
			::SetCursor(::LoadCursor(NULL,IDC_NO));
			break;
		}
	}

	int EditerView::Update() {
		return 0;
	}

	Gdiplus::Size EditerView::getTxtSize(tstring txt) {
		Gdiplus::GraphicsPath path;
		Gdiplus::FontFamily ff;
		Gdiplus::Font* font = this->cfg_.font;
		font->GetFamily(&ff);

		Gdiplus::Rect rc;
		Gdiplus::Size sz;
		int len = txt.length();
		if (len == 0) {
			return sz;
		}

		bool is_space = false;
		if (txt[len - 1] == T(' ')) {
			txt[len - 1] = T('M');
			is_space = true;
		}

		path.AddString(txt.c_str(), 
			len, &ff, 
			font->GetStyle(), 
			font->GetSize(), 
			Gdiplus::Point(0, 0), 
			NULL);
		path.GetBounds(&rc);

		int sd = (int)(font->GetSize() / 4);
		sz.Width = rc.Width + sd;
		if (is_space) {
			sz.Width -= sd;
		}
		sz.Height = rc.Height;
		return sz;
	}

	Pos EditerView::pt2Pos(const Point& pt) {
		int h = this->scroll_v_ + pt.y;
		int cnt = h / this->line_height_;
		Pos pos;
		pos.id = cnt;
		Line* l = this->doc_->GetLine(pos);
		pos.id = l->index;
	
		tstring& txt(l->txt);
		int len =txt.length();
		int w = (this->scroll_h_ + pt.x - this->lnu_width_) / this->char_width_;
		for (int i = 0; i < len && i < w; ++i) {
			if (txt[i] > 0x80) {
				--w;
			}
			if (txt[i] == T('\t')) {
				w -= this->cfg_.tabstop - 1;
			}
		}

		if (w < 0) {
			w = 0;
		}
		pos.pos = w < len ? w : len;
		return pos;	
	}

	Point EditerView::pos2Pt(const Pos& pos) {
		Point pt;
		pt.y = pos.id * this->line_height_ - this->scroll_v_;
		Line* l = this->doc_->GetLine(pos);

		int w = pos.pos;
		for (int i = 0; i < pos.pos; ++i) {
			if (l->txt[i] > 0x80) {
				++w;
			}
			if (l->txt[i] == T('\t')) {
				w += this->cfg_.tabstop - 1;
			}
		}

		int x = w * this->char_width_ + this->lnu_width_;
		pt.x = x - this->scroll_h_ + (int)this->cfg_.font->GetSize() / 6;
		return pt;
	}

	void EditerView::setSize() {
		//static bool once = false;
		//if (!once) {
			tstring chars(T("abcdefghijklmnopqrstuvwxyz") \
							T("ABCDEFGHIJKLMNOPQRSTUVWXYZ") \
							T("0123456789!@#$%^&*()_+/?<>,.:;\'[]{}|\\\"~"));
			Gdiplus::Size sz = getTxtSize(chars);
			this->line_height_ = sz.Height + 1;
			this->char_width_ = sz.Width / chars.length() + 1;
			//once = true;
		//}
	}

	int EditerView::createCaret() {
		//static bool once = false;
		//if (!once) {
			int cw = 1;
			int size = (((cw + 15) & ~15) >> 3) * this->line_height_;
			char* bits = new char[size];
			::memset(bits, 255, size);
			HBITMAP hbp = ::CreateBitmap(cw, this->line_height_, 1, 1, 
				reinterpret_cast<BYTE*>(bits));
			delete [] bits;

			::CreateCaret(this->hw_, hbp, cw, this->line_height_);
			if (this->caret_bmp_) {
				::DeleteObject(this->caret_bmp_);
			}
			this->caret_bmp_ = hbp;
			::ShowCaret(this->hw_);
			//once = true;
		//}

		return 0;
	}

	static void del_tokens(std::list<Token*>& tl) {
		std::list<Token*>::iterator it = tl.begin();
		for (; it != tl.end(); ++it) {
			delete *it;
		}
	}

	static int get_bytes(const tstring& txt, int tab = 4) {
		int len = txt.length();
		int lw = len;
		for (int i = 0; i < len; ++i) {
			if (txt[i] > 0x80) {
				++lw;
			}
			if (txt[i] == T('\t')) {
				lw += tab - 1;
			}
		}
		return lw;
	}

	static bool lineInSelRgn(SelectPos& sp, 
		const Line* l, int& p1, int& p2) {
		if (sp.sel_2 < sp.sel_1) {
			std::swap(sp.sel_1, sp.sel_2);
		}

		if (sp.sel_1.id == l->index) {
			p1 = sp.sel_1.pos;
			if (sp.sel_2.id == l->index) {
				p2 = sp.sel_2.pos;
			} else {
				p2 = l->txt.length();
			}
		} else if (sp.sel_2.id == l->index) {
			p1 = 0;
			p2 = sp.sel_2.pos;
		} else if (sp.sel_1.id < l->index && l->index < sp.sel_2.id) {
			p1 = 0;
			p2 = l->txt.length();
		} else {
			return false;
		}
		return true;
	}

	void EditerView::drawSel(Gdiplus::Graphics* g, const Line* l) {
		if (!this->doc_->seleting()) {
			return;
		}
		SelectPos psp = this->doc_->sel_pos();
		if (psp.sel_1 == psp.sel_2) {
			return;
		}

		int p1, p2;
		if (!lineInSelRgn(psp, l, p1, p2)) {
			return;
		}

		Gdiplus::Rect rc;
		rc.Y = l->index * this->line_height_ - this->scroll_v_;
		rc.Height = this->line_height_;

		rc.X = get_bytes(l->txt.substr(0, p1), this->cfg_.tabstop) * this->char_width_ + this->lnu_width_;
		rc.Width = get_bytes(l->txt.substr(p1, p2 - p1), this->cfg_.tabstop) * this->char_width_;
		if (psp.sel_1.id != psp.sel_2.id && l->index != psp.sel_2.id && p2 - p1  == l->txt.length()) {
			rc.Width += this->char_width_;
		}

		rc.X += (int)this->cfg_.font->GetSize() / 6;

		Gdiplus::SolidBrush bkbr(this->cfg_.select_color);
		g->FillRectangle(&bkbr, rc);	
	}

	void EditerView::drawFind(Gdiplus::Graphics* g, Line* l) {
		if (!this->doc_->finding()) {
			return;
		}
		SelectPos p(this->doc_->found_pos());
		if (l->index != p.sel_1.id) {
			return;
		}
		Gdiplus::SolidBrush br(Gdiplus::Color::Yellow);
		Point pt1(this->pos2Pt(p.sel_1));
		Point pt2(this->pos2Pt(p.sel_2));
		Gdiplus::Rect rc;
		rc.X = pt1.x + this->scroll_h_;
		rc.Y = pt1.y;
		rc.Width = pt2.x - pt1.x;
		rc.Height = this->line_height_;
		g->FillRectangle(&br, rc);
	}

	void EditerView::drawLnu(Gdiplus::Graphics* g, Line* l, Gdiplus::PointF& pt) {
		if (!show_ln_) {
			this->lnu_width_ = 0;
			return;
		}
		int nl = this->doc_->GetLineList()->size();
		int id = l->index + 1;

		int lg = (int)log10((double)nl) + 1;
		int lg2 = (int)log10((double)id);

		tstring sp(T("-"));
		tstring sps;
		tchar num[16];
		::memset(num, 0, sizeof(num));
		for (int i = lg2; i < lg; ++i) {
			sps += sp;
		}
		::_stprintf_s(num, T("%s%d"), sps.c_str(), id);

		Gdiplus::SolidBrush nbr(Gdiplus::Color(192, 192, 192));
		g->DrawString(num, -1, this->cfg_.font, pt, &nbr);
		this->lnu_width_ = this->char_width_ * (lg + 3);
		pt.X += this->lnu_width_;
	}

	void EditerView::drawLine(Gdiplus::Graphics* g, Line* l) {
		this->drawSel(g, l);

		Gdiplus::PointF pt;
		pt.X = (Gdiplus::REAL)0;
		pt.Y = (Gdiplus::REAL)l->index * this->line_height_ - this->scroll_v_;
		this->drawLnu(g, l, pt);

		tstring txt(l->txt);
		Scanner sc(txt);
		std::list<Token*> tl;
		sc.Scan(tl);
		std::list<Token*>::iterator it = tl.begin();
		for (; it != tl.end(); ++it) {
			Token* tk = *it;
			Gdiplus::SolidBrush br(this->cfg_.txt_color);
			switch(tk->type) {
			case CommentType: 
				br.SetColor(this->cfg_.comment_color);
				break;
			case IdentType: 	
				br.SetColor(this->cfg_.txt_color);
				break;
			case KewordType: 	
				br.SetColor(this->cfg_.kw_color);
				break;
			case FuncType: 		
				br.SetColor(this->cfg_.func_color);
				break;
			case OperatorType: 	
				br.SetColor(this->cfg_.txt_color);
				break;
			case NumberType: 	
				br.SetColor(this->cfg_.num_color);
				break;
			case StringType:
				br.SetColor(this->cfg_.str_color);
				break;				
			case CtrlType: 	
				br.SetColor(this->cfg_.ctrl_color);
				break;
			case VariableType:
				br.SetColor(this->cfg_.var_color);
				break;				
			}
			Pos p1 = this->doc_->found_pos().sel_1;
			Pos p2 = this->doc_->found_pos().sel_2;
			Pos pp;
			pp.id = l->index;
			pp.pos = tk->pos;
			bool b = false;
			if (this->doc_->finding() && p1 == pp) {
				br.SetColor(Gdiplus::Color::Black);
				b = true;
			}
			int len = tk->txt.length();
			for (int i = 0; i < len; ++i) {
				if (b && (i > p2.pos - p1.pos - 1)) {
					br.SetColor(Gdiplus::Color::White);
				}
				g->DrawString(&tk->txt[i], 1, this->cfg_.font, pt, &br);
				pt.X += this->char_width_;
				if (tk->txt[i] > 0x80) {
					pt.X += this->char_width_;
				}
				if (tk->txt[i] == T('\t')) {
					pt.X += this->char_width_ * (this->cfg_.tabstop - 1);
				}
			}
		}
		del_tokens(tl);
	}

	void EditerView::showScroolV(int h) {
		LineList* ll = this->doc_->GetLineList();
		int tl = ll->size() * this->line_height_;

		if (tl > h) {
			::ShowScrollBar(this->hw_, SB_VERT, TRUE);
		} else {
			::ShowScrollBar(this->hw_, SB_VERT, FALSE);
			this->scroll_v_ = 0;
			return;
		}

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = tl + this->line_height_ * 5;
		si.nPage = h;
		si.nPos = min(this->scroll_v_, si.nMax - h);
		::SetScrollInfo(this->hw_, SB_VERT, &si, TRUE);
		if (this->scroll_v_ > si.nMax) {
			this->scroll_v_ = si.nMax;
		}
	}

	void EditerView::showScroolH(int w) {
		int max_w = this->max_line_w_ * this->char_width_;

		if (max_w > w) {
			::ShowScrollBar(this->hw_, SB_HORZ, TRUE);
		} else {
			::ShowScrollBar(this->hw_, SB_HORZ, FALSE);
			this->scroll_h_ = 0;
			return;
		}

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = max_w + this->char_width_ * 10 + this->lnu_width_;
		si.nPage = w;
		si.nPos = min(this->scroll_h_, si.nMax - w);
		::SetScrollInfo(this->hw_, SB_HORZ, &si, TRUE);
	}

	void EditerView::draw(Gdiplus::Graphics* g, int w, int h) {
		// draw background
		Gdiplus::SolidBrush br(this->cfg_.bkgrd_color);
		g->FillRectangle(&br, 0, 0, w, h);
		
		// draw text
		LineList* ll = this->doc_->GetLineList();

		// only draw the lines in client window
		LIT it1 = ll->begin();
		int cnt1 = this->scroll_v_ / this->line_height_;
		if (cnt1 > (int)ll->size()) {
			return;
		} else {
			std::advance(it1, cnt1);
		}

		LIT it2 = it1;
		int cnt2 = h / this->line_height_ + 1; // +1
		if (cnt2 + cnt1 > (int)ll->size()) {
			it2 = ll->end();
		} else {
			std::advance(it2, cnt2);
		}
		
		this->max_line_w_ = this->doc_->line_max();//max_line_w(it1, it2);	

		for (; it1 != it2; ++it1) {
			// draw line
			Line* l = *it1;
			this->drawFind(g, l);
			this->drawLine(g, l);
		}

		// set caret
		Point p = this->pos2Pt(this->doc_->pos());
		::SetCaretPos(p.x, p.y);
	}

	void EditerView::reDraw() {
		RECT rc;
		::GetClientRect(this->hw_, &rc);
		::InvalidateRect(this->hw_, &rc, TRUE);
	}

	void EditerView::draw(HDC hdc) {
		RECT rect;
		::GetClientRect(this->hw_, &rect);
		int x = rect.left;
		int y = rect.top;
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		int rw = this->scroll_h_ + w;
		int rh = h + this->line_height_;
		HBITMAP memmap = ::CreateCompatibleBitmap(hdc, rw, rh);
		HDC mdc = ::CreateCompatibleDC(hdc); 
		HBITMAP oldmap = (HBITMAP)::SelectObject(mdc, (HGDIOBJ)memmap);
		Gdiplus::Graphics* g = new Gdiplus::Graphics(mdc);

		this->showScroolH(w);
		this->showScroolV(h);

		this->draw(g, rw, rh);
		
		BitBlt(hdc, 
			x, y, w, h, 
			mdc, x + this->scroll_h_, y, SRCCOPY);

		delete g;
		::SelectObject(mdc, oldmap);
		::DeleteObject(memmap); 
		::DeleteDC(mdc);
	}

	void EditerView::showTips() {
		std::list<tstring>* lst = this->doc_->GetTips();
		if (!lst || lst->empty()) {
			::ShowWindow(this->tipbox_, SW_HIDE);
			return;
		}

		tstring ident(this->doc_->GetCurrentIndent());
		if (ident.empty()) {
			::ShowWindow(this->tipbox_, SW_HIDE);
			return;
		}

		Point pt(this->pos2Pt(this->doc_->pos()));
		int w = 200;
		int y = this->line_height_ * 10;
		if (lst->size() <= 10) {
			y = lst->size() * this->line_height_;
		}
		RECT rc;
		::GetClientRect(this->hw_, &rc);
		if (pt.x + w > rc.right) {
			pt.x = rc.right - w;
		}

		::MoveWindow(this->tipbox_, pt.x, pt.y + this->line_height_, w, y, TRUE);
		::ShowWindow(this->tipbox_, SW_SHOW);

		int cnt = ::SendMessage(this->tipbox_, LB_GETCOUNT, 0, 0);
		for (int i = 0; i < cnt; ++i) {
			::SendMessage(this->tipbox_, LB_DELETESTRING, 0, 0);
		}
		std::list<tstring>::iterator it = lst->begin();
		for (; it != lst->end(); ++it) {
			::SendMessage(this->tipbox_, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(it->c_str()));
		}
		::SendMessage(this->tipbox_, LB_SETCURSEL, 0, 0);
		::SetFocus(this->tipbox_);
	}

	void EditerView::input(int c, int b) {
		if (c < 32 && c != T('\n') && c != T('\r') && c != T('\t')) {
			return;
		}
		if (c == T('\r')) {
			c = T('\n');
		}
		if (this->doc_->seleting()) {
			RUEvent* e = new DeleteSel(this->doc_);
			Commander::instance()->Do(this->id_, e);
		}
		RUEvent* e = new Input(this->doc_,  c);
		Commander::instance()->Do(this->id_, e);
		this->updatePos();
		this->showTips();
	}

	static int tokenRightPos(Line* l, int pos) {
		Scanner sc(l->txt);
		std::list<Token*> tl;
		sc.Scan(tl);
		std::list<Token*>::iterator it = tl.begin();
		Token* tk = 0;
		for (; it != tl.end(); ++it) {
			tk = *it;
			if (tk->pos > pos) {
				break;
			}
		}
		if (it == tl.end()) {
			pos = l->txt.length();
		} else {
			pos = tk->pos;
		}
		del_tokens(tl);
		return pos;
	}

	static int tokenLeftPos(Line* l, int pos) {
		tstring s(l->txt.substr(0, pos));
		Scanner sc(s);
		std::list<Token*> tl;
		sc.Scan(tl);
		std::list<Token*>::iterator it = tl.begin();
		Token* tk = 0;
		for (; it != tl.end(); ++it) {
			tk = *it;
			int len = tk->txt.length() + 1;
			if (tk->pos + len > pos) {
				break;
			}
		}
		if (it == tl.end()) {
			pos = l->txt.length();
		} else {
			pos = tk->pos;
		}
		del_tokens(tl);
		return pos;
	}

	void EditerView::keyDown(int k, int b) {
		Pos pos = this->doc_->pos();
		Line* l = this->doc_->GetLine(pos);
		LineList* ll = this->doc_->GetLineList();
		bool selecting = this->doc_->seleting();

		RECT rc;
		::GetClientRect(this->hw_, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;	

		switch(k) {
		case VK_ESCAPE: 
			{
				::ShowWindow(this->tipbox_, SW_HIDE);
				this->fb_->Hide();
				this->rb_->Hide();
				this->fb_show_ = false;
				this->rb_show_ = false;
				RECT rc;
				::GetClientRect(this->pw_, &rc);
				::MoveWindow(this->hw_, 0, 0, rc.right, rc.bottom, TRUE);
			}
			break;
		case VK_DELETE:
			if (selecting) {
				RUEvent* e = new DeleteSel(this->doc_);
				Commander::instance()->Do(this->id_, e);
				selecting = true;
			} else {
				RUEvent* e = new Delete(this->doc_);
				Commander::instance()->Do(this->id_, e);
			}
			this->showTips();
			break;
		case VK_BACK:
			if (selecting) {
				RUEvent* e = new DeleteSel(this->doc_);
				Commander::instance()->Do(this->id_, e);
				selecting = true;
			} else {
				RUEvent* e = new DeleteBack(this->doc_);
				Commander::instance()->Do(this->id_, e);
			}
			this->showTips();
			break;
		case VK_END:
			if (::GetKeyState(VK_CONTROL) < 0) { // goto text end
				pos.id = ll->size() - 1;
				pos.pos = ll->back()->txt.length();
			} else { // move caret to the most right
				pos.pos = l->txt.length();
			}
			break;
		case VK_HOME:
			if (::GetKeyState(VK_CONTROL) < 0) { // goto text begin
				pos.id = 0;
				pos.pos = 0;
			} else { // move caret to the most left
				pos.pos = 0;
			}
			break;
		case VK_LEFT:
			if (pos.pos > 0) { // move a char
				if (::GetKeyState(VK_CONTROL) < 0) { // move a token
					int p = tokenLeftPos(l, pos.pos);
					pos.pos = p;
				} else {
					--pos.pos;
				}
			} else if (pos.id != 0) { // move prev line
				--pos.id;
				Line* nl = this->doc_->GetLine(pos);
				pos.pos = nl->txt.length();
				if (::GetKeyState(VK_CONTROL) < 0) { // move a token
					int p = tokenLeftPos(nl, nl->txt.length());
					pos.pos = p;
				}
			}
			break;
		case VK_RIGHT:
			if (pos.pos < (int)l->txt.length()) {
				if (::GetKeyState(VK_CONTROL) < 0) { // move a token
					int p = tokenRightPos(l, pos.pos);
					pos.pos = p;
				} else {
					++pos.pos;
				}
			} else { // move next line
				++pos.id;
				if (pos.id == ll->size()) {
					--pos.id;
				} else {
					pos.pos = 0;
					if (::GetKeyState(VK_CONTROL) < 0) { // move a token
						Line* nl  = this->doc_->GetLine(pos);	
						int p = tokenRightPos(nl, 0);
						pos.pos = p;
					}
				}
			}
			break;
		case VK_UP:
			// move caret up
			if (pos.id != 0) {
				--pos.id;
				Line* nl = this->doc_->GetLine(pos);
				int nlw = nl->txt.length();
				if (pos.pos > nlw) {
					pos.pos = nlw;
				}
			}
			break;
		case VK_DOWN:
			// move caret down
			++pos.id;
			if (pos.id != ll->size()) {
				Line* nl = this->doc_->GetLine(pos);
				int nlw = nl->txt.length();
				if (pos.pos > nlw) {
					pos.pos = nlw;
				}
			} else {
				--pos.id;
			}
			break;
		case VK_PRIOR: // page up
			{
				this->scrollV(SB_PAGEUP, 0);
				int cnt = h / this->line_height_;
				pos.id -= cnt;
				if (pos.id < 0) {
					pos.id = 0;
				}
			}
			break;
		case VK_NEXT: // page down
			{
				this->scrollV(SB_PAGEDOWN, 0);
				int cnt = h / this->line_height_;
				pos.id += cnt;
				if (pos.id >= (int)ll->size()) {
					pos.id = ll->back()->index;
				}
			}
			break;
		case VK_SHIFT: // begin select
			{
				SelectStart e(this->doc_, pos);
				e.Do();
			}
			return;
		case VK_F3: // find next
			this->find_next();
			break;
		case VK_F4: // find prev
			this->find_prev();
			break;
		case VK_INSERT:
			if (::GetKeyState(VK_CONTROL) < 0) { // copy
				this->copy();
			} else if (::GetKeyState(VK_SHIFT) < 0) { // paste
				RUEvent* e = new DeleteSel(this->doc_);
				Commander::instance()->Do(this->id_, e);
				this->paste();
			}
			break;
		case VK_ADD:
			{
				int n = 0;
			}
			break;
		default:
			break;
		}

		if (k > 0x20 && k < 0x29) { // home end left right up down pageup and pagedown
			if (::GetKeyState(VK_SHIFT) < 0) {
				SelectEnd e(this->doc_, pos);
				e.Do();
				this->doc_->set_seleting(true);
			} else if (this->doc_->seleting()) {
				SelectPos sel(this->doc_->sel_pos());
				if (sel.sel_2 < sel.sel_1) {
					std::swap(sel.sel_1, sel.sel_2);
				}
				if (k == VK_LEFT) {
					pos = sel.sel_1;
				} else if (k == VK_RIGHT) {
					pos = sel.sel_2;
				}
				this->doc_->set_seleting(false);
			} 
			this->doc_->set_pos(pos);
		}
		

		if (::GetKeyState(VK_CONTROL) < 0 && k != VK_CONTROL) { // process ctrl + key
			this->ctrlKey(k);
		} 

		this->updatePos(w, h);			
		this->reDraw();
	}

	void EditerView::copy() {
		if (!::OpenClipboard(NULL)) {
			return;
		}	

		SelectPos& pos = this->doc_->sel_pos();
		if (pos.sel_2 < pos.sel_1) {
			std::swap(pos.sel_1, pos.sel_2);
		}
		if (pos.sel_1 == pos.sel_2) {
			::CloseClipboard();
			return;
		}
		::EmptyClipboard();
		tstring txt(this->doc_->GetText(pos.sel_1, pos.sel_2));
		int len = txt.length() * sizeof(txt[0]);
		HGLOBAL ghl = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, len + sizeof(txt[0]));
		if (ghl == NULL) {
			::CloseClipboard();
			return;
		}

		tchar* copy = (tchar*)::GlobalLock(ghl);
		::memcpy(copy, txt.c_str(), len);
		::GlobalUnlock(ghl);
		copy[txt.length()] = 0;

		::SetClipboardData(CF_UNICODETEXT, copy);
		::CloseClipboard();
	}

	void EditerView::paste() {
		if (!::OpenClipboard(NULL)) {
			return;
		}
		tchar* txt = (tchar*)::GetClipboardData(CF_UNICODETEXT);
		::CloseClipboard();

		SelectPos sel(this->doc_->sel_pos());
		if (this->doc_->seleting() && sel.sel_1 != sel.sel_2) {
			RUEvent* e = new DeleteSel(this->doc_);
			Commander::instance()->Do(this->id_, e);
		}
		RUEvent* e = new Paste(this->doc_, txt);
		Commander::instance()->Do(this->id_, e);
	}

	void EditerView::cut() {
		this->copy();
		RUEvent* e = new DeleteSel(this->doc_);
		Commander::instance()->Do(this->id_, e);
	}

	static std::wstring module_path() {
		wchar_t path[256];
		if (::GetModuleFileName(NULL, path, 256) == 0) {
			return tstring();
		}
		std::wstring ws(path);
		std::size_t pos = ws.find_last_of(T("\\"));
		if (pos != std::wstring::npos) {		
			return ws.substr(0, pos);
		}
		return tstring();
	}

	void EditerView::find_prev() {
		if (this->find_txt_.empty()) {
			return;
		}
		FindPrevEvent fe(this->doc_);
		fe.Find(this->find_txt_, this->match_case_, this->match_whole_word_);
		this->finding_ = true;
		this->updatePos();
	}

	void EditerView::find_next() {
		if (this->find_txt_.empty()) {
			return;
		}
		FindNextEvent fe(this->doc_);
		fe.Find(this->find_txt_, this->match_case_, this->match_whole_word_);
		this->finding_ = true;
		this->updatePos();
	}

	void EditerView::find_all() {
		if (this->find_txt_.empty()) {
			return;
		}
		FindEvent* fe = new FindAllEvent(this->doc_);
		fe->Find(this->find_txt_, this->match_case_, this->match_whole_word_);
		this->finding_ = true;
		delete fe;
	}

	void EditerView::replace_find() {
		if (this->find_txt_.empty()) {
			return;
		}
		if (!this->finding_) {
			this->find_next();
			return;
		}

		RUEvent* e = 
			new Replace(this->doc_, 
					this->find_txt_, 
					this->rep_txt_);
		Commander::instance()->Do(this->id_, e);
		this->finding_ = false;
		this->updatePos();
	}

	void EditerView::replace_all() {
		if (this->find_txt_.empty()) {
			return;
		}
		
		RUEvent* e = 
			new ReplaceAll(this->doc_, 
					this->find_txt_, 
					this->rep_txt_, 
					this->match_case_, 
					this->match_whole_word_);
		Commander::instance()->Do(this->id_, e);
		this->updatePos();
	}


	void EditerView::find() {
		if (this->fb_show_) {
			return;
		}
		this->rb_->Hide();
		this->rb_show_ = false;

		RECT rc;
		::GetClientRect(this->pw_, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;


		int fw = w;
		int fh = this->line_height_ * 2 - 5;
		int fx = rc.left;
		int fy = rc.top + h - fh;

		::MoveWindow(this->hw_, 0, 0, w, h - fh, TRUE);

		this->fb_->Show(fx, fy, fw, fh);

		this->fb_show_ = true;
		this->reDraw();
	}

	void EditerView::replace() {
		if (this->rb_show_) {
			return;
		}
		this->fb_->Hide();
		this->fb_show_ = false;

		RECT rc;
		::GetClientRect(this->pw_, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;


		int rw = w;
		int rh = this->line_height_ * 4 - 20;
		int rx = rc.left;
		int ry = rc.top + h - rh;

		::MoveWindow(this->hw_, 0, 0, w, h - rh, TRUE);

		this->rb_->Show(rx, ry, rw, rh);

		this->replacing_ = true;
		this->reDraw();
	}

	void EditerView::goto_ln() {
		//::MoveWindow(this->goto_box_, 200, 0, 300, this->line_height_ + 10, TRUE);
		//::ShowWindow(this->goto_box_, SW_SHOW);
		::MoveWindow(this->goto_box_edit_, 200, 0, 300, this->line_height_+10, TRUE);
		::ShowWindow(this->goto_box_edit_, SW_SHOW);
		::SetFocus(this->goto_box_edit_);
	}

	void EditerView::ctrlKey(int k) {
		switch (k) {
		case T('C') : // copy
			this->copy();
			break;
		case T('V') : // paste
			this->paste();
			break;
		case T('X') : // cut
			this->cut();
			break;
		case T('Z') : // undo
			Commander::instance()->Undo(this->id_);
			break;
		case T('Y') : // redo
			Commander::instance()->Redo(this->id_);
			break;
		case T('F') : // find
			this->find();
			break;
		case T('H') : // replace
			this->replace();
			break;
		case T('G') : // goto
			this->goto_ln();
			break;
		case T('A') : // select all
			{
				SelectAll e(this->doc_);
				e.Do();
			}
			break;
		case T('S') : // save
			break;
		case T('L') : // show line number
			this->show_ln_ = !this->show_ln_;
			break;
		case VK_OEM_PLUS : // ++font size
			{
				Gdiplus::FontFamily ff;
				this->cfg_.font->GetFamily(&ff);
				auto size = this->cfg_.font->GetSize();
				++size;
				delete this->cfg_.font;
				this->cfg_.font = new Gdiplus::Font(&ff, size, 
					Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				this->setSize();
				this->createCaret();
			}
			break;
		case VK_OEM_MINUS : // --font size
			{
				Gdiplus::FontFamily ff;
				this->cfg_.font->GetFamily(&ff);
				auto size = this->cfg_.font->GetSize();
				--size;
				if (size < 5) {
					return;
				}
				delete this->cfg_.font;
				this->cfg_.font = new Gdiplus::Font(&ff, size, 
					Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				this->setSize();
				this->createCaret();
			}
			break;
		default:
			break;
		}

		return;
	}
	
	void EditerView::updatePos() {
		RECT rc;
		::GetClientRect(this->hw_, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		this->updatePos(w, h);
		::InvalidateRect(this->hw_, &rc, TRUE);
	}

	void EditerView::updatePos(int w, int h) {
		LineList* ll = this->doc_->GetLineList();

		Pos& pos = this->doc_->pos();
		Line* l = this->doc_->GetLine(pos);
		tstring s(l->txt.substr(0, pos.pos));
		int cnt = get_bytes(s, this->cfg_.tabstop);
		int lw = cnt * this->char_width_ + this->lnu_width_;
	
		while (this->scroll_h_ + w < lw + this->char_width_) {
			this->scroll_h_ += this->char_width_ * 10;
		} 
		while(lw < this->scroll_h_) {
			this->scroll_h_ -= this->char_width_ * 10;
		}
		if (this->scroll_h_ < 0) {
			this->scroll_h_ = 0;
		}
		
		while (this->scroll_v_ + h < pos.id * this->line_height_ + this->line_height_) {
			this->scroll_v_ += this->line_height_;
		}
		while (this->scroll_v_ > pos.id * this->line_height_) {
			this->scroll_v_ -= this->line_height_;
		}
		if (this->scroll_v_ < 0) {
			this->scroll_v_ = 0;
		}
	}

	void EditerView::lbdClickDown(int x, int y) { // select a token
	}

	bool EditerView::ptInSelRgn(const Point& pt) {
		if (this->doc_->seleting()) {
			Pos pos(this->pt2Pos(pt));
			Line* l = this->doc_->GetLine(pos);
			int p1, p2;
			if (!lineInSelRgn(this->doc_->sel_pos(), l, p1, p2)) {
				return false;
			}
			POINT ptt;
			ptt.x = pt.x;
			ptt.y = pt.y + this->scroll_v_;

			RECT rc;
			rc.top = l->index * this->line_height_;
			rc.bottom = l->index * this->line_height_ + this->line_height_;

			rc.left = get_bytes(l->txt.substr(0, p1), this->cfg_.tabstop) * this->char_width_ + this->lnu_width_;
			int w = get_bytes(l->txt.substr(p1, p2 - p1), this->cfg_.tabstop) * this->char_width_;
			
			rc.left += (int)this->cfg_.font->GetSize() / 6;
			rc.right = rc.left + w;
			if (::PtInRect(&rc, ptt)) {
				return true;
			}
		}
		return false;
	}

	void EditerView::lbClickDown(int x, int y) {
		::SetFocus(this->hw_);
		Point pt;
		pt.x = x;
		pt.y = y;
		if (ptInSelRgn(pt) || x < this->lnu_width_) {
			this->setCursor(CursorArrow);
			if (x < this->lnu_width_) {
				SelectLine e(this->doc_, this->pt2Pos(pt));
				e.Do();
			} else {
				this->moving_ = true;
			}
		} else {
			SelectStart e(this->doc_, this->pt2Pos(pt));
			e.Do();
		}
		this->reDraw();
	}

	void EditerView::lbClickUp(int x, int y) {
		if (this->moving_) { // insert select txt 
			RUEvent* e = new Move(this->doc_);
			Commander::instance()->Do(this->id_, e);			
			this->reDraw();
		}
		this->moving_ = false;
	}

	void EditerView::mmove(int x, int y) {
		Point pt;
		pt.x = x;
		pt.y = y;

		if (ptInSelRgn(pt)) {
			this->setCursor(CursorArrow);
		} else if (x < this->lnu_width_) {
			this->setCursor(CursorReverseArrow);
		}
		//this->reDraw();
	}

	void EditerView::lbdMmove(int x, int y) { 
		Point pt;
		pt.x = x;
		pt.y = y;

		Pos pos(this->pt2Pos(pt));
		this->doc_->set_pos(pos);

		if (this->moving_) {
			if (ptInSelRgn(pt)) {
				this->setCursor(CursorInvalid);
			} else {
				this->setCursor(CursorHand);
			}
		} else {		
			SelectEnd e(this->doc_, pos);
			e.Do();		
		}

		
		RECT rc;
		::GetClientRect(this->hw_, &rc);
		if (y >= rc.bottom - this->line_height_) {
			++pos.id;
			//this->scrollV(SB_LINEDOWN, 0);
		} else if (y <= rc.top + this->line_height_) {
			--pos.id;
			//this->scrollV(SB_LINEUP, 0);
		}
		if (x <= rc.left + this->char_width_) {
			--pos.pos;
		} else if (x >= rc.right - this->char_width_) {
			//this->scrollH(SB_LINERIGHT, 0);
			++pos.pos;
		}
		this->doc_->set_pos(pos);
		this->updatePos(rc.right, rc.bottom);

		this->reDraw();
	}
	
	void EditerView::scrollV(int sb, int pos) {
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask  = SIF_ALL;
		::GetScrollInfo(this->hw_, SB_VERT, &si);
		int ph = si.nPage;
		this->scroll_v_ = si.nPos;

		switch(sb) {
		case SB_LINEUP:
			this->scroll_v_ -= this->line_height_;
			break;
		case SB_LINEDOWN:
			this->scroll_v_ += this->line_height_;
			break;
		case SB_PAGEUP:
			this->scroll_v_ -= ph;
			break;
		case SB_PAGEDOWN:
			this->scroll_v_ += ph;
			break;
		case SB_THUMBTRACK:
			this->scroll_v_ = si.nTrackPos;
			break;
		case SB_THUMBPOSITION:
			this->scroll_v_ = si.nTrackPos;
			break;
		case -1:
			if ((short)pos > 0) {
				this->scroll_v_ -= this->line_height_ * 3;
			} else {
				this->scroll_v_ += this->line_height_ * 3;
			}
			break;
		default:
			break;
		}

		if (this->scroll_v_ < 0) {
			this->scroll_v_ = 0;
		}
		if (this->scroll_v_ > si.nMax) {
			this->scroll_v_ = si.nMax;
		}
		
		this->reDraw();
	}

	void EditerView::scrollH(int sb, int pos) {
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask  = SIF_ALL;
		::GetScrollInfo(this->hw_, SB_HORZ, &si);
		int pw = si.nPage;
		this->scroll_h_ = si.nPos;

		switch(sb) {
		case SB_LINELEFT:
			this->scroll_h_ -= this->char_width_;
			break;
		case SB_LINERIGHT:
			this->scroll_h_ += this->char_width_;
			break;
		case SB_PAGELEFT:
			this->scroll_h_ -= pw;
			break;
		case SB_PAGERIGHT:
			this->scroll_h_ += pw;
			break;
		case SB_THUMBTRACK:
			this->scroll_h_ = pos;
			break;
		case SB_THUMBPOSITION:
			this->scroll_h_ = pos;
			break;
		case -1:
			if ((short)pos > 0) {
				this->scroll_h_ -= this->char_width_ * 3;
			} else {
				this->scroll_h_ += this->char_width_ * 3;
			}
			break;
		default:
			break;
		}

		if (this->scroll_h_ < 0) {
			this->scroll_h_ = 0;
		}
		if (this->scroll_h_ > si.nMax) {
			this->scroll_h_ = si.nMax;
		}

		this->reDraw();
	}

	void EditerView::tipboxProc(int wp, int lp) {
		if (wp == LBN_DBLCLK) {
			int i = (int)SendMessage(this->tipbox_, LB_GETCURSEL, 0, 0); 
			tchar buf[256];
			::SendMessage(this->tipbox_, LB_GETTEXT, i, (LPARAM)buf);
			::ShowWindow(this->tipbox_, SW_HIDE);
			
			Pos pos(this->doc_->pos());			
			SelectStart ss(this->doc_, pos);
			ss.Do();
			
			tstring ident(this->doc_->GetCurrentIndent());
			pos.pos -= ident.length();
			if (ident[0] == T('%')) {
				++pos.pos;
			}
			SelectEnd se(this->doc_, pos);
			se.Do();
			
			RUEvent* e = new DeleteSel(this->doc_);
			Commander::instance()->Do(this->id_, e);

			e = new Paste(this->doc_, buf);
			Commander::instance()->Do(this->id_, e);
			this->doc_->set_seleting(false);

			this->reDraw();
			return;
		}
		if (wp == LBN_KILLFOCUS) {
			::ShowWindow(this->tipbox_, SW_HIDE);
		}
	}

	LRESULT CALLBACK EditerView::GotoboxProc(int msg, int wp, int lp) {
		if (msg == WM_KEYDOWN) {
			if (wp == VK_RETURN) {
				tchar txt[1024];
				::GetWindowText(this->goto_box_edit_, txt, 1024);
				int n = ::_ttol(txt);
				Pos pos;
				pos.id = n;
				pos.pos = 0;
				this->doc_->set_pos(pos);
				::ShowWindow(this->goto_box_edit_, SW_HIDE);
				::SetFocus(this->hw_);
				this->updatePos();
				return 0;
			}
			if (wp == VK_ESCAPE) {
				::ShowWindow(this->goto_box_edit_, SW_HIDE);
				::SetFocus(this->hw_);
				//this->reDraw();
				return 0;
			}
		}
		return ::CallWindowProc(this->goto_proc_, this->goto_box_edit_, msg, wp, lp);
	}

	LRESULT CALLBACK EditerView::TipboxProc(int msg, int wp, int lp) {
		if (msg == WM_KEYDOWN) {
			int i = ::SendMessage(this->tipbox_, LB_GETCURSEL, 0, 0);
			int cnt = ::SendMessage(this->tipbox_, LB_GETCOUNT, 0, 0);
			switch(wp) {
			case VK_RETURN:
				tipboxProc(LBN_DBLCLK, 0);
				break;
			case VK_ESCAPE:
				::ShowWindow(this->tipbox_, SW_HIDE);
				break;
			case VK_UP:
				if (i > 0) {
					::SendMessage(this->tipbox_, LB_SETCURSEL, --i, 0);
				}
				break;
			case VK_DOWN:
				if (i < cnt) {
					::SendMessage(this->tipbox_, LB_SETCURSEL, ++i, 0);
				}
				break;
			case VK_HOME:
				::SendMessage(this->tipbox_, LB_SETCURSEL, 0, 0);
				break;
			case VK_END:
				::SendMessage(this->tipbox_, LB_SETCURSEL, cnt, 0);
				break;
			case VK_PRIOR:
				i -= 10;
				if (i < 0) {
					i = 0;
				}
				::SendMessage(this->tipbox_, LB_SETCURSEL, i, 0);
				break;
			case VK_NEXT:
				i += 10;
				if (i > cnt) {
					i = cnt;
				}
				::SendMessage(this->tipbox_, LB_SETCURSEL, i, 0);
				break;
			default:
				//return ::CallWindowProc(this->tipbox_proc_, this->tipbox_, msg, wp, lp);
				::SendMessage(this->hw_, WM_KEYDOWN, wp, 0);
				return 0;
			}
			return 0;
		} else if (msg == WM_CHAR) {
			if (wp == 13) {
				return 0;
			}
			::SendMessage(this->hw_, WM_CHAR, wp, 0);
			return 0;
		}

		return ::CallWindowProc(tipbox_proc_, this->tipbox_, msg, wp, lp);
	}

	LRESULT CALLBACK EditerView::WndProc(UINT message, WPARAM wParam, LPARAM lParam) {
		PAINTSTRUCT ps;
		HDC hdc;

		switch(message) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_TIPBOX) {
				this->tipboxProc(HIWORD(wParam), lParam);
			}
			if (LOWORD(wParam) == IDC_FINDBOX) {
				//this->findboxProc(HIWORD(wParam), lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(this->hw_, &ps);
			this->draw(hdc);
			EndPaint(this->hw_, &ps);
			break;
		case WM_SIZE:
			this->reDraw();
			break;
		case WM_ERASEBKGND:
			break;
		case WM_DESTROY:
			::SetWindowLongPtr(this->hw_, GWLP_USERDATA, NULL);
		case WM_KEYDOWN:
			this->keyDown(wParam, lParam);
			break;
		case WM_CHAR:
			this->input(wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			this->lbClickDown(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONUP:
			this->lbClickUp(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_VSCROLL:
			this->scrollV(LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_HSCROLL:
			this->scrollH(LOWORD(wParam), HIWORD(wParam));
			break;
		case WM_MOUSEWHEEL:
			this->scrollV(-1, HIWORD(wParam));
			break;
		case WM_SETFOCUS:
			::CreateCaret(this->hw_, this->caret_bmp_, 1, this->line_height_);
			::ShowCaret(this->hw_);
			break;
		case WM_KILLFOCUS:
			::HideCaret(this->hw_);
			::DestroyCaret();
			break;
		case WM_MOUSEMOVE:
			(wParam & MK_LBUTTON) == MK_LBUTTON ? 
				this->lbdMmove(LOWORD(lParam), HIWORD(lParam)) :
			this->mmove(LOWORD(lParam), HIWORD(lParam));

			break;
		default:
			return ::DefWindowProc(this->hw_, message, wParam, lParam);
		}

		return 0;
	}

} // namespace wediter