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
#include <main/php_ini.h>
#include <ext/standard/info.h>
#include <ext/standard/php_string.h>
#include <ext/pcre/php_pcre.h>
#include <Zend/zend_types.h>
#include <Zend/zend_objects.h>

#include "execute.h"
#include "joinpoint.h"

zend_object_handlers aspekt_joinpoint_handlers;
ASPEKT_API zend_class_entry *aspekt_joinpoint_ce_ptr;

static zend_always_inline aspekt_joinpoint_object_t *fetch_object(zend_object *obj) /* {{{ */
{
	return (aspekt_joinpoint_object_t *)((char *)(obj) - XtOffsetOf(aspekt_joinpoint_object_t, std));
}
/* }}} */

static zend_object *create_object(zend_class_entry *ce_ptr) /* {{{ */
{
	aspekt_joinpoint_object_t *intern;

	intern = ecalloc(1, sizeof(aspekt_joinpoint_object_t) + zend_object_properties_size(ce_ptr));

	zend_object_std_init(&intern->std, ce_ptr);
	intern->std.handlers = &aspekt_joinpoint_handlers;

	return &intern->std;
}
/* }}} */

static void object_free(zend_object *object) /* {{{ */
{
	aspekt_joinpoint_object_t *intern = fetch_object(object);

	if (intern->args != NULL) {
		zval_ptr_dtor(intern->args);
		efree(intern->args);
	}

	if (intern->return_value != NULL) {
		zval_ptr_dtor(intern->return_value);
		efree(intern->return_value);
	}

	if (Z_TYPE(intern->property_value) != IS_UNDEF) {
		zval_ptr_dtor(&intern->property_value);
		// TODO
		// efree(obj->property_value);
	}

	zend_object_std_dtor(&intern->std);
}
/* }}} */

static inline void zend_assign_to_variable_reference_ex(zval *variable_ptr, zval *value_ptr) /* {{{ */
{
	zend_reference *ref;

	if (EXPECTED(!Z_ISREF_P(value_ptr))) {
		ZVAL_NEW_REF(value_ptr, value_ptr);
	} else if (UNEXPECTED(variable_ptr == value_ptr)) {
		return;
	}

	ref = Z_REF_P(value_ptr);
	GC_ADDREF(ref);

	if (Z_REFCOUNTED_P(variable_ptr)) {
		zend_refcounted *garbage = Z_COUNTED_P(variable_ptr);
		if (GC_DELREF(garbage) == 0) {
			ZVAL_REF(variable_ptr, ref);
			zval_dtor_func(garbage);
			return;
		}

		GC_ZVAL_CHECK_POSSIBLE_ROOT(variable_ptr);
	}

	ZVAL_REF(variable_ptr, ref);
}
/* }}} */

/* {{{ aspekt_JoinPoint_init
   Create and register 'Aspekt\JoinPoint' class. */
