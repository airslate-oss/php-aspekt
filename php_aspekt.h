/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef ASPEKT_H
#define ASPEKT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Zend/zend_API.h>
#include <Zend/zend_string.h>
#include <ext/pcre/php_pcre.h>

/* Working with scopes */
#if PHP_VERSION_ID >= 70100
# define aspekt_get_scope(e) ((e) ? zend_get_executed_scope() : EG(fake_scope))
# define aspekt_set_scope(s) EG(fake_scope) = (s)
#else
# define aspekt_get_scope(e) EG(scope)
# define aspekt_set_scope(s) EG(scope) = (s)
#endif

/* Backwards compatibility for GC API change in PHP 7.3 */
#if PHP_VERSION_ID < 70300
# define GC_ADDREF(p)            ++GC_REFCOUNT(p)
# define GC_DELREF(p)            --GC_REFCOUNT(p)
# define GC_SET_REFCOUNT(p, rc)  GC_REFCOUNT(p) = rc
#endif

/* Kept for compatibility */
#if PHP_VERSION_ID < 70100
# define zval_dtor_func(f) zval_dtor_func_for_ptr(f)
#endif
#if PHP_VERSION_ID >= 70200
# define GC_ZVAL_CHECK_POSSIBLE_ROOT(z) gc_check_possible_root(Z_COUNTED_P(z))
#endif

extern zend_module_entry aspekt_module_entry;
#define phpext_aspekt_ptr &aspekt_module_entry

/* {{{ Extension name */
#define ASPEKT_NAME "aspekt"
#define ASPEKT_VERSION "1.2.1"
#define ASPEKT_AUTHOR "Serghei Iakovlev <egrep@protonmail.ch>"
#define ASPEKT_DESCRIPTION \
	"A modern aspect-oriented PHP extension with rich features for the new level of software development."
/* }}} */

#ifdef PHP_WIN32
#	define ASPEKT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define ASPEKT_API __attribute__ ((visibility("default")))
#elif defined(PHPAPI)
#	define ASPEKT_API PHPAPI
#else
#	define ASPEKT_API
#endif

#ifdef ZTS
# include <TSRM.h>
#endif

#define ASPEKT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(aspekt, v)

/* {{{ Constants */
#define ASPEKT_KIND_AROUND 1
#define ASPEKT_KIND_BEFORE 2
#define ASPEKT_KIND_AFTER  4
#define ASPEKT_KIND_READ 8
#define ASPEKT_KIND_WRITE 16
#define ASPEKT_KIND_PROPERTY 32
#define ASPEKT_KIND_METHOD 64
#define ASPEKT_KIND_FUNCTION 128
#define ASPEKT_KIND_CATCH 256
#define ASPEKT_KIND_RETURN 512

#define ASPEKT_KIND_AROUND_READ_PROPERTY (ASPEKT_KIND_AROUND + ASPEKT_KIND_READ + ASPEKT_KIND_PROPERTY)
#define ASPEKT_KIND_AROUND_WRITE_PROPERTY (ASPEKT_KIND_AROUND + ASPEKT_KIND_WRITE + ASPEKT_KIND_PROPERTY)
#define ASPEKT_KIND_BEFORE_READ_PROPERTY (ASPEKT_KIND_BEFORE + ASPEKT_KIND_READ + ASPEKT_KIND_PROPERTY)
#define ASPEKT_KIND_BEFORE_WRITE_PROPERTY (ASPEKT_KIND_BEFORE + ASPEKT_KIND_WRITE + ASPEKT_KIND_PROPERTY)
#define ASPEKT_KIND_AFTER_READ_PROPERTY (ASPEKT_KIND_AFTER + ASPEKT_KIND_READ + ASPEKT_KIND_PROPERTY)
#define ASPEKT_KIND_AFTER_WRITE_PROPERTY (ASPEKT_KIND_AFTER + ASPEKT_KIND_WRITE + ASPEKT_KIND_PROPERTY)

#define ASPEKT_KIND_AROUND_METHOD (ASPEKT_KIND_AROUND + ASPEKT_KIND_METHOD)
#define ASPEKT_KIND_AROUND_FUNCTION (ASPEKT_KIND_AROUND + ASPEKT_KIND_FUNCTION)
#define ASPEKT_KIND_BEFORE_METHOD (ASPEKT_KIND_BEFORE + ASPEKT_KIND_METHOD)
#define ASPEKT_KIND_BEFORE_FUNCTION (ASPEKT_KIND_BEFORE + ASPEKT_KIND_FUNCTION)
#define ASPEKT_KIND_AFTER_METHOD (ASPEKT_KIND_AFTER + ASPEKT_KIND_METHOD)
#define ASPEKT_KIND_AFTER_FUNCTION (ASPEKT_KIND_AFTER + ASPEKT_KIND_FUNCTION)
/* }}} */

typedef struct { /* {{{ */
	int                    scope;
	int                    static_state;

	zend_string           *class_name;
	int                    class_jok;

	zend_string           *method;
	int                    method_jok;

	zend_string           *selector;
	int                    kind_of_advice;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fci_cache;

#if PHP_VERSION_ID < 70300
	pcre                  *re_method;
	pcre                  *re_class;
#else
	pcre2_code            *re_method;
	pcre2_code            *re_class;
#endif
} pointcut;
/* }}} */

typedef struct { /* {{{ */
	zend_array       *ht;
	int               version;
	zend_class_entry *ce;
} pointcut_cache;
/* }}} */

typedef struct { /* {{{ */
	zend_array *read;
	zend_array *write;
	zend_array *func;
} object_cache;
/* }}} */

ZEND_BEGIN_MODULE_GLOBALS(aspekt) /* {{{  */
	zend_bool       aspekt_enable;
	zend_array     *pointcuts_table;
	int             pointcut_version;
	int             overloaded;

	zend_array    *function_cache;

	object_cache **object_cache;
	int            object_cache_size;

	int            lock_read_property;
	int            lock_write_property;

	zval          *property_value;
ZEND_END_MODULE_GLOBALS(aspekt)
/* }}} */

int aspekt_declare_class_constant(zend_class_entry *ce, const char *name, size_t name_length, zval *value);
int aspekt_declare_class_constant_long(zend_class_entry *ce, const char *name, size_t name_length, zend_long value);

void free_pointcut_cache(zval *elem);

/* {{{ ASPEKT_REGISTER_CLASS_EX */
#define ASPEKT_INIT_CLASS(name) \
	int aspekt_ ##name## _init(INIT_FUNC_ARGS)
/* }}} */

/* {{{ ASPEKT_REGISTER_CLASS
 class/interface registering */
#define ASPEKT_REGISTER_CLASS(ns, cl, lns, n, m, f)                    \
	{                                                                  \
		zend_class_entry ce;                                           \
		memset(&ce, 0, sizeof(zend_class_entry));                      \
		INIT_NS_CLASS_ENTRY(ce, #ns, #cl, m);                          \
		lns## _ ##n## _ce_ptr = zend_register_internal_class(&ce);     \
		if (UNEXPECTED(!lns## _ ##n## _ce_ptr)) {                      \
			zend_error(E_ERROR, "Class '%s' registration has failed.", \
				ZEND_NS_NAME(#ns, #cl));                               \
			return FAILURE;                                            \
		}                                                              \
		lns## _ ##n## _ce_ptr->ce_flags |= f;                          \
	}
/* }}} */

/* {{{ ASPEKT_INIT */
#define ASPEKT_INIT(name)                                             \
	if (aspekt_ ##name## _init(INIT_FUNC_ARGS_PASSTHRU) == FAILURE) { \
		return FAILURE;                                               \
	}
/* }}} */

/* {{{ ASPEKT_INIT_THIS */
#define ASPEKT_INIT_THIS()                     \
	zval this_zv;                              \
	zval *this_ptr = getThis();                \
	if (EXPECTED(this_ptr)) {                  \
		ZVAL_OBJ(&this_zv, Z_OBJ_P(this_ptr)); \
		this_ptr = &this_zv;                   \
	} else {                                   \
		ZVAL_NULL(&this_zv);                   \
		this_ptr = &this_zv;                   \
	}
/* }}} */

extern ZEND_DECLARE_MODULE_GLOBALS(aspekt);

#if defined(ZTS) && defined(COMPILE_DL_ASPEKT)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif // ASPEKT_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
