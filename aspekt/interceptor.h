/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef PHP_ASPEKT_INTERCEPTOR_H
#define PHP_ASPEKT_INTERCEPTOR_H

#include <php.h>
#include <Zend/zend_API.h>

#include "../php_aspekt.h"

extern ASPEKT_API zend_class_entry *aspekt_interceptor_ce_ptr;
extern zend_object_handlers aspekt_interceptor_handlers;

typedef struct { /* {{{ */
	zend_object std;
} aspekt_interceptor_object_t; /* }}} */

ASPEKT_INIT_CLASS(Interceptor);

/* {{{ Methods proto */
PHP_METHOD(AspektInterceptor, addAround);
PHP_METHOD(AspektInterceptor, addBefore);
PHP_METHOD(AspektInterceptor, addAfter);
PHP_METHOD(AspektInterceptor, addAfterReturning);
PHP_METHOD(AspektInterceptor, addAfterThrowing);
/* }}} */

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(arginfo_aspekt_interceptor_add, 0, 0, 2)
	ZEND_ARG_INFO(0, pointcut)
	ZEND_ARG_INFO(0, advice)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry aspekt_interceptor_method_entry[] = { /* {{{ */
	PHP_ME(AspektInterceptor, addAround, arginfo_aspekt_interceptor_add, 0)
	PHP_ME(AspektInterceptor, addBefore, arginfo_aspekt_interceptor_add, 0)
	PHP_ME(AspektInterceptor, addAfter, arginfo_aspekt_interceptor_add, 0)
	PHP_ME(AspektInterceptor, addAfterReturning, arginfo_aspekt_interceptor_add, 0)
	PHP_ME(AspektInterceptor, addAfterThrowing, arginfo_aspekt_interceptor_add, 0)
	PHP_FE_END
};
/* }}} */

#endif // PHP_ASPEKT_INTERCEPTOR_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