ASPEKT_INIT_CLASS(JoinPoint) {
	ASPEKT_REGISTER_CLASS(Aspekt, JoinPoint, aspekt, joinpoint, aspekt_joinpoint_method_entry, 0);

	aspekt_joinpoint_ce_ptr->create_object = create_object;

	memcpy(&aspekt_joinpoint_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* offset of real object header (usually zero) */
	aspekt_joinpoint_handlers.offset = (int) XtOffsetOf(aspekt_joinpoint_object_t, std);

	/* general object functions */
	aspekt_joinpoint_handlers.free_obj  = object_free;
	aspekt_joinpoint_handlers.dtor_obj  = zend_objects_destroy_object;
	aspekt_joinpoint_handlers.clone_obj = NULL; /* has no clone implementation */

	return SUCCESS;
}
/* }}} */

/* {{{ proto array Aspekt\Joinpoint::getArguments()
   Get the triggering method arguments as an indexed array. */
PHP_METHOD(AspektJoinpoint, getArguments)
{
	aspekt_joinpoint_object_t *object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->args == NULL) {
		uint32_t call_num_args, first_extra_arg, i;
		zval *arg, *extra_start;
		zval *ret = emalloc(sizeof(zval));
		zend_op_array *op_array = &object->ex->func->op_array;

		array_init(ret);

		first_extra_arg = op_array->num_args;
		call_num_args = ZEND_CALL_NUM_ARGS(object->ex);

		if (call_num_args <= first_extra_arg) {
			for (i = 0; i < call_num_args; ++i) {
				arg = ZEND_CALL_VAR_NUM(object->ex, i);
				if (Z_ISUNDEF_P(arg)) {
					continue;
				}
				Z_TRY_ADDREF_P(arg);
				zend_hash_next_index_insert(Z_ARR_P(ret), arg);
			}
		} else {
			for (i = 0; i < first_extra_arg; ++i) {
				arg = ZEND_CALL_VAR_NUM(object->ex, i);
				if (Z_ISUNDEF_P(arg)) {
					continue;
				}
				Z_TRY_ADDREF_P(arg);
				zend_hash_next_index_insert(Z_ARR_P(ret), arg);
			}
			extra_start = ZEND_CALL_VAR_NUM(object->ex, op_array->last_var + op_array->T);
			for (i = 0; i < call_num_args - first_extra_arg; ++i) {
				Z_TRY_ADDREF_P(extra_start + i);
				zend_hash_next_index_insert(Z_ARR_P(ret), extra_start + i);
			}
		}

		object->args = ret;
	}

	RETURN_ZVAL(object->args, 1, 0);
}
/* }}} */

/* {{{ proto void Aspekt\Joinpoint::setArguments(array arguments)
   Replace all the arguments the triggering method will receive. */
PHP_METHOD(AspektJoinpoint, setArguments)
{
	zval* params;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a function or is a method call",
			"Aspekt\\Joinpoint::setArguments"
		);
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(params)
	ZEND_PARSE_PARAMETERS_END();

	if (object->args != NULL) {
		zval_ptr_dtor(object->args);
	} else {
		object->args = emalloc(sizeof(zval));
	}

	ZVAL_COPY(object->args, params);

	RETURN_NULL();
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getException() */
PHP_METHOD(AspektJoinpoint, getException)
{
	zval exception_val;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (!(object->current_pointcut->kind_of_advice & ASPEKT_KIND_CATCH)) {
		zend_error(E_ERROR,
			"%s() is only available when the advice was added with %s() either with %s()",
			"Aspekt\\Joinpoint::getException",
			"Aspekt\\Interceptor::addAfter",
			"Aspekt\\Interceptor::addAfterThrowing"
		);
	}

	if (object->exception != NULL) {
		ZVAL_OBJ(&exception_val, object->exception);
		RETURN_ZVAL(&exception_val, 1, 0);
	}
	RETURN_NULL();
}
/* }}} */

/* {{{ proto string Aspekt\Joinpoint::getPointcut()
   Get the pointcut that triggered the joinpoint. */
PHP_METHOD(AspektJoinpoint, getPointcut)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	RETURN_STR(object->current_pointcut->selector);
}
/* }}} */

/* {{{ proto void Aspekt\Joinpoint::process()
   Explicitly launch the triggering method or property operation. */
PHP_METHOD(AspektJoinpoint, process)
{
	zval call_ret;
	int is_ret_overloaded = 0;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (!object || !object->current_pointcut || !object->current_pointcut->kind_of_advice) {
		// TODO: Add descriptive error message
		zend_error(E_ERROR, "Error");
	}

	if (!(object->current_pointcut->kind_of_advice & ASPEKT_KIND_AROUND)) {
		zend_error(E_ERROR,
			"%s() is only available when the advice was added with %s()",
			"Aspekt\\Joinpoint::process",
			"Aspekt\\Interceptor::addAround"
		);
	}
	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY) {
		if (object->kind_of_advice & ASPEKT_KIND_WRITE) {
			do_write_property(object->pos, object->advice, getThis());
		} else {
			do_read_property(object->pos, object->advice, getThis());
		}
	} else {
		if (object->ex->return_value == NULL) {
			object->ex->return_value = &call_ret;
			is_ret_overloaded = 1;
		}
		do_func_execute(object->pos, object->advice, object->ex, getThis());
		if (is_ret_overloaded == 0) {
			if (EG(exception) == NULL) {
				ZVAL_COPY(return_value, object->ex->return_value);
			}
		} else {
			if (EG(exception) == NULL) {
				ZVAL_COPY_VALUE(return_value, object->ex->return_value);
			}
			object->ex->return_value = NULL;
		}
	}
}
/* }}} */

/* {{{ proto int Aspekt\Joinpoint::getKindOfAdvice()
   Get the condition where an advice was launched. */
PHP_METHOD(AspektJoinpoint, getKindOfAdvice)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	RETURN_LONG(object->kind_of_advice);
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getObject()
   Get the object of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getObject)
{
	zend_object *call_object = NULL;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY) {
		if (object->object != NULL) {
			RETURN_ZVAL(object->object, 1, 0);
		}
	} else {
#if PHP_VERSION_ID < 70100
		call_object = Z_OBJ(object->ex->This);
#else
		if (Z_TYPE(object->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(object->ex->This);
		}
#endif
		if (call_object != NULL) {
			RETURN_ZVAL(&object->ex->This, 1, 0);
		}
	}

	RETURN_NULL();
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getReturnedValue()
   Get the returned value of the triggering method. */
PHP_METHOD(AspektJoinpoint, getReturnedValue)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY) {
		zend_error(E_ERROR,
			"%s() is not available when the JoinPoint is a property operation (read or write)",
			"Aspekt\\Joinpoint::getReturnedValue"
		);
	}

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_BEFORE) {
		zend_error(E_ERROR,
			"%s() is not available when the advice was added with aspekt_add_before",
			"Aspekt\\Joinpoint::getReturnedValue"
		);
	}

	if (object->ex->return_value != NULL) {
		if (EXPECTED(Z_TYPE_P(object->ex->return_value) != IS_UNDEF)) {
			if (EXPECTED(!Z_ISREF_P(object->ex->return_value))) {
				object->return_value_changed = 1;
			}

			zend_assign_to_variable_reference_ex(return_value, object->ex->return_value);
		}
	}
}
/* }}} */

/* {{{ proto void Aspekt\Joinpoint::setReturnedValue(mixed value)
   Define the resulting value of the triggering method. */
PHP_METHOD(AspektJoinpoint, setReturnedValue)
{
	zval *returned_val;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->kind_of_advice & ASPEKT_KIND_WRITE) {
		zend_error(E_ERROR,
			"%s() is not available when the JoinPoint is a property write operation",
			"Aspekt\\Joinpoint::setReturnedValue"
		);
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(returned_val)
	ZEND_PARSE_PARAMETERS_END();

	if (object->return_value != NULL) {
		zval_ptr_dtor(object->return_value);
	} else {
		object->return_value = emalloc(sizeof(zval));
	}

	ZVAL_COPY(object->return_value, returned_val);

	RETURN_NULL();
}
/* }}} */

/* {{{ proto string Aspekt\Joinpoint::getClassName()
   Get the object's class name of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getClassName)
{
	zend_class_entry *ce = NULL;
	zend_object *call_object = NULL;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY) {
		if (object->object != NULL) {
			ce = Z_OBJCE_P(object->object);
			RETURN_STR(ce->name);
		}
	} else {
#if PHP_VERSION_ID < 70100
		call_object = Z_OBJ(object->ex->This);
#else
		if (Z_TYPE(object->ex->This) == IS_OBJECT) {
			call_object = Z_OBJ(object->ex->This);
		}
#endif
		if (call_object != NULL) {
			ce = Z_OBJCE(object->ex->This);
			RETURN_STR(ce->name);
		}

		if (ce == NULL && object->ex->func->common.fn_flags & ZEND_ACC_STATIC) {
			ce = object->ex->func->common.scope;
			RETURN_STR(ce->name);
		}
	}

	RETURN_NULL();
}
/* }}} */

