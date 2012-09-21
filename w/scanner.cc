/*
 * scanner.cc
 */

#include "scanner.h"
#include "util.h"

namespace w {

	static tchar* keywords[] = {
		T("ABORT"),			   T("COLLATE"),			T("ELSE"),			 T("INDEXED"),		  T("OF"),			   T("ROW"),
		T("ACTION"),		   T("COLUMN"),				T("END"),			 T("INITIALLY"),	  T("OFFSET"),		   T("SAVEPOINT"),
		T("ADD"),			   T("COMMIT"),				T("ESCAPE"),		 T("INNER"),		  T("ON"),			   T("SELECT"),
		T("AFTER"),			   T("CONFLICT"),			T("EXCEPT"),		 T("INSERT"),		  T("OR"),			   T("SET"),
		T("ALL"),			   T("CONSTRAINT"),			T("EXCLUSIVE"),		 T("INSTEAD"),		  T("ORDER"),		   T("TABLE"),
		T("ALTER"),			   T("CREATE"),				T("EXISTS"),		 T("INTERSECT"),	  T("OUTER"),		   T("TEMP"),
		T("ANALYZE"),		   T("CROSS"),				T("EXPLAIN"),		 T("INTO"),			  T("PLAN"),		   T("TEMPORARY"),
		T("AND"),			   T("CURRENT_DATE"),		T("FAIL"),			 T("IS"),			  T("PRAGMA"),		   T("THEN"),
		T("AS"),			   T("CURRENT_TIME"),		T("FOR"),			 T("ISNULL"),		  T("PRIMARY"),		   T("TO"),
		T("ASC"),			   T("CURRENT_TIMESTAMP"),	T("FOREIGN"),		 T("JOIN"),			  T("QUERY"),		   T("TRANSACTION"),
		T("ATTACH"),		   T("DATABASE"),			T("FROM"),			 T("KEY"),			  T("RAISE"),		   T("TRIGGER"),
		T("AUTOINCREMENT"),	   T("DEFAULT"),			T("FULL"),			 T("LEFT"),			  T("REFERENCES"),	   T("UNION"),
		T("BEFORE"),		   T("DEFERRABLE"),			T("GLOB"),			 T("LIKE"),			  T("REGEXP"),		   T("UNIQUE"),
		T("BEGIN"),			   T("DEFERRED"),			T("GROUP"),			 T("LIMIT"),		  T("REINDEX"),		   T("UPDATE"),
		T("BETWEEN"),		   T("DELETE"),				T("HAVING"),		 T("MATCH"),		  T("RELEASE"),		   T("USING"),
		T("BY"),			   T("DESC"),				T("IF"),			 T("NATURAL"),		  T("RENAME"),		   T("VACUUM"),
		T("CASCADE"),		   T("DETACH"),				T("IGNORE"),		 T("NO"),			  T("REPLACE"),		   T("VALUES"),
		T("CASE"),			   T("DISTINCT"),			T("IMMEDIATE"),		 T("NOT"),			  T("RESTRICT"),	   T("VIEW"),
		T("CAST"),			   T("DROP"),				T("IN"),			 T("NOTNULL"),		  T("RIGHT"),		   T("VIRTUAL"),
		T("CHECK"),			   T("EACH"),				T("INDEX"),			 T("NULL"),			  T("ROLLBACK"),	   T("WHEN"),
		T("WHERE")
	};	
		
	static tchar* ctrl_words[] = { // after "%"
		T("did_data"), 	T("if"), 		T("else"), 			T("end"), 	
		T("for_each"), 	T("for"), 		T("while"), 		T("continue"), 
		T("break"), 	T("exit"), 		T("notify_table"), 	T("call"), 
		T("info_list"), T("switch"), 	T("case"), 			T("default"), 
		T("stk_data"), 	T("ss_data"), 	T("submit")
	};
	
	static tchar* builtin_funcs[] = {
		T("abs"), T("avedev")		
	};

	static bool finded(const tstring& s, tchar* words[], int size, bool upper = false) {
		tstring ss(s);
		int len = s.length();
		for (int i = 0; i < len; ++i) {
			if (upper) {
				ss[i] = ::toupper(ss[i]);
			} else {
				ss[i] = ::tolower(s[i]);
			}
		}
		for (int i = 0; i < size; ++i) {
			if (ss == words[i]) {
				return true;
			}
		}
		return false;
	}

	static bool isKeyword(const tstring& s) {
		return finded(s, keywords, sizeof(keywords) / sizeof(keywords[0]), true);
	}

	static bool isBuiltinFunc(const tstring& s) {
		return finded(s, builtin_funcs, sizeof(builtin_funcs) / sizeof(builtin_funcs[0]));
	}
	
	static bool isCtrlwords(const tstring& s) {
		return finded(s, ctrl_words, sizeof(ctrl_words) / sizeof(ctrl_words[0]));
	}

	Scanner::Scanner(const tstring& txt) : txt_(txt), last_tk_(0) {
	}

	Scanner::~Scanner() {
	}

	int Scanner::Scan(std::list<Token*>& l) {
		this->size_ = this->txt_.length();
		if (this->size_ == 0) {
			return 0;
		}

		this->offset_ = 1;
		this->ch_ = this->txt_[0];
		this->line_offset_ = 0;

		for (Token* tk = this->scan(); tk && tk->type != EofType; tk = this->scan()) {
			l.push_back(tk);
		}

		return 0;
	}

