/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include <php.h>
#include <Zend/zend_types.h>
#include <Zend/zend_objects.h>
#include <ext/standard/php_string.h>
#include <ext/pcre/php_pcre.h>

#include "../php_aspekt.h"
#include "lexer.h"
#include "interceptor.h"

zend_object_handlers aspekt_interceptor_handlers;
ASPEKT_API zend_class_entry *aspekt_interceptor_ce_ptr;

static zend_always_inline aspekt_interceptor_object_t *fetch_object(zend_object *obj) /* {{{ */
{
	return (aspekt_interceptor_object_t *)((char *)(obj) - XtOffsetOf(aspekt_interceptor_object_t, std));
}
/* }}} */

static zend_object *create_object(zend_class_entry *ce_ptr) /* {{{ */
{
	aspekt_interceptor_object_t *intern;

	intern = ecalloc(1, sizeof(aspekt_interceptor_object_t) + zend_object_properties_size(ce_ptr));

	zend_object_std_init(&intern->std, ce_ptr);
	intern->std.handlers = &aspekt_interceptor_handlers;

	return &intern->std;
}
/* }}} */

static void object_free(zend_object *object) /* {{{ */
{
	aspekt_interceptor_object_t *intern = fetch_object(object);
	zend_object_std_dtor(&intern->std);
}
/* }}} */

/* {{{ aspekt_Interceptor_init
   Create and register 'Aspekt\Interceptor' class. */
ASPEKT_INIT_CLASS(Interceptor) {
	ASPEKT_REGISTER_CLASS(Aspekt, Interceptor, aspekt, interceptor, aspekt_interceptor_method_entry, 0);

	aspekt_interceptor_ce_ptr->create_object = create_object;

	memcpy(&aspekt_interceptor_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* offset of real object header (usually zero) */
	aspekt_interceptor_handlers.offset = (int) XtOffsetOf(aspekt_interceptor_object_t, std);

	/* general object functions */
	aspekt_interceptor_handlers.free_obj  = object_free;
	aspekt_interceptor_handlers.dtor_obj  = zend_objects_destroy_object;
	aspekt_interceptor_handlers.clone_obj = NULL; /* has no clone implementation */

	return SUCCESS;
}
/* }}} */

static pointcut *alloc_pointcut() /* {{{ */
{
	pointcut *pc = (pointcut *)emalloc(sizeof(pointcut));

	pc->scope = 0;
	pc->static_state = 2;
	pc->method_jok = 0;
	pc->class_jok = 0;
	pc->class_name = NULL;
	pc->method = NULL;
	pc->selector = NULL;
	pc->kind_of_advice = 0;

	// TODO: ???
	// pc->fci = NULL;
	// pc->fcic = NULL;

	pc->re_method = NULL;
	pc->re_class = NULL;

	return pc;
}
/* }}} */

static void make_regexp_on_pointcut(pointcut *pc) /* {{{ */
{
	if (pc->method == NULL && pc->class_name == NULL) {
		/* nothng to do */
		return;
	}

#if PHP_VERSION_ID >= 70400
	uint32_t *pcre_extra = NULL;
#elif PHP_VERSION_ID >= 70300
	uint32_t *pcre_extra = NULL;
	uint32_t preg_options = 0;
#else
	pcre_extra *pcre_extra = NULL;
	int preg_options = 0;
#endif
	zend_string *regexp;
	zend_string *regexp_buffer = NULL;
	zend_string *regexp_tmp = NULL;
	char tempregexp[500];

	pc->method_jok = (pc->method != NULL && strchr(ZSTR_VAL(pc->method), '*') != NULL);

	regexp_buffer = php_str_to_str(ZSTR_VAL(pc->method), ZSTR_LEN(pc->method), "**\\", 3, "[.#}", 4);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "**", 2, "[.#]", 4);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "\\", 1, "\\\\", 2);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "*", 1, "[^\\\\]*", 6);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#]", 4, ".*", 2);
	zend_string_release(regexp_tmp);

	regexp_tmp = regexp_buffer;
	regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#}", 4, "(.*\\\\)?", 7);
	zend_string_release(regexp_tmp);

	if (ZSTR_VAL(regexp_buffer)[0] != '\\') {
		sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer));
	} else {
		sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer) + 2);
	}
	zend_string_release(regexp_buffer);

	regexp = zend_string_init(tempregexp, strlen(tempregexp), 0);
