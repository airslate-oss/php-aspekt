/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <php.h>
#include <main/php_ini.h>
#include <ext/standard/info.h>
#include <ext/pcre/php_pcre.h>
#include <Zend/zend_operators.h>

#include "php_aspekt.h"
#include "aspekt/execute.h"
#include "aspekt/interceptor.h"
#include "aspekt/joinpoint.h"
#include "aspekt/kind.h"

ZEND_DECLARE_MODULE_GLOBALS(aspekt)

int aspekt_declare_class_constant(zend_class_entry *ce, const char *name, size_t length, zval *value) /* {{{ */
{
#if PHP_VERSION_ID >= 70100
	int ret;

	zend_string *key = zend_string_init(name, length, ce->type & ZEND_INTERNAL_CLASS);
	ret = zend_declare_class_constant_ex(ce, key, value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(key);
	return ret;
#else
	if (Z_CONSTANT_P(value)) {
		ce->ce_flags &= ~ZEND_ACC_CONSTANTS_UPDATED;
	}
	ZVAL_NEW_PERSISTENT_REF(value, value);
	return zend_hash_str_update(&ce->constants_table, name, length, value) ?
		SUCCESS : FAILURE;
#endif
}
/* }}} */

int aspekt_declare_class_constant_long(zend_class_entry *ce, const char *name, size_t length, zend_long value) /* {{{ */
{
	zval constant;
	ZVAL_LONG(&constant, value);

	return aspekt_declare_class_constant(ce, name, length, &constant);
}
/* }}} */

static void free_pointcut(zval *elem) /* {{{ */
{
	pointcut *pc = (pointcut *)Z_PTR_P(elem);

	if (pc == NULL) {
		return; // LCOV_EXCL_LINE
	}

	if (&(pc->fci.function_name)) {
		zval_ptr_dtor(&pc->fci.function_name);
	}

	if (pc->method != NULL) {
		zend_string_release(pc->method);
	}
	if (pc->class_name != NULL) {
		zend_string_release(pc->class_name);
	}

	efree(pc);
}
/* }}} */

void free_pointcut_cache(zval *elem) /* {{{ */
{
	pointcut_cache *cache = (pointcut_cache *)Z_PTR_P(elem);

	if (cache->ht != NULL) {
		zend_hash_destroy(cache->ht);
		FREE_HASHTABLE(cache->ht);
	}

	efree(cache);
}
/* }}} */

/* {{{ INI Settings */
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("aspekt.enable", "1", PHP_INI_ALL,
		OnUpdateBool, aspekt_enable, zend_aspekt_globals, aspekt_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(aspekt)
{
	REGISTER_INI_ENTRIES();

	// 1. overload zend_execute_ex and zend_execute_internal
	original_zend_execute_ex = zend_execute_ex;
	zend_execute_ex = aspekt_execute_ex;

	original_zend_execute_internal = zend_execute_internal;
	zend_execute_internal = aspekt_execute_internal;

	// 2.overload zend_std_read_property and zend_std_write_property
	//original_zend_std_read_property = std_object_handlers.read_property;
	//std_object_handlers.read_property = aspekt_read_property;

	//original_zend_std_write_property = std_object_handlers.write_property;
	//std_object_handlers.write_property = aspekt_write_property;

	/*
	 * To avoid zendvm inc/dec property value directly
	 * When get_property_ptr_ptr return NULL, zendvm will use write_property to inc/dec property value
	 */
	//original_zend_std_get_property_ptr_ptr = std_object_handlers.get_property_ptr_ptr;
	//std_object_handlers.get_property_ptr_ptr = aspekt_get_property_ptr_ptr;

	ASPEKT_INIT(JoinPoint);
	ASPEKT_INIT(Interceptor);
	ASPEKT_INIT(Kind);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(aspekt)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(aspekt)
{
#if defined(COMPILE_DL_ASPEKT) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	ASPEKT_G(overloaded) = 0;
	ASPEKT_G(pointcut_version) = 0;

	ASPEKT_G(object_cache_size) = 1024;
	ASPEKT_G(object_cache) = ecalloc(1024, sizeof(object_cache *));

	ASPEKT_G(property_value) = NULL;

	ASPEKT_G(lock_read_property) = 0;
	ASPEKT_G(lock_write_property) = 0;

	ALLOC_HASHTABLE(ASPEKT_G(pointcuts_table));
	zend_hash_init(ASPEKT_G(pointcuts_table), 16, NULL, free_pointcut, 0);

	ALLOC_HASHTABLE(ASPEKT_G(function_cache));
	zend_hash_init(ASPEKT_G(function_cache), 16, NULL, free_pointcut_cache, 0);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(aspekt)
{
	zend_array_destroy(ASPEKT_G(pointcuts_table));
	zend_array_destroy(ASPEKT_G(function_cache));

	int i;
	for (i = 0; i < ASPEKT_G(object_cache_size); ++i) {
		if (ASPEKT_G(object_cache)[i] != NULL) {
			object_cache *aspekt_object_cache_g = ASPEKT_G(object_cache)[i];

			if (aspekt_object_cache_g->write!=NULL) {
				zend_hash_destroy(aspekt_object_cache_g->write);
				FREE_HASHTABLE(aspekt_object_cache_g->write);
			}

			if (aspekt_object_cache_g->read!=NULL) {
				zend_hash_destroy(aspekt_object_cache_g->read);
				FREE_HASHTABLE(aspekt_object_cache_g->read);
			}

			if (aspekt_object_cache_g->func!=NULL) {
				zend_hash_destroy(aspekt_object_cache_g->func);
				FREE_HASHTABLE(aspekt_object_cache_g->func);
			}

			efree(aspekt_object_cache_g);
		}
	}

	efree(ASPEKT_G(object_cache));

	if (ASPEKT_G(property_value) != NULL) {
		zval_ptr_dtor(ASPEKT_G(property_value));
		efree(ASPEKT_G(property_value));
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(aspekt)
{
	php_info_print_box_start(0);
	php_printf("%s", ASPEKT_DESCRIPTION);
	php_info_print_box_end();

	php_info_print_table_start();
	php_info_print_table_header(2, "aspekt support", "enabled");
	php_info_print_table_row(2, "Author", ASPEKT_AUTHOR);
	php_info_print_table_row(2, "Version", ASPEKT_VERSION);
	php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__ );
	php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

static const zend_module_dep aspekt_deps[] = { /* {{{ */
	ZEND_MOD_REQUIRED("spl")
	ZEND_MOD_REQUIRED("pcre")
	ZEND_MOD_REQUIRED("standard")
	ZEND_MOD_END
};
/* }}} */

zend_module_entry aspekt_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER_EX,
	NULL,
	aspekt_deps,
	ASPEKT_NAME,
	NULL,
	PHP_MINIT(aspekt),
	PHP_MSHUTDOWN(aspekt),
	PHP_RINIT(aspekt),
	PHP_RSHUTDOWN(aspekt),
	PHP_MINFO(aspekt),
	ASPEKT_VERSION,
	PHP_MODULE_GLOBALS(aspekt),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

/* implement standard "stub" routine to introduce ourselves to Zend */
#ifdef COMPILE_DL_ASPEKT
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(aspekt)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
