/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef ASPEKT_JOINPOINT_H
#define ASPEKT_JOINPOINT_H

#include <main/php.h>
#include <Zend/zend_API.h>

#include "../php_aspekt.h"

extern ASPEKT_API zend_class_entry *aspekt_joinpoint_ce_ptr;
extern zend_object_handlers aspekt_joinpoint_handlers;

typedef struct { /* {{{ */
	zend_object        std;

	pointcut          *current_pointcut;
	zend_array        *advice;
	HashPosition       pos;
	int                kind_of_advice;
	zend_execute_data *ex;
	int                is_ex_executed;
	zend_object       *exception;

	zval              *args;
	zval              *return_value;
	int                return_value_changed;

	zval              *object;
	zval              *member;
	int                type;
	void             **cache_slot;
	zval              *rv;
	zval               property_value;
} aspekt_joinpoint_object_t;
/* }}} */

ASPEKT_INIT_CLASS(JoinPoint);

/* {{{ Methods proto */
PHP_METHOD(AspektJoinpoint, getArguments);
PHP_METHOD(AspektJoinpoint, getPropertyName);
PHP_METHOD(AspektJoinpoint, getPropertyValue);
PHP_METHOD(AspektJoinpoint, setArguments);
PHP_METHOD(AspektJoinpoint, getKindOfAdvice);
PHP_METHOD(AspektJoinpoint, getReturnedValue);
PHP_METHOD(AspektJoinpoint, setReturnedValue);
PHP_METHOD(AspektJoinpoint, getAssignedValue);
PHP_METHOD(AspektJoinpoint, setAssignedValue);
PHP_METHOD(AspektJoinpoint, getPointcut);
PHP_METHOD(AspektJoinpoint, getObject);
PHP_METHOD(AspektJoinpoint, getClassName);
PHP_METHOD(AspektJoinpoint, getMethodName);
PHP_METHOD(AspektJoinpoint, getFunctionName);
PHP_METHOD(AspektJoinpoint, getException);
PHP_METHOD(AspektJoinpoint, process);
/* }}} */

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(arginfo_aspekt_joinpoint_returnbyref, 0, ZEND_RETURN_REFERENCE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_aspekt_joinpoint_set_args, 0)
	ZEND_ARG_ARRAY_INFO(0, arguments, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_aspekt_joinpoint_set_return_val, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_aspekt_joinpoint_set_assign_val, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry aspekt_joinpoint_method_entry[] = { /* {{{  */
	PHP_ME(AspektJoinpoint, getArguments, NULL, 0)
	PHP_ME(AspektJoinpoint, setArguments, arginfo_aspekt_joinpoint_set_args, 0)
	PHP_ME(AspektJoinpoint, getException, NULL, 0)
	PHP_ME(AspektJoinpoint, getPointcut, NULL, 0)
	PHP_ME(AspektJoinpoint, process, NULL, 0)
	PHP_ME(AspektJoinpoint, getKindOfAdvice, NULL, 0)
	PHP_ME(AspektJoinpoint, getObject, NULL, 0)
	PHP_ME(AspektJoinpoint, getReturnedValue, arginfo_aspekt_joinpoint_returnbyref, 0)
	PHP_ME(AspektJoinpoint, setReturnedValue, arginfo_aspekt_joinpoint_set_return_val, 0)
	PHP_ME(AspektJoinpoint, getClassName, NULL, 0)
	PHP_ME(AspektJoinpoint, getMethodName, NULL, 0)
	PHP_ME(AspektJoinpoint, getFunctionName, NULL, 0)
	PHP_ME(AspektJoinpoint, getAssignedValue, arginfo_aspekt_joinpoint_returnbyref, 0)
	PHP_ME(AspektJoinpoint, setAssignedValue, arginfo_aspekt_joinpoint_set_assign_val, 0)
	PHP_ME(AspektJoinpoint, getPropertyName, NULL, 0)
	PHP_ME(AspektJoinpoint, getPropertyValue, NULL, 0)
	PHP_FE_END
};
/* }}} */

#endif // ASPEKT_JOINPOINT_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