#if PHP_VERSION_ID >= 70400
	pc->re_method = pcre_get_compiled_regex(regexp, pcre_extra);
#elif PHP_VERSION_ID >= 70300
	pc->re_method = pcre_get_compiled_regex(regexp, pcre_extra, &preg_options);
#else
	pc->re_method = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options);
#endif
	zend_string_release(regexp);

	if (!pc->re_method) {
		php_error_docref(NULL, E_WARNING, "Invalid regular expression to match function");
	}

	if (pc->class_name != NULL) {
		regexp_buffer = php_str_to_str(ZSTR_VAL(pc->class_name), ZSTR_LEN(pc->class_name), "**\\", 3, "[.#}", 4);

		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "**", 2, "[.#]", 4);
		zend_string_release(regexp_tmp);

		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "\\", 1, "\\\\", 2);
		zend_string_release(regexp_tmp);

		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "*", 1, "[^\\\\]*", 6);
		zend_string_release(regexp_tmp);

		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#]", 4, ".*", 2);
		zend_string_release(regexp_tmp);

		regexp_tmp = regexp_buffer;
		regexp_buffer = php_str_to_str(ZSTR_VAL(regexp_tmp), ZSTR_LEN(regexp_buffer), "[.#}", 4, "(.*\\\\)?", 7);
		zend_string_release(regexp_tmp);

		if (ZSTR_VAL(regexp_buffer)[0] != '\\') {
			sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer));
		} else {
			sprintf((char *)tempregexp, "/^%s$/i", ZSTR_VAL(regexp_buffer) + 2);
		}
		zend_string_release(regexp_buffer);

		regexp = zend_string_init(tempregexp, strlen(tempregexp), 0);

#if PHP_VERSION_ID >= 70400
		pc->re_class = pcre_get_compiled_regex(regexp, pcre_extra);
#elif PHP_VERSION_ID >= 70300
		pc->re_class = pcre_get_compiled_regex(regexp, pcre_extra, &preg_options);
#else
		pc->re_class = pcre_get_compiled_regex(regexp, &pcre_extra, &preg_options);
#endif
		zend_string_release(regexp);

		if (!pc->re_class) {
			php_error_docref(NULL, E_WARNING, "regular expression to match class");
		}
	}
}
/* }}} */

static void add_pointcut(zend_fcall_info fci, zend_fcall_info_cache fci_cache, zend_string *selector, int cut_type) /* {{{ */
{
	zval pointcut_val;
	pointcut *pc = NULL;
	char *temp_str = NULL;
	int is_class = 0;
	scanner_state *state;
	scanner_token *token;

	if (ZSTR_LEN(selector) < 2) {
		zend_error(E_ERROR,
				   "The given pointcut is invalid. You must specify a function call, a method call or a property operation");
	}

	pc = alloc_pointcut();
	pc->selector = selector;
	pc->fci = fci;
	pc->fci_cache = fci_cache;
	pc->kind_of_advice = cut_type;

	state = (scanner_state *)emalloc(sizeof(scanner_state));
	token = (scanner_token *)emalloc(sizeof(scanner_token));

	state->start = ZSTR_VAL(selector);
	state->end = state->start;
	state->is_class = 0;

	// TODO: Move this to the separated parser
	while(0 <= scan(state, token)) {
		switch (token->TOKEN) {
			case TOKEN_STATIC:
				pc->static_state = token->int_val;
				break;

			case TOKEN_SCOPE:
				pc->scope |= token->int_val;
				break;

			case TOKEN_CLASS:
				pc->class_name = zend_string_init(temp_str, strlen(temp_str), 0);
				efree(temp_str);
				temp_str = NULL;
				is_class = 1;
				break;

			case TOKEN_PROPERTY:
				pc->kind_of_advice |= ASPEKT_KIND_PROPERTY | token->int_val;
				break;

			case TOKEN_FUNCTION:
				if (is_class) {
					pc->kind_of_advice |= ASPEKT_KIND_METHOD;
				} else {
					pc->kind_of_advice |= ASPEKT_KIND_FUNCTION;
				}
				break;

			case TOKEN_TEXT:
				if (temp_str != NULL) {
					efree(temp_str);
				}
				temp_str = estrdup(token->str_val);
				efree(token->str_val);
				break;

			default:
				break;
		}
	}

	if (temp_str != NULL) {
		pc->method = zend_string_init(temp_str, strlen(temp_str), 0);
		efree(temp_str);
	}

	efree(state);
	efree(token);

	if (pc->kind_of_advice == cut_type) {
		pc->kind_of_advice |= ASPEKT_KIND_READ | ASPEKT_KIND_WRITE | ASPEKT_KIND_PROPERTY;
	}

	make_regexp_on_pointcut(pc);

	ZVAL_PTR(&pointcut_val, pc);
	zend_hash_next_index_insert(ASPEKT_G(pointcuts_table), &pointcut_val);
	ASPEKT_G(pointcut_version)++;
}
/* }}} */

