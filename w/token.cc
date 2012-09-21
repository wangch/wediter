/*
 * token.cc
 */

#include "token.h"
#include "types.h"

namespace w {

	static tchar* keywords[] = {
		T("ABORT"),			   T("COLLATE"),			T("ELSE"),			 T("INDEXED"),		  T("OF"),
		T("ACTION"),		   T("COLUMN"),				T("END"),			 T("INITIALLY"),	  T("OFFSET"),
		T("ADD"),			   T("COMMIT"),				T("ESCAPE"),		 T("INNER"),		  T("ON"),
		T("AFTER"),			   T("CONFLICT"),			T("EXCEPT"),		 T("INSERT"),		  T("OR"),
		T("ALL"),			   T("CONSTRAINT"),			T("EXCLUSIVE"),		 T("INSTEAD"),		  T("ORDER"),
		T("ALTER"),			   T("CREATE"),				T("EXISTS"),		 T("INTERSECT"),	  T("OUTER"),
		T("ANALYZE"),		   T("CROSS"),				T("EXPLAIN"),		 T("INTO"),			  T("PLAN"),
		T("AND"),			   T("CURRENT_DATE"),		T("FAIL"),			 T("IS"),			  T("PRAGMA"),
		T("AS"),			   T("CURRENT_TIME"),		T("FOR"),			 T("ISNULL"),		  T("PRIMARY"),
		T("ASC"),			   T("CURRENT_TIMESTAMP"),	T("FOREIGN"),		 T("JOIN"),			  T("QUERY"),
		T("ATTACH"),		   T("DATABASE"),			T("FROM"),			 T("KEY"),			  T("RAISE"),
		T("AUTOINCREMENT"),	   T("DEFAULT"),			T("FULL"),			 T("LEFT"),			  T("REFERENCES"),
		T("BEFORE"),		   T("DEFERRABLE"),			T("GLOB"),			 T("LIKE"),			  T("REGEXP"),
		T("BEGIN"),			   T("DEFERRED"),			T("GROUP"),			 T("LIMIT"),		  T("REINDEX"),
		T("BETWEEN"),		   T("DELETE"),				T("HAVING"),		 T("MATCH"),		  T("RELEASE"),
		T("BY"),			   T("DESC"),				T("IF"),			 T("NATURAL"),		  T("RENAME"),
		T("CASCADE"),		   T("DETACH"),				T("IGNORE"),		 T("NO"),			  T("REPLACE"),
		T("CASE"),			   T("DISTINCT"),			T("IMMEDIATE"),		 T("NOT"),			  T("RESTRICT"),
		T("CAST"),			   T("DROP"),				T("IN"),			 T("NOTNULL"),		  T("RIGHT"),
		T("CHECK"),			   T("EACH"),				T("INDEX"),			 T("NULL"),			  T("ROLLBACK")
	};

	static tchar* operators[] = {
		T("+"), T("-"), T("*"), T("/"), T("%"), T("<<"), T(">>"), T("&"), T("|"), 
		T("<"), T("<="), T(">"), T(">="), T("="), T("=="), T("!="), T("<>"), T("~"), 
		T("("), T(")"), T("["), T("]"), T("{"), T("}"), T(","), T(";")
	};

	static tchar* tt_keyword(TokenType tt) {
		if (tt > keyword_beg && tt < keyword_end) {
			return keywords[tt];
		}
		return 0;
	}

} // namespace w
