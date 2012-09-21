/*
  * tocken.h
  */

#ifndef W_TOKEN_H_
#define W_TOKEN_H_

#include "types.h"
 
namespace w {

	enum TokenType {
		// Special tokens
		ILLEGAL = 0,
		EOF,
		COMMENT,
		WHITESPACE,

		literal_beg,
		// Identifiers and basic type literals
		// (these tokens stand for classes of literals)
		IDENT,	// main
		INT,	// 12345
		FLOAT,	// 123.45
		IMAG,	// 123.45i
		CHAR,	// 'a'
		STRING, // "abc"
		literal_end,

		operator_beg,
		// Operators and delimiters
		OP_ADD, // +
		OP_SUB, // -
		OP_MUL, // *
		OP_QUO, // /
		OP_REM, // %

		OP_AND,	 // &
		OP_OR, 	 // |
		OP_NOT,  // ~
		//OP_XOR,	 // ^
		OP_SHL,	 // <<
		OP_SHR,	 // >>
		//AND_NOT, // &^

		//OP_ADD_ASSIGN, // +=
		//OP_SUB_ASSIGN, // -=
		//OP_MUL_ASSIGN, // *=
		//OP_QUO_ASSIGN, // /=
		//OP_REM_ASSIGN, // %=

		//OP_AND_ASSIGN, 	// &=
		//OP_OR_ASSIGN,	// |=
		//OP_XOR_ASSIGN, 	// ^=
		//OP_SHL_ASSIGN, 	// <<=
		//OP_SHR_ASSIGN, 	// >>=
		//OP_AND_NOT_ASSIGN, // &^=

		//OP_LAND,  // AND
		//OP_LOR,   // OR
		//OP_CAT,   // || "abc" || "def" == "abcdef"
		//OP_ARROW, // <-
		//OP_INC,   // ++
		//OP_DEC,   // --

		OP_EQL,	// == 
		OP_LSS,	// <
		OP_GTR,	// >
		OP_ASSIGN, // =
		//OP_NOT,	// !

		OP_NEQ,	  // != 
		OP_LEQ,	  // <=
		OP_GEQ,	  // >=
		//DEFINE,   // :=
		//ELLIPSIS, // ...

		LPAREN, // (
		LBRACK, // [
		LBRACE, // {
		COMMA,	// ,
		PERIOD, // .

		RPAREN,    // )
		RBRACK,    // ]
		RBRACE,    // }
		SEMICOLON, // ;
		COLON,	  // :
		operator_end,

		keyword_beg,
		// Keywords 121 for sqlite
		ABORT,
		ACTION,
		ADD,
		AFTER,
		ALL,
		ALTER,
		ANALYZE,
		AND,
		AS,
		ASC,
		ATTACH,
		AUTOINCREMENT,
		BEFORE,
		BEGIN,
		BETWEEN,
		BY,
		CASCADE,
		CASE,
		CAST,
		CHECK,
		COLLATE,
		COLUMN,
		COMMIT,
		CONFLICT,
		CONSTRAINT,
		CREATE,
		CROSS,
		CURRENT_DATE,
		CURRENT_TIME,
		CURRENT_TIMESTAMP,
		DATABASE,
		DEFAULT,
		DEFERRABLE,
		DEFERRED,
		DELETE,
		DESC,
		DETACH,
		DISTINCT,
		DROP,
		EACH,
		ELSE,
		END,
		ESCAPE,
		EXCEPT,
		EXCLUSIVE,
		EXISTS,
		EXPLAIN,
		FAIL,
		FOR,
		FOREIGN,
		FROM,
		FULL,
		GLOB,
		GROUP,
		HAVING,
		IF,
		IGNORE,
		IMMEDIATE,
		IN,
		INDEX,
		INDEXED,
		INITIALLY,
		INNER,
		INSERT,
		INSTEAD,
		INTERSECT,
		INTO,
		IS,
		ISNULL,
		JOIN,
		KEY,
		LEFT,
		LIKE,
		LIMIT,
		MATCH,
		NATURAL,
		NO,
		NOT,
		NOTNULL,
		NULL,
		OF,
		OFFSET,
		ON,
		OR,
		ORDER,
		OUTER,
		PLAN,
		PRAGMA,
		PRIMARY,
		QUERY,
		RAISE,
		REFERENCES,
		REGEXP,
		REINDEX,
		RELEASE,
		RENAME,
		REPLACE,
		RESTRICT,
		RIGHT,
		ROLLBACK,
		ROW,
		SAVEPOINT,
		SELECT,
		SET,
		TABLE,
		TEMP,
		TEMPORARY,
		THEN,
		TO,
		TRANSACTION,
		TRIGGER,
		UNION,
		UNIQUE,
		UPDATE,
		USING,
		VACUUM,
		VALUES,
		VIEW,
		VIRTUAL,
		WHEN,
		WHERE,		
		keyword_end
	};



	class Token {
	private:
	};
}

#endif // W_TOKEN_H_