/* {{{ proto void Aspekt\Interceptor::addAround(string selector, mixed pointcut)
   Adds an advice that will be called around real method or function. */
PHP_METHOD(AspektInterceptor, addAround)
{
	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a string for the pointcut as a first argument and a callback as a second argument",
		   "Aspekt\\Interceptor::addAround"
		);
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}

	add_pointcut(fci, fci_cache, selector, ASPEKT_KIND_AROUND);
}
/* }}} */

/* {{{ proto void Aspekt\Interceptor::addBefore(string selector, mixed pointcut)
   Adds an advice that will be called before real method or function. */
PHP_METHOD(AspektInterceptor, addBefore)
{
	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a string for the pointcut as a first argument and a callback as a second argument",
		   "Aspekt\\Interceptor::addBefore"
		);
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}

	add_pointcut(fci, fci_cache, selector, ASPEKT_KIND_BEFORE);
}
/* }}} */

/* {{{ proto void Aspekt\Interceptor::addAfter(string selector, mixed pointcut)
   Adds an advice that will be called after real method or function. */
PHP_METHOD(AspektInterceptor, addAfter)
{
	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a string for the pointcut as a first argument and a callback as a second argument",
		   "Aspekt\\Interceptor::addAfter"
		);
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}

	add_pointcut(fci, fci_cache, selector, ASPEKT_KIND_AFTER | ASPEKT_KIND_CATCH | ASPEKT_KIND_RETURN);
}
/* }}} */

/* {{{ proto void Aspekt\Interceptor::addAfterReturning(string selector, mixed pointcut)
   Adds an advice that will be called after return from a real method or function. */
PHP_METHOD(AspektInterceptor, addAfterReturning)
{
	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a string for the pointcut as a first argument and a callback as a second argument",
		   "Aspekt\\Interceptor::addAfterReturning"
		);
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}

	add_pointcut(fci, fci_cache, selector, ASPEKT_KIND_AFTER | ASPEKT_KIND_RETURN);
}
/* }}} */

/* {{{ proto void Aspekt\Interceptor::addAfterThrowing(string selector, mixed pointcut)
   Adds an advice that will be called after throwing an exception from a real method or function. */
PHP_METHOD(AspektInterceptor, addAfterThrowing)
{
	zend_string *selector;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_STR(selector)
		Z_PARAM_FUNC(fci, fci_cache)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a string for the pointcut as a first argument and a callback as a second argument",
		   "Aspekt\\Interceptor::addAfterThrowing"
		);
		return;
	);

	if (&(fci.function_name)) {
		Z_TRY_ADDREF(fci.function_name);
	}

	add_pointcut(fci, fci_cache, selector, ASPEKT_KIND_AFTER | ASPEKT_KIND_CATCH);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