	void Scanner::next() {
		if (this->offset_ >= this->size_) {
			this->ch_ = -1;
			//return;
		} else {
			this->ch_ = this->txt_[this->offset_];
		}
		++this->offset_;
	}

	tstring Scanner::scanComment() {
		int offset = this->offset_;
		tchar ch = this->ch_;
		if (ch == T('/')) {
			this->next();
			for(; this->ch_ != T('\n') && this->ch_ > 0;) {
				this->next();
			}
			return this->txt_.substr(offset, this->offset_ - offset);
		}

		this->next();
		for (; this->ch_ > 0; ) {
			tchar ch = this->ch_;
			this->next();
			if (this->ch_ == '*' && ch == '/') {
				return this->txt_.substr(offset, this->offset_ - offset - 1);		
			}
		}

		return tstring();
	}

	tstring Scanner::scanIdent() {
		int offset = this->offset_ - 1;
		for (;is_letter(this->ch_) || is_digit(this->ch_);) {
			this->next();
		}
		return this->txt_.substr(offset, this->offset_ - offset - 1);
	}

	tstring Scanner::scanNumber() {
		int offset = this->offset_ - 1;

		for (tchar ch = this->ch_; is_digit(ch) || 
			(ch == T('.') && is_digit(this->ch_)); this->next()) {
				ch = this->ch_;
		}
		return this->txt_.substr(offset, this->offset_ - offset - 1);
	}

	tstring Scanner::scanString() {
		int offset = this->offset_ - 1;
		tchar ch = this->ch_;
		for (this->next(); this->ch_ != T('"') && this->ch_ != -1; this->next()) { }
		this->next();
		return this->txt_.substr(offset, this->offset_ - offset - 1);
	}

	tstring Scanner::scanRawString() {
		int offset = this->offset_ - 1;
		tchar ch = this->ch_;
		for (this->next(); this->ch_ != T('"') && this->ch_ != -1; this->next()) { }
		this->next();
		return this->txt_.substr(offset, this->offset_ - offset - 1);
	}

	tstring Scanner::scanWhitespace() {
		int offset = this->offset_ - 1;
		for (;(this->ch_ == T(' ') || this->ch_ == T('\t'));) {
			this->next();
		}
		return this->txt_.substr(offset, this->offset_ - offset - 1);
	}

	tstring Scanner::scanVar() {
		tstring s;
		s.push_back(this->ch_);

		this->next();
		if (is_letter(this->ch_)) {
			s += this->scanIdent();
			return s;
		}
		return s;
	}

	void Scanner::scanCtrl(Token* tk) {
		tstring s;
		s.push_back(this->ch_);

		this->next();
		if (is_letter(this->ch_)) {
			tstring ss(this->scanIdent());
			if (isCtrlwords(ss)) {
				tk->type = CtrlType;
			} else {
				tk->type = IdentType;
			}
			s += ss;
		} else {
			tk->type = OperatorType;
		}
		tk->txt = s;
	}

	Token* Scanner::scan() {
		int ch = this->ch_;
		if (ch == T('\n') || ch == T('\r')) {
			return NULL;
		}

		Token* tk = new Token();
		tk->pos = this->offset_ - 1;
		tk->pos < 0 ? 0 : tk->pos;
		if(ch == -1) {
			tk->type = EofType;
			return tk;
		}

		if (is_letter(ch)) {
			tk->txt = this->scanIdent();
			tk->type = IdentType;
			if (isKeyword(tk->txt)) {
				tk->type = KewordType;
			}
			last_tk_ = tk;
			return tk;
		}

		if (is_digit(ch)) {		
			tk->txt = this->scanNumber();
			tk->type = NumberType;
			return tk;
		}

		switch(ch) {
		case -1:
			tk->type = EofType;
			break;
		case T(' '):
		case T('\t'):
			tk->txt = this->scanWhitespace();
			tk->type = WhitespaceType;
			break;
		case T('"'):
			tk->txt = this->scanString();
			tk->type = StringType;
			break;
		case T('`'):
			tk->txt = this->scanRawString();
			tk->type = StringType;
			break;
		case T('@'):
		case T('$'):
			{
				tk->txt = this->scanVar();
				if (tk->txt.length() > 1) { // var
					tk->type = VariableType;
				} else {
					tk->type = OperatorType;
				}
			}
			break;
		case T('%'):
			{
				this->scanCtrl(tk);
			}
			break;
		case T('('):
			if (last_tk_ != NULL && last_tk_->type == IdentType) {
				last_tk_->type = FuncType;
			}
			//break;
		case T(')'):
			//break;
		case T('['):
			//break;
		case T(']'):
			//break;
		case T('{'):
			//break;
		case T('}'):
		case T(':'):
			//break;
		case T('.'):
			//break;
		case T(','):
			//break;
		case T(';'):
			//break;
		case T('+'):
			//break;
		case T('-'):
			//break;
		case T('*'):
			//break;
		//case T('%'):
			//break;
		case T('^'):
			//break;
		case T('>'):
			//break;
		case T('<'):
			//break;
		case T('='):
			//break;
		case T('!'):
			//break;
		case T('&'):
			//break;
		case T('|'):
		default:
			//break;
			tk->txt.push_back(ch);
			tk->type = OperatorType;
			this->next();
			break;
		}
		return tk;
	}

}