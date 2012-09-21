/*
 * replace.cc
 */

#include "stdafx.h"
#include "view.h"
#include "resource.h"
#include "doc.h"

namespace wediter {

/// RepBox
	RepBox::RepBox(EditerView* view) : view_(view) {
	}

	RepBox::~RepBox() {
	}

	static LRESULT CALLBACK _repboxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		RepBox* rb = (RepBox*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (rb == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return rb->WndProc(message, wParam, lParam);
	}

	/*
	static LRESULT CALLBACK _editProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		RepBox* fb = (RepBox*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (fb == NULL) {
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}

		return fb->EdtProc(message, wParam, lParam);
	}
	*/

	int RepBox::Init(HWND pw, HINSTANCE hinst, HFONT hf) {
		HWND hw = ::CreateWindowEx(0, T("static"), T(""),
			WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0,
			pw, (HMENU)IDC_FINDBOX, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->hw_ = hw;
		
		hw = ::CreateWindowEx(WS_EX_CLIENTEDGE, T("edit"), T(""),
			WS_CHILD | WS_VISIBLE | ES_LEFT,
			0, 0, 0, 0,
			this->hw_, (HMENU)IDC_REPBOX_FIND_EDIT, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->find_edit_ = hw;

		hw = ::CreateWindowEx(WS_EX_CLIENTEDGE, T("edit"), T(""),
			WS_CHILD | WS_VISIBLE | ES_LEFT,
			0, 0, 0, 0,
			this->hw_, (HMENU)IDC_REPBOX_REP_EDIT, hinst, NULL);
		if (!hw) {
			return -1;
		}
		this->rep_edit_ = hw;

		::SetWindowLongPtr(this->hw_, GWLP_USERDATA, (LONG)this);
		this->repbox_proc_ = (WNDPROC)::SetWindowLongPtr(this->hw_, GWLP_WNDPROC, (LONG)_repboxProc);

		//::SetWindowLongPtr(this->edit_, GWLP_USERDATA, (LONG)this);
		//this->edit_proc_ = (WNDPROC)::SetWindowLongPtr(this->edit_, GWLP_WNDPROC, (LONG)_editProc);
		::SendMessage(this->find_edit_, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));
		::SendMessage(this->rep_edit_, WM_SETFONT, WPARAM(hf), MAKELPARAM(TRUE, 0));

		Gdiplus::FontFamily ff(T("Consolas"));
		Gdiplus::Font* font = new Gdiplus::Font(&ff, 16, 
			Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

		for (int i = 0; i < sizeof(this->btns_) / sizeof(this->btns_[0]); ++i) {
			this->btns_[i].Init(i, this, this->hw_, hinst, font, i < 2, IDC_FINDBOX_PREV + i);
			this->btns_[i].SetBkColor(Gdiplus::Color::Brown);
		}

		return 0;
	}

	/*
	LRESULT CALLBACK RepBox::EdtProc(UINT message, WPARAM wParam, LPARAM lParam) {
		if (message == WM_KEYDOWN) {
			if (wParam == VK_RETURN) {
				tchar txt[1024];
				::GetWindowText(this->edit_, txt, 1024);
				this->txt_ = txt;
				this->view_->find_all(this->txt_, this->case_, this->ww_);
				::SetFocus(this->edit_);
				::SendMessage(this->edit_, EM_SETSEL, 0, -1);
				return 0;
			}
			if (wParam == VK_ESCAPE) {
				HWND hw = ::GetParent(this->hw_);
				for (;;) {
					HWND vw = ::GetWindow(hw, GW_CHILD);
					if (vw != this->hw_) {
						::SendMessage(vw, WM_KEYDOWN, VK_ESCAPE, 0);
						break;
					}
				}
				return 0;
			}
		}
		return ::CallWindowProc(this->edit_proc_, this->edit_, message, wParam, lParam);
	}
	*/

	void RepBox::Notify(int id) {
		switch(id) {
		case 0: // case
			this->view_->match_case_ = this->btns_[id].Check();
			break;
		case 1: // ww
			this->view_->match_whole_word_ = this->btns_[id].Check();
			break;
		case 2: // find prev
			this->view_->find_prev();
			break;
		case 3: // find next
			this->view_->find_next();
			break;
		case 4: // replace
			this->view_->replace_find();
			break;
		case 5: // replace all
			this->view_->replace_all();
			break;
		}
	}

	LRESULT CALLBACK RepBox::WndProc(UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) {
				case IDC_REPBOX_FIND_EDIT:
					if (HIWORD(wParam) == EN_CHANGE) {
						tchar txt[1024];
						::GetWindowText(this->find_edit_, txt, 1024);
						this->view_->find_txt_ = txt;
					}
					break;
				case IDC_REPBOX_REP_EDIT:
					if (HIWORD(wParam) == EN_CHANGE) {
						tchar txt[1024];
						::GetWindowText(this->rep_edit_, txt, 1024);
						this->view_->rep_txt_ = txt;
					}
					break;
				}
			}
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				return 0;
			}
			break;
		default:
			return ::CallWindowProc(this->repbox_proc_, this->hw_, message, wParam, lParam);
		}
		return 0;
	}

	void RepBox::Show(int x, int y, int w, int h) {
		::MoveWindow(this->hw_, x, y, w, h, TRUE);
		::ShowWindow(this->hw_, SW_SHOW);

		static int btn_width = 10 * 10;
		static int btn_hight = 10 * 2;
		static int left_interval = 10;
		static int top_inerval = 5;

		int cx = left_interval;
		int cy = top_inerval;
		int cy2 = top_inerval + btn_hight + top_inerval;

		this->btns_[0].SetText(T("Aa"), Gdiplus::Color::Red);
		this->btns_[0].Show(cx, cy, 30, btn_hight);//, btn_width, btn_hight); //, TRUE);
		
		this->btns_[1].SetText(T("����"), Gdiplus::Color::Red);
		this->btns_[1].Show(cx, cy2, 30, btn_hight);//, btn_width, btn_hight);
														
		cx += left_interval + 30;				
		int ew = w - (left_interval + btn_width) * 3;	
		::MoveWindow(this->find_edit_, cx, cy, ew, btn_hight+3, TRUE);
		::SetFocus(this->find_edit_);

		//ew = 10*64; //w - (left_interval + btn_width) * 5;	
		::MoveWindow(this->rep_edit_, cx, cy2, ew, btn_hight+3, TRUE);
		::SetFocus(this->rep_edit_);
														
		cx += left_interval + ew;						
		this->btns_[2].SetText(T("find prev"), Gdiplus::Color::Red);
		this->btns_[2].Show(cx, cy, btn_width - 10, btn_hight);

		this->btns_[3].SetText(T("find next"), Gdiplus::Color::Red);
		this->btns_[3].Show(cx, cy2, btn_width - 10, btn_hight);
														
		cx += left_interval + btn_width;				
		this->btns_[4].SetText(T("replace"), Gdiplus::Color::Red);
		this->btns_[4].Show(cx, cy, btn_width + 10, btn_hight);
														
		this->btns_[5].SetText(T("replace all"), Gdiplus::Color::Red);
		this->btns_[5].Show(cx, cy2, btn_width + 10, btn_hight);
		::UpdateWindow(this->hw_);
	}

	void RepBox::Hide() {
		::MoveWindow(this->hw_, 0, 0, 0, 0, TRUE);
		::ShowWindow(this->hw_, SW_HIDE);
	}
}