/*
 * button.cc
 */

#include "stdafx.h"
#include "view.h"
#include "resource.h"


namespace wediter {
	/// Button
	Button::Button() 
		: checkbox_(false), btn_down_(false), moving_(false), checked_(false) {
	}

	Button::~Button() {
	}

	static LRESULT CALLBACK _btnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Button* btn = (Button*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (btn == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return btn->WndProc(message, wParam, lParam);
	}

	void Button::draw(HDC dc) {
		//Gdiplus::Graphics g(dc);
		RECT rc;
		::GetClientRect(this->hw_, &rc);
		int x = rc.left;
		int y = rc.top;
		int w = rc.right - rc.left - 1;
		int h = rc.bottom - rc.top - 1;
		
		HBITMAP memmap = ::CreateCompatibleBitmap(dc, w, h);
		HDC mdc = ::CreateCompatibleDC(dc); 
		HBITMAP oldmap = (HBITMAP)::SelectObject(mdc, (HGDIOBJ)memmap);
		Gdiplus::Graphics* g = new Gdiplus::Graphics(mdc);
		

		Gdiplus::Rect grc(x, y, w, h);
		Gdiplus::Pen pen(Gdiplus::Color(37, 37, 37));
		g->DrawRectangle(&pen, grc);

		Gdiplus::LinearGradientBrush lgb(grc, 
			Gdiplus::Color(61, 62, 63),
			Gdiplus::Color(101, 102, 103),  
			271, Gdiplus::LinearGradientModeVertical);
		g->FillRectangle(&lgb, x + 2, y + 2, w - 3, h - 3);

		pen.SetColor(Gdiplus::Color(119, 119, 119));
		g->DrawLine(&pen, w+1, y+h+1, x+w, y+h+1);

		if (this->btn_down_) {
			Gdiplus::SolidBrush br(Gdiplus::Color(37, 37, 37));
			g->FillRectangle(&br, x + 1, y + 1, w - 1, h - 1);
		}

		pen.SetColor(Gdiplus::Color(97, 98, 100));
		if (this->moving_) {
			pen.SetColor(Gdiplus::Color::Red);
		}
		g->DrawRectangle(&pen, x + 1, y + 1, w - 2, h - 2);

		Gdiplus::SolidBrush sbr(Gdiplus::Color::White);
		g->DrawString(this->txt_.c_str(), this->txt_.length(), this->font_, Gdiplus::PointF(2, 0), &sbr);

		BitBlt(dc, 
			x, y, w, h, 
			mdc, x, y, SRCCOPY);

		delete g;
		::SelectObject(mdc, oldmap);
		::DeleteObject(memmap); 
		::DeleteDC(mdc);
	}

	LRESULT CALLBACK Button::WndProc(UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = ::BeginPaint(this->hw_, &ps);
				this->draw(dc);
				::EndPaint(this->hw_, &ps);
			}
			break;
		case WM_LBUTTONDOWN:
			this->btn_down_ = true;
			::InvalidateRect(this->hw_, NULL, TRUE);
			::SetCapture(this->hw_);
			break;
		case WM_LBUTTONUP:
			if (this->checkbox_) {
				this->checked_ = !this->checked_;
				this->btn_down_ = this->checked_;
			} else {
				this->btn_down_ = false;
			}
			::InvalidateRect(this->hw_, NULL, TRUE);
			::ReleaseCapture();
			this->cw_->Notify(this->id_);
			break;
		case WM_MOUSEMOVE:
			{
				::SetCapture(this->hw_);
				POINT pt;
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				RECT rc;
				bool b = this->moving_;
				::GetClientRect(this->hw_, &rc);
				if (::PtInRect(&rc, pt)) {
					this->moving_ = true;
				} else {
					this->moving_ = false;
					if ((wParam & MK_LBUTTON) == 0) {
						::ReleaseCapture();
					}
				}
				if (b != this->moving_) {
					::InvalidateRect(this->hw_, NULL, TRUE);
				}
			}
			break;
		case WM_MOUSELEAVE:
			this->moving_ = false;
			break;
		case WM_ERASEBKGND:
			//this->moving_ = false;
			//this->btn_down_ = false;
			break;
		default:
			return ::CallWindowProc(this->btn_proc_, this->hw_, message, wParam, lParam);
		}
		return 0;
	}

	int Button::Init(int id, CtrlWindow* cw, HWND pw, HINSTANCE hinst, Gdiplus::Font* f, bool checkbox, int idc) {
		HWND hw = ::CreateWindowEx(0, T("button"), T(""),
			WS_CHILD | WS_VISIBLE | BS_CENTER | BS_FLAT | BS_OWNERDRAW, 
			0, 0, 0, 0,
			pw, (HMENU)idc, hinst, NULL);
		if (!hw) {
			return -1;
		}

		this->hw_ = hw;
		this->font_ = f;
		this->checkbox_ = checkbox;
		this->cw_ = cw;
		this->id_ = id;

		::SetWindowLongPtr(this->hw_, GWLP_USERDATA, (LONG)this);
		this->btn_proc_ = (WNDPROC)::SetWindowLongPtr(this->hw_, GWLP_WNDPROC, (LONG)_btnProc);

		return 0;
	}

	void Button::SetText(const tstring& txt, Gdiplus::Color color) {
		this->txt_ = txt;
		this->txt_color_ = color;
	}

	void Button::SetBkColor(Gdiplus::Color color) {
		this->bk_color_ = color;
	}

	void Button::Show(int x, int y, int w, int h) {
		::MoveWindow(this->hw_, x, y, w, h, TRUE);
		::UpdateWindow(this->hw_);
		::ShowWindow(this->hw_, SW_SHOW);
	}

	void Button::Show(int x, int y) {
		this->Show(x, y, 25, 25);
	}

	void Button::SetRgn(HRGN rgn) {
		::SetWindowRgn(this->hw_, rgn, TRUE);
	}

}