/* lexer.h
 *
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef ASPEKT_LEXER_H
#define ASPEKT_LEXER_H

#include <php.h>

/* {{{ Scanner state */
#define SCANNER_RETCODE_EOF -1
#define SCANNER_RETCODE_ERR -2
#define SCANNER_RETCODE_IMPOSSIBLE -3
/* }}} */

/* {{{ Tokens */
#define TOKEN_SPACE 0
#define TOKEN_FUNCTION 1
#define TOKEN_CLASS 2
#define TOKEN_JOKER 3
#define TOKEN_SUPER_JOKER 4
#define TOKEN_PROPERTY 5
#define TOKEN_SCOPE 6
#define TOKEN_STATIC 7
#define TOKEN_OR 8
#define TOKEN_TEXT 9
/* }}} */

typedef struct _scanner_state { /* {{{ */
	char* start;
	char* end;
	char* marker;
	int is_class;
} scanner_state;
/* }}} */

typedef struct _scanner_token { /* {{{ */
	int TOKEN;
	char *str_val;
	int int_val;
} scanner_token;
/* }}} */

int scan(scanner_state *state, scanner_token *token);

/* {{{ YYDEBUG */
/* The YYDEBUG macro is designed to produce of trace information,
 * that will be written on stderr.
 *
 * To enable this feature just export ASPEKT_YYDEBUG environment
 * variable with the value of 1.
 */

#ifdef YYDEBUG
#undef YYDEBUG
#endif

#define YYDEBUG(s, c) do {                                 \
		char *tmp;                                         \
		tmp = getenv("ASPEKT_YYDEBUG");                    \
		if (tmp && zend_atoi(tmp, 1)) {                    \
			fprintf(stderr, "State: %d char: %c\n", s, c); \
		}                                                  \
	} while(0);
/* }}} */

#endif // ASPEKT_LEXER_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