/* {{{ proto string Aspekt\Joinpoint::getMethodName()
   Get the name of the method of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getMethodName)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY ||
		object->current_pointcut->kind_of_advice & ASPEKT_KIND_FUNCTION
	) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a method call",
			"Aspekt\\Joinpoint::getMethodName"
		);
	}

	if (object->ex == NULL) {
		RETURN_NULL();
	}

	RETURN_STR(object->ex->func->common.function_name);
}
/* }}} */

/* {{{ proto string Aspekt\Joinpoint::getFunctionName()
   Get the name of the function of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getFunctionName)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY ||
		object->current_pointcut->kind_of_advice & ASPEKT_KIND_METHOD
	) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a function call",
			"Aspekt\\Joinpoint::getFunctionName"
		);
	}

	if (object->ex == NULL) {
		RETURN_NULL();
	}

	RETURN_STR(object->ex->func->common.function_name);
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getAssignedValue()
   Get the value assigned to the property of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getAssignedValue)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (!(object->kind_of_advice & ASPEKT_KIND_WRITE)) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a property write operation",
			"Aspekt\\Joinpoint::getAssignedValue"
		);
	}

	if (Z_TYPE(object->property_value) != IS_UNDEF) {
		zend_assign_to_variable_reference_ex(return_value, &object->property_value);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto void Aspekt\Joinpoint::setAssignedValue(mixed value) */
PHP_METHOD(AspektJoinpoint, setAssignedValue)
{
	zval *assigned_val;
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (object->kind_of_advice & ASPEKT_KIND_READ) {
		zend_error(E_ERROR,
			"%s() is not available when the JoinPoint is a property read operation",
			"Aspekt\\Joinpoint::setAssignedValue"
		);
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(assigned_val)
	ZEND_PARSE_PARAMETERS_END_EX(
		zend_error(E_ERROR,
		   "%s() expects a variable as its first argument",
		   "Aspekt\\Joinpoint::setAssignedValue"
		);
		return;
	);

	if (Z_TYPE(object->property_value) != IS_UNDEF) {
		zval_ptr_dtor(&object->property_value);
	}

	ZVAL_COPY(&object->property_value, assigned_val);

	RETURN_NULL();
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getPropertyName()
   Get the name of the property of the triggered joinpoint. */
PHP_METHOD(AspektJoinpoint, getPropertyName)
{
	aspekt_joinpoint_object_t* object;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	object = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (!(object->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY)) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a property operation (read or write)",
			"Aspekt\\Joinpoint::getPropertyName"
		);
	}

	if (object->member != NULL) {
		RETURN_ZVAL(object->member, 1, 0);
		return;
	}

	RETURN_NULL();
}
/* }}} */

/* {{{ proto mixed Aspekt\Joinpoint::getPropertyValue() */
PHP_METHOD(AspektJoinpoint, getPropertyValue)
{
	zval *ret = NULL;
	zend_class_entry *scope;
	aspekt_joinpoint_object_t* intern;

	ASPEKT_INIT_THIS();
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, aspekt_joinpoint_ce_ptr);
	}

	intern = (aspekt_joinpoint_object_t *)Z_OBJ_P(this_ptr);

	if (!(intern->current_pointcut->kind_of_advice & ASPEKT_KIND_PROPERTY)) {
		zend_error(E_ERROR,
			"%s() is only available when the JoinPoint is a property operation (read or write)",
			"Aspekt\\Joinpoint::getPropertyValue"
		);
		return;
	}

	if (intern->object != NULL && intern->member != NULL) {
		/* Backup current scope */
		scope = aspekt_get_scope(0);

		/* Use intern's scope */
		aspekt_set_scope(Z_OBJCE_P(intern->object));

		ret = aspekt_get_property_ptr_ptr(intern->object, intern->member,
										  intern->type, intern->cache_slot);

		/* Restore original scope */
		aspekt_set_scope(scope);
	}

	if (ret == NULL) {
		RETURN_NULL();
	}

	RETURN_ZVAL(ret, 1, 0);
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
