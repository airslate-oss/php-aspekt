/* lexer.re
 *
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#include "lexer.h"
#include "../php_aspekt.h"

// for re2c
#define YYCTYPE char
#define YYCURSOR (s->start)
#define YYLIMIT (s->end)
#define YYMARKER (s->marker)

int scan(scanner_state *s, scanner_token *t) {

	char *start = YYCURSOR;
	int status = SCANNER_RETCODE_IMPOSSIBLE;

	while(SCANNER_RETCODE_IMPOSSIBLE == status) {

		/*!re2c
		re2c:indent:top = 2;
		re2c:yyfill:enable = 0;

		'()' {
			t->TOKEN = TOKEN_FUNCTION;
			return 0;
		}

		'->' {
			t->TOKEN = TOKEN_CLASS;
			s->is_class = 1;
			return 0;
		}

		'::' {
			t->TOKEN = TOKEN_CLASS;
			s->is_class = 1;
			return 0;
		}

		'read' {
			// The workaround for "Foo::read" and "Foo::write"
			if (s->is_class) goto identifier;

			t->TOKEN = TOKEN_PROPERTY;
			t->int_val = ASPEKT_KIND_READ;
			s->is_class = 0;
			return 0;
		}

		'write' {
			// The workaround for "Foo::read" and "Foo::write"
			if (s->is_class) goto identifier;

			t->TOKEN = TOKEN_PROPERTY;
			t->int_val = ASPEKT_KIND_WRITE;
			s->is_class = 0;
			return 0;
		}

		'public' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val = ZEND_ACC_PUBLIC;
			return 0;
		}

		'protected' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val = ZEND_ACC_PROTECTED;
			return 0;
		}

		'private' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val = ZEND_ACC_PRIVATE;
			return 0;
		}

		'static' {
			t->TOKEN = TOKEN_STATIC;
			t->int_val = 1;
			return 0;
		}

		"|" {
			t->TOKEN = TOKEN_OR;
			return 0;
		}

		'!public' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val  = ZEND_ACC_PROTECTED | ZEND_ACC_PRIVATE;
			return 0;
		}

		'!protected' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val  = ZEND_ACC_PUBLIC | ZEND_ACC_PRIVATE;
			return 0;
		}

		'!private' {
			t->TOKEN = TOKEN_SCOPE;
			t->int_val  = ZEND_ACC_PUBLIC | ZEND_ACC_PROTECTED;
			return 0;
		}

		'!static' {
			t->TOKEN = TOKEN_STATIC;
			t->int_val = 0;
			return 0;
		}

		LABEL = [a-zA-Z0-9_\x7f-\xff\\*]*;
		LABEL {
identifier:
			t->str_val = estrndup(start, YYCURSOR - start);
			t->TOKEN = TOKEN_TEXT;
			return 0;
		}

		SPACE = [ \t$]*;
		SPACE {
			t->TOKEN = TOKEN_SPACE;
			return 0;
		}

		"\000" {
			status = SCANNER_RETCODE_EOF;
			break;
		}

		* {
			status = SCANNER_RETCODE_ERR;
			break;
		}
		*/
	}

	return status;
}
