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

#include "../php_aspekt.h"
#include "execute.h"
#include "joinpoint.h"

zend_object_read_property_t original_zend_std_read_property;
zend_object_write_property_t original_zend_std_write_property;
zend_object_get_property_ptr_ptr_t original_zend_std_get_property_ptr_ptr;

void (*original_zend_execute_ex)(zend_execute_data *execute_data);
void (*original_zend_execute_internal)(zend_execute_data *execute_data, zval *return_value);

static void execute_pointcut(pointcut *pc, zval *arg, zval *retval) /* {{{ */
{
	zval params[1];

	ZVAL_COPY_VALUE(&params[0], arg);

	pc->fci.retval = retval;
	pc->fci.param_count = 1;
	pc->fci.params = params;

	if (zend_call_function(&pc->fci, &pc->fci_cache) == FAILURE) {
		zend_error(E_ERROR, "An error occurred during to executing provided advice");
	}
}
/* }}} */

static void execute_context(zend_execute_data *execute_data, zval *args) /* {{{ */
{
	if (EG(exception)) {
		return;
	}

	// Overload arguments
	if (args != NULL) {
		uint32_t i, first_extra_arg, call_num_args;
		zval *original_args_value;
		zval *overload_args_value;
		zend_op_array *op_array = &EX(func)->op_array;

		first_extra_arg = op_array->num_args;
		call_num_args = zend_hash_num_elements(Z_ARR_P(args));

		if (call_num_args <= first_extra_arg) {
			for (i = 0; i < call_num_args; ++i) {
				original_args_value = ZEND_CALL_VAR_NUM(execute_data, i);
				overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)i);

				zval_ptr_dtor(original_args_value);
				ZVAL_COPY(original_args_value, overload_args_value);
			}
		} else {
			// 1. Overload common params
			for (i = 0; i < first_extra_arg; ++i) {
				original_args_value = ZEND_CALL_VAR_NUM(execute_data, i);
				overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)i);

				zval_ptr_dtor(original_args_value);
				ZVAL_COPY(original_args_value, overload_args_value);
			}

			// 2. Overload extra params
			if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
				for (i = 0; i < call_num_args - first_extra_arg; ++i) {
					original_args_value = ZEND_CALL_VAR_NUM(execute_data, op_array->last_var + op_array->T + i);
					overload_args_value = zend_hash_index_find(Z_ARR_P(args), (zend_ulong)(i + first_extra_arg));

					zval_ptr_dtor(original_args_value);
					ZVAL_COPY(original_args_value, overload_args_value);
				}
			}
		}
		ZEND_CALL_NUM_ARGS(execute_data) = call_num_args;
	}

	EG(current_execute_data) = execute_data;

	if (execute_data->func->common.type == ZEND_USER_FUNCTION) {
		original_zend_execute_ex(execute_data);
		return;
	}

	if (execute_data->func->common.type == ZEND_INTERNAL_FUNCTION) {
		if (original_zend_execute_internal) {
			original_zend_execute_internal(execute_data, execute_data->return_value);
		} else{
			execute_internal(execute_data, execute_data->return_value);
		}

		return;
	}


	// This will never happened because there's no hook for overload function
#if PHP_VERSION_ID >= 70400
#elif PHP_VERSION_ID >= 70200
	zend_do_fcall_overloaded(execute_data, execute_data->return_value);
#elif PHP_VERSION_ID >= 70100
	zend_do_fcall_overloaded(execute_data->func, execute_data, execute_data->return_value);
#endif
}
/* }}} */

static object_cache *get_object_cache(zend_object *object) /* {{{ */
{
	int i;
	uint32_t handle;

	handle = object->handle;
	if (handle >= ASPEKT_G(object_cache_size)) {
		ASPEKT_G(object_cache) = erealloc(ASPEKT_G(object_cache), sizeof(object_cache) * handle + 1);
		for (i = ASPEKT_G(object_cache_size); i <= handle; ++i) {
			ASPEKT_G(object_cache)[i] = NULL;
		}
		ASPEKT_G(object_cache_size) = handle + 1;
	}

	if (ASPEKT_G(object_cache)[handle] == NULL) {
		ASPEKT_G(object_cache)[handle] = emalloc(sizeof(object_cache));
		ASPEKT_G(object_cache)[handle]->write = NULL;
		ASPEKT_G(object_cache)[handle]->read = NULL;
		ASPEKT_G(object_cache)[handle]->func = NULL;
	}

	return ASPEKT_G(object_cache)[handle];
}
/* }}} */

static zend_array *get_object_cache_func(zend_object *object) /* {{{ */
{
	object_cache *cache;
	cache = get_object_cache(object);
	if (cache->func == NULL) {
		ALLOC_HASHTABLE(cache->func);
		zend_hash_init(cache->func, 16, NULL, free_pointcut_cache , 0);
	}

	return cache->func;
}
/* }}} */

static zend_array *get_object_cache_read(zend_object *object) /* {{{ */
{
	object_cache *cache;
	cache = get_object_cache(object);

	if (cache->read == NULL) {
		ALLOC_HASHTABLE(cache->read);
		zend_hash_init(cache->read, 16, NULL, free_pointcut_cache, 0);
	}

	return cache->read;
}
/* }}} */

static zend_array *get_object_cache_write(zend_object *object) /* {{{ */
{
	object_cache *cache;
	cache = get_object_cache(object);

	if (cache->write == NULL) {
		ALLOC_HASHTABLE(cache->write);
		zend_hash_init(cache->write, 16, NULL, free_pointcut_cache , 0);
	}

	return cache->write;
}
/* }}} */

static int strcmp_with_joker_case(char *str_with_jok, char *str, int case_sensitive) /* {{{ */
{
	if (str_with_jok[0] == '*') {
		if (str_with_jok[1] == '\0') {
			return 1;
		}
	}

	if (str_with_jok[0] == '*') {
		if (case_sensitive) {
			return !strcmp(str_with_jok + 1, str+(strlen(str)-(strlen(str_with_jok) - 1)));
		}

		return !strcasecmp(str_with_jok + 1, str+(strlen(str)-(strlen(str_with_jok) - 1)));
	}

	if (str_with_jok[strlen(str_with_jok) - 1] == '*') {
		if (case_sensitive) {
			return !strncmp(str_with_jok, str, strlen(str_with_jok) - 1);
		}

		return !strncasecmp(str_with_jok, str, strlen(str_with_jok) - 1);
	}

	if (case_sensitive) {
		return !strcmp(str_with_jok, str);
	}

	return !strcasecmp(str_with_jok, str);
}
/* }}} */

static int test_property_scope(pointcut *current_pc, zend_class_entry *ce, zend_string *member_str) /* {{{ */
{
	zval *property_info_val;
	zend_property_info *property_info = NULL;

	property_info_val = zend_hash_find(&ce->properties_info, member_str);
	if (property_info_val) {
		property_info = (zend_property_info *)Z_PTR_P(property_info_val);
		if (current_pc->static_state != 2) {
			if (current_pc->static_state) {
				if (!(property_info->flags & ZEND_ACC_STATIC)) {
					return 0;
				}
			} else {
				if ((property_info->flags & ZEND_ACC_STATIC)) {
					return 0;
				}
			}
		}

		if (current_pc->scope != 0 && !(current_pc->scope & (property_info->flags & ZEND_ACC_PPP_MASK))) {
			return 0;
		}
	} else {
		if (current_pc->scope != 0 && !(current_pc->scope & ZEND_ACC_PUBLIC)) {
			return 0;
		}
		if (current_pc->static_state == 1) {
			return 0;
		}
	}
	return 1;
}
/* }}} */

static int pointcut_match_zend_class_entry(pointcut *pc, zend_class_entry *ce) /* {{{ */
{
	int i, matches;

	/* TODO(serghei): In fact, this should never happen.
	 * However, I managed to catch this in tests (PHP 7.4):
	 *   - tests/after_returning/001.phpt
	 *   - tests/after_throwing/001.phpt */
	if (pc == NULL || pc->re_class == NULL) {
		return 0;
	}

#if PHP_VERSION_ID >= 70300
	pcre2_match_data *match_data = php_pcre_create_match_data(0, pc->re_class);
	if (NULL == match_data) {
		return 0;
	}

	// 1. looking for class
	matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->name),
						  ZSTR_LEN(ce->name), 0, 0,  match_data, php_pcre_mctx());

	if (matches >= 0) {
		php_pcre_free_match_data(match_data);
		return 1;
	}
#else
	matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->name),
						(int) ZSTR_LEN(ce->name), 0, 0, NULL, 0);

	if (matches >= 0) {
		return 1;
	}
#endif

	// 2. looking for interface
	for (i = 0; i < (int) ce->num_interfaces; ++i) {
#if PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class,
							  (PCRE2_SPTR)ZSTR_VAL(ce->interfaces[i]->name),
							  ZSTR_LEN(ce->interfaces[i]->name), 0, 0, match_data,
							  php_pcre_mctx());

		if (matches >= 0) {
			php_pcre_free_match_data(match_data);
			return 1;
		}
#else
		matches = pcre_exec(pc->re_class, NULL,
							ZSTR_VAL(ce->interfaces[i]->name),
							(int) ZSTR_LEN(ce->interfaces[i]->name), 0, 0,
							NULL, 0);

		if (matches >= 0) {
			return 1;
		}
#endif
	}

	// 3. looking for trait
	for (i = 0; i < (int) ce->num_traits; ++i) {
#if PHP_VERSION_ID >= 70400
		matches = pcre2_match(pc->re_class,
							  (PCRE2_SPTR)ZSTR_VAL(ce->trait_names[i].name),
							  ZSTR_LEN(ce->trait_names[i].name), 0, 0,
							  match_data, php_pcre_mctx());

		if (matches >= 0) {
			php_pcre_free_match_data(match_data);
			return 1;
		}
#elif PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class,
							  (PCRE2_SPTR)ZSTR_VAL(ce->traits[i]->name),
							  ZSTR_LEN(ce->traits[i]->name), 0, 0,
							  match_data, php_pcre_mctx());

		if (matches >= 0) {
			php_pcre_free_match_data(match_data);
			return 1;
		}
#else
		matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->traits[i]->name),
			(int) ZSTR_LEN(ce->traits[i]->name), 0, 0, NULL, 0);

		if (matches>=0) {
			return 1;
		}
#endif
	}

	ce = ce->parent;
	// 4. looking for parent class
	while (ce != NULL) {
#if PHP_VERSION_ID >= 70300
		matches = pcre2_match(pc->re_class, (PCRE2_SPTR)ZSTR_VAL(ce->name),
							  ZSTR_LEN(ce->name), 0, 0,
							  match_data, php_pcre_mctx());

		if (matches >= 0) {
			php_pcre_free_match_data(match_data);
			return 1;
		}
#else
		matches = pcre_exec(pc->re_class, NULL, ZSTR_VAL(ce->name),
							(int) ZSTR_LEN(ce->name), 0, 0, NULL, 0);

		if (matches >= 0) {
			return 1;
		}
#endif
		ce = ce->parent;
	}

#if PHP_VERSION_ID >= 70300
	php_pcre_free_match_data(match_data);
#endif

	return 0;
}
/* }}} */

static int pointcut_match_zend_function(pointcut *pc, zend_execute_data *ex) /* {{{ */
{
	int comp_start = 0;
	zend_function *curr_func = ex->func;

	// check static
	if (pc->static_state != 2) {
        if (pc->static_state != 0 == !(curr_func->common.fn_flags & ZEND_ACC_STATIC)) {
            return 0;
        }
	}

	// check public/protect/private
	if (pc->scope != 0 && !(pc->scope & (curr_func->common.fn_flags & ZEND_ACC_PPP_MASK))) {
		return 0;
	}

	if (pc->class_name == NULL && ZSTR_VAL(pc->method)[0] == '*' && ZSTR_VAL(pc->method)[1] == '\0') {
		return 1;
	}

	if (pc->class_name == NULL && curr_func->common.scope != NULL) {
		return 0;
	}

	if (pc->method_jok) {
		int matches;

#if PHP_VERSION_ID >= 70300
		pcre2_match_data *match_data = php_pcre_create_match_data(0, pc->re_method);
		if (NULL == match_data) {
			return 0;
		}

		matches = pcre2_match(pc->re_method,
							  (PCRE2_SPTR)ZSTR_VAL(curr_func->common.function_name),
							  ZSTR_LEN(curr_func->common.function_name), 0, 0,
							  match_data, php_pcre_mctx());

		php_pcre_free_match_data(match_data);
		if (matches < 0) {
			return 0;
		}
#else
		matches = pcre_exec(pc->re_method, NULL,
							ZSTR_VAL(curr_func->common.function_name),
							(int) ZSTR_LEN(curr_func->common.function_name),
							0, 0, NULL, 0);

		if (matches < 0) {
			return 0;
		}
#endif
	} else {
		if (ZSTR_VAL(pc->method)[0] == '\\') {
			comp_start = 1;
		}
		if (strcasecmp(ZSTR_VAL(pc->method) + comp_start, ZSTR_VAL(curr_func->common.function_name))) {
			return 0;
		}
	}

	return 1;
}
/* }}} */

static zend_array *calculate_class_pointcuts(zend_class_entry *ce, int kind_of_advice) /* {{{ */
{
	pointcut *pc;
	zend_array *ht;
	zval *pc_value;

	ALLOC_HASHTABLE(ht);
	zend_hash_init(ht, 16, NULL, NULL , 0);

	ZEND_HASH_FOREACH_VAL(ASPEKT_G(pointcuts_table), pc_value) {
		pc = (pointcut *)Z_PTR_P(pc_value);
		if (!(pc->kind_of_advice & kind_of_advice)) {
			continue;
		}

		if ((ce == NULL && pc->kind_of_advice & ASPEKT_KIND_FUNCTION)
			|| (ce != NULL && pointcut_match_zend_class_entry(pc, ce))) {
			zend_hash_next_index_insert(ht, pc_value);
		}
	} ZEND_HASH_FOREACH_END();

	return ht;
}
/* }}} */

static zend_array *calculate_function_pointcuts(zend_execute_data *ex) /* {{{ */
{
	zend_object *object = NULL;
	zend_class_entry *ce = NULL;
	zend_array *class_pointcuts;
	zval *pc_value;
	pointcut *pc;
	zend_ulong h;

#if PHP_VERSION_ID < 70100
	object = Z_OBJ(ex->This);
#else
	if (Z_TYPE(ex->This) == IS_OBJECT) {
		object = Z_OBJ(ex->This);
	}
#endif

	if (object != NULL) {
		ce = Z_OBJCE(ex->This);
	}

	if (ce == NULL && ex->func->common.fn_flags & ZEND_ACC_STATIC) {
		ce = ex->func->common.scope;
	}

	class_pointcuts = calculate_class_pointcuts(ce, ASPEKT_KIND_FUNCTION | ASPEKT_KIND_METHOD);

	ZEND_HASH_FOREACH_NUM_KEY_VAL(class_pointcuts, h, pc_value) {
		pc = (pointcut *)Z_PTR_P(pc_value);
		if (pointcut_match_zend_function(pc, ex)) {
			continue;
		}

		// delete unmatch element
		zend_hash_index_del(class_pointcuts, h);
	} ZEND_HASH_FOREACH_END();

	return class_pointcuts;
}
/* }}} */

static zend_array *calculate_property_pointcuts(zval *object, zend_string *member_str, int kind) /* {{{ */
{
	zend_array *class_pointcuts;
	zval *pc_value;
	pointcut *pc;
	zend_ulong h;

	class_pointcuts = calculate_class_pointcuts(Z_OBJCE_P(object), kind);

	ZEND_HASH_FOREACH_NUM_KEY_VAL(class_pointcuts, h, pc_value) {
		pc = (pointcut *)Z_PTR_P(pc_value);
		if (ZSTR_VAL(pc->method)[0] != '*') {
			if (!strcmp_with_joker_case(ZSTR_VAL(pc->method), ZSTR_VAL(member_str), 1)) {
				zend_hash_index_del(class_pointcuts, h);
				continue;
			}
		}

		if (pc->static_state != 2 || pc->scope != 0) {
			if (!test_property_scope(pc, Z_OBJCE_P(object), member_str)) {
				zend_hash_index_del(class_pointcuts, h);
				continue;
			}
		}
	} ZEND_HASH_FOREACH_END();

	return class_pointcuts;
}
/* }}} */

static zend_array *get_cache_func(zend_execute_data *ex) /* {{{ */
{
	zend_array *ht_object_cache = NULL;
	zend_object *object = NULL;
	zend_class_entry *ce;
	zend_string *cache_key;
	zval *cache = NULL;
	zval pc_value;
	pointcut_cache *pcc = NULL;

#if PHP_VERSION_ID < 70100
	object = Z_OBJ(ex->This);
#else
	if (Z_TYPE(ex->This) == IS_OBJECT) {
		object = Z_OBJ(ex->This);
	}
#endif

	// 1. search cache
	if (object == NULL) { // function or static method
		ht_object_cache = ASPEKT_G(function_cache);
		if (ex->func->common.fn_flags & ZEND_ACC_STATIC) {
			ce = ex->func->common.scope;
			cache_key = zend_string_init("", ZSTR_LEN(ex->func->common.function_name) + ZSTR_LEN(ce->name) + 2, 0);
			sprintf((char *)ZSTR_VAL(cache_key),
					"%s::%s", ZSTR_VAL(ce->name), ZSTR_VAL(ex->func->common.function_name));
		} else {
			cache_key = zend_string_copy(ex->func->common.function_name);
		}
	} else { // method
		ce = ex->func->common.scope;
		cache_key = zend_string_copy(ex->func->common.function_name);
		ht_object_cache = get_object_cache_func(object);
	}

	cache = zend_hash_find(ht_object_cache, cache_key);

	if (cache != NULL) {
		pcc = (pointcut_cache *)Z_PTR_P(cache);
		if (pcc->version != ASPEKT_G(pointcut_version) || (object != NULL && pcc->ce != ce)) {
			// cache lost
			pcc = NULL;
			zend_hash_del(ht_object_cache, cache_key);
		}
	}

	// 2. calculate function hit pointcut
	if (pcc == NULL) {
		pcc = (pointcut_cache *)emalloc(sizeof(pointcut_cache));
		pcc->ht = calculate_function_pointcuts(ex);
		pcc->version = ASPEKT_G(pointcut_version);

		if (object == NULL) {
			pcc->ce = NULL;
		} else {
			pcc->ce = ce;
		}

		ZVAL_PTR(&pc_value, pcc);
		zend_hash_add(ht_object_cache, cache_key, &pc_value);
	}

	zend_string_release(cache_key);

	return pcc->ht;
}
/* }}} */

static zend_array *get_cache_property(zval *object, zval *member, int type) /* {{{ */
{
	zend_array *ht_object_cache = NULL;
	zval *cache = NULL;
	pointcut_cache *pcc = NULL;
	zval pc_value;
	zend_string *member_str = NULL;

	if (type & ASPEKT_KIND_READ) {
		ht_object_cache = get_object_cache_read(Z_OBJ_P(object));
	} else {
		ht_object_cache = get_object_cache_write(Z_OBJ_P(object));
	}

	if (Z_TYPE_P(member) != IS_STRING ) {
		member_str = zval_get_string(member);
	} else {
		member_str = Z_STR_P(member);
	}

	cache = zend_hash_find(ht_object_cache, member_str);

	if (cache != NULL) {
		pcc = (pointcut_cache *)Z_PTR_P(cache);
		if (pcc->version != ASPEKT_G(pointcut_version) || pcc->ce != Z_OBJCE_P(object)) {
			// cache lost
			pcc = NULL;
			zend_hash_del(ht_object_cache, member_str);
		}
	}

	if (pcc == NULL) {
		pcc = (pointcut_cache *)emalloc(sizeof(pointcut_cache));
		pcc->ht = calculate_property_pointcuts(object, member_str, type);
		pcc->version = ASPEKT_G(pointcut_version);
		pcc->ce = Z_OBJCE_P(object);

		ZVAL_PTR(&pc_value, pcc);
		zend_hash_add(ht_object_cache, member_str, &pc_value);
	}

	if (member_str != Z_STR_P(member)) {
		zend_string_release(member_str);
	}

	return pcc->ht;
}
/* }}} */

static void func_pointcut_and_execute(zend_execute_data *ex) /* {{{ */
{
	zval aspekt_object;
	aspekt_joinpoint_object_t *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;
	zval *real_return_value;

	// find pointcut of current call function
	pointcut_table = get_cache_func(ex);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
		ASPEKT_G(overloaded) = 0;
		execute_context(ex, NULL);
		ASPEKT_G(overloaded) = 1;

		return;
	}

	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aspekt_object, aspekt_joinpoint_ce_ptr);
	joinpoint = (aspekt_joinpoint_object_t *) Z_OBJ(aspekt_object);

	joinpoint->ex = ex;
	joinpoint->is_ex_executed = 0;
	joinpoint->advice = pointcut_table;
	joinpoint->exception = NULL;
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;

	ZVAL_UNDEF(&joinpoint->property_value);

	if (EG(current_execute_data) == ex) {
		// delay to execute call function, execute pointcut first
		EG(current_execute_data) = ex->prev_execute_data;
	}

	int no_ret = 0;
	if (ex->return_value == NULL) {
		no_ret = 1;
		ex->return_value = emalloc(sizeof(zval));
		ZVAL_UNDEF(ex->return_value);
	}

	do_func_execute(pos, pointcut_table, ex, &aspekt_object);

	if (no_ret == 1) {
		zval_ptr_dtor(ex->return_value);
		efree(ex->return_value);
	} else {
		if (joinpoint->return_value_changed && Z_ISREF_P(ex->return_value)) {
			real_return_value = Z_REFVAL_P(ex->return_value);
			Z_TRY_ADDREF_P(real_return_value);
			zval_ptr_dtor(ex->return_value);
			ZVAL_COPY_VALUE(ex->return_value, real_return_value);
		}
	}

	if (joinpoint->is_ex_executed == 0) {
		uint32_t i;
		zval *original_args_value;

		for (i = 0; i < ex->func->common.num_args; ++i) {
			original_args_value = ZEND_CALL_VAR_NUM(ex, i);
			zval_ptr_dtor(original_args_value);
		}
	}

	zval_ptr_dtor(&aspekt_object);
}
/* }}} */

void do_func_execute(HashPosition pos, zend_array *pointcut_table, zend_execute_data *ex, zval *aspekt_object) /* {{{ */
{
	pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret;
	zend_object *exception = NULL;
	aspekt_joinpoint_object_t* joinpoint;
#if PHP_VERSION_ID < 70100
	zend_class_entry *current_scope = NULL;
#endif

	joinpoint = (aspekt_joinpoint_object_t *) Z_OBJ_P(aspekt_object);

	while(1) {
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_VERSION_ID < 70100
		if (EG(scope) != ex->called_scope) {
            current_scope = EG(scope);
            EG(scope) = ex->called_scope;
        }
#endif
		ASPEKT_G(overloaded) = 0;
		execute_context(ex, joinpoint->args);
		ASPEKT_G(overloaded) = 1;

		joinpoint->is_ex_executed = 1;

#if PHP_VERSION_ID < 70100
		if (current_scope != NULL) {
            EG(scope) = current_scope;
        }
#endif

		return;
	}

	zend_hash_move_forward_ex(pointcut_table, &pos);
	current_pc = (pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & ASPEKT_KIND_BEFORE) {
		if (!EG(exception)) {
			execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
			if (&pointcut_ret != NULL) {
				zval_ptr_dtor(&pointcut_ret);
			}
		}
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AROUND) {
		if (!EG(exception)) {
			execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
			if (&pointcut_ret != NULL && !Z_ISNULL(pointcut_ret)) {
				if (ex->return_value != NULL) {
					// TODO(serghei): Fixme
					if (Z_TYPE_P(ex->return_value) > IS_NULL && Z_TYPE_P(ex->return_value) < 21) {
						zval_ptr_dtor(ex->return_value);
					}

					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}
		}
	} else {
		do_func_execute(pos, pointcut_table, ex, aspekt_object);
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AFTER) {
		if (current_pc->kind_of_advice & ASPEKT_KIND_CATCH && EG(exception)) {
			exception = EG(exception);
			joinpoint->exception = exception;
			EG(exception) = NULL;
			execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
			EG(exception) = exception;

			if (&pointcut_ret != NULL && !Z_ISNULL(pointcut_ret)) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}

		} else if (current_pc->kind_of_advice & ASPEKT_KIND_RETURN && !EG(exception)) {
			execute_pointcut(current_pc, aspekt_object, &pointcut_ret);

			if (&pointcut_ret != NULL && !Z_ISNULL(pointcut_ret)) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY_VALUE(ex->return_value, &pointcut_ret);
				} else {
					zval_ptr_dtor(&pointcut_ret);
				}
			} else if (joinpoint->return_value != NULL) {
				if (ex->return_value != NULL) {
					zval_ptr_dtor(ex->return_value);
					ZVAL_COPY(ex->return_value, joinpoint->return_value);
				}
			}
		}
	}
}
/* }}} */

void do_read_property(HashPosition pos, zend_array *pointcut_table, zval *aspekt_object) /* {{{ */
{
	pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret;
	aspekt_joinpoint_object_t* joinpoint;
	zval *property_value;
	zend_class_entry *current_scope = NULL;

	joinpoint = (aspekt_joinpoint_object_t *) Z_OBJ_P(aspekt_object);

	while(1) {
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_VERSION_ID < 70100
		if (EG(scope) != joinpoint->ex->called_scope) {
            current_scope = EG(scope);
            EG(scope) = joinpoint->ex->called_scope;
        }
#else
		if (EG(fake_scope) != joinpoint->ex->func->common.scope) {
			current_scope = EG(fake_scope);
			EG(fake_scope) = joinpoint->ex->func->common.scope;
		}
#endif

		property_value = original_zend_std_read_property(
			joinpoint->object,
			joinpoint->member,
			joinpoint->type,
			joinpoint->cache_slot,
			joinpoint->rv
		);

		ZVAL_COPY_VALUE(ASPEKT_G(property_value), property_value);

		if (current_scope != NULL) {
			aspekt_set_scope(current_scope);
		}

		return;
	}
	zend_hash_move_forward_ex(pointcut_table, &pos);

	current_pc = (pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = (current_pc->kind_of_advice & ASPEKT_KIND_WRITE) ?
		(current_pc->kind_of_advice - ASPEKT_KIND_WRITE) :
		current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & ASPEKT_KIND_BEFORE) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
		if (&pointcut_ret != NULL) {
			zval_ptr_dtor(&pointcut_ret);
		}
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AROUND) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
		if (&pointcut_ret != NULL && !Z_ISNULL(pointcut_ret)) {
			ZVAL_COPY_VALUE(ASPEKT_G(property_value), &pointcut_ret);
		} else if (joinpoint->return_value != NULL) {
			ZVAL_COPY(ASPEKT_G(property_value), joinpoint->return_value);
		}
	} else {
		do_read_property(pos, pointcut_table, aspekt_object);
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AFTER) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
		if (&pointcut_ret != NULL && !Z_ISNULL(pointcut_ret)) {
			ZVAL_COPY_VALUE(ASPEKT_G(property_value), &pointcut_ret);
		} else if (joinpoint->return_value != NULL) {
			ZVAL_COPY(ASPEKT_G(property_value), joinpoint->return_value);
		}
	}
}
/* }}} */

void do_write_property(HashPosition pos, zend_array *pointcut_table, zval *aspekt_object) /* {{{ */
{
	pointcut *current_pc = NULL;
	zval *current_pc_value = NULL;
	zval pointcut_ret;
	aspekt_joinpoint_object_t* joinpoint;
	zend_class_entry *current_scope = NULL;

	joinpoint = (aspekt_joinpoint_object_t *) Z_OBJ_P(aspekt_object);

	while(1) {
		current_pc_value = zend_hash_get_current_data_ex(pointcut_table, &pos);
		if (current_pc_value == NULL || Z_TYPE_P(current_pc_value) != IS_UNDEF) {
			break;
		} else {
			zend_hash_move_forward_ex(pointcut_table, &pos);
		}
	}

	if (current_pc_value == NULL) {
#if PHP_VERSION_ID < 70100
	if (EG(scope) != joinpoint->ex->called_scope) {
		current_scope = EG(scope);
		EG(scope) = joinpoint->ex->called_scope;
	}
#else
	if (EG(fake_scope) != joinpoint->ex->func->common.scope) {
		current_scope = EG(fake_scope);
		EG(fake_scope) = joinpoint->ex->func->common.scope;
	}
#endif
	original_zend_std_write_property(
		joinpoint->object,
		joinpoint->member,
		&joinpoint->property_value,
		joinpoint->cache_slot
	);

	if (current_scope != NULL) {
#if PHP_VERSION_ID < 70100
		EG(scope) = current_scope;
#else
		EG(fake_scope) = current_scope;
#endif
	}

	return;
}

	zend_hash_move_forward_ex(pointcut_table, &pos);

	current_pc = (pointcut *)Z_PTR_P(current_pc_value);

	joinpoint->current_pointcut = current_pc;
	joinpoint->pos = pos;
	joinpoint->kind_of_advice = (current_pc->kind_of_advice & ASPEKT_KIND_READ) ?
		(current_pc->kind_of_advice - ASPEKT_KIND_READ) :
		current_pc->kind_of_advice;

	if (current_pc->kind_of_advice & ASPEKT_KIND_BEFORE) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
		if (&pointcut_ret != NULL) {
			zval_ptr_dtor(&pointcut_ret);
		}
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AROUND) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
	} else {
		do_write_property(pos, pointcut_table, aspekt_object);
	}

	if (current_pc->kind_of_advice & ASPEKT_KIND_AFTER) {
		execute_pointcut(current_pc, aspekt_object, &pointcut_ret);
	}
}
/* }}} */

zval *aspekt_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv) /* {{{ */
{
	zval aspekt_object;
	aspekt_joinpoint_object_t *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;

	if (ASPEKT_G(lock_read_property) > 25) {
		zend_error(E_ERROR, "Too many level of nested advices. Is there any recursive call?");
	}

	pointcut_table = get_cache_property(object, member, ASPEKT_KIND_READ);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
		return original_zend_std_read_property(object, member, type, cache_slot ,rv);
	}
	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aspekt_object, aspekt_joinpoint_ce_ptr);
	joinpoint = (aspekt_joinpoint_object_t *)Z_OBJ(aspekt_object);

	joinpoint->advice = pointcut_table;
	joinpoint->ex = EG(current_execute_data);
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;
	joinpoint->object = object;
	joinpoint->member = member;
	joinpoint->type = type;
	// To avoid use runtime cache
	joinpoint->cache_slot = NULL;
	joinpoint->rv = rv;

	ZVAL_UNDEF(&joinpoint->property_value);

	if (ASPEKT_G(property_value) == NULL) {
		ASPEKT_G(property_value) = emalloc(sizeof(zval));
	}
	ZVAL_NULL(ASPEKT_G(property_value));

	ASPEKT_G(lock_read_property)++;
	do_read_property(0, pointcut_table, &aspekt_object);
	ASPEKT_G(lock_read_property)--;

	zval_ptr_dtor(&aspekt_object);

	return ASPEKT_G(property_value);
}
/* }}} */

zval *aspekt_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot)
{
	zend_execute_data *ex = EG(current_execute_data);

	if (ex->opline == NULL ||
		(ex->opline->opcode != ZEND_PRE_INC_OBJ &&
		ex->opline->opcode != ZEND_POST_INC_OBJ &&
		ex->opline->opcode != ZEND_PRE_DEC_OBJ &&
		ex->opline->opcode != ZEND_POST_DEC_OBJ)
	) {
		return original_zend_std_get_property_ptr_ptr(object, member, type, cache_slot);
	}

	// Call original to not have a notice
	original_zend_std_get_property_ptr_ptr(object, member, type, cache_slot);

	return NULL;
}

#if PHP_VERSION_ID >= 70400
zval* aspekt_write_property(zval *object, zval *member, zval *value, void **cache_slot) /* {{{ */
#else
void aspekt_write_property(zval *object, zval *member, zval *value, void **cache_slot)
#endif
{
	zval aspekt_object;
	aspekt_joinpoint_object_t *joinpoint;
	zend_array *pointcut_table = NULL;
	HashPosition pos;

	if (ASPEKT_G(lock_write_property) > 25) {
		zend_error(E_ERROR, "Too many level of nested advices. Are there any recursive call ?");
#if PHP_VERSION_ID >= 70400
		return value;
#else
		return;
#endif
	}

	pointcut_table = get_cache_property(object, member, ASPEKT_KIND_WRITE);
	if (pointcut_table == NULL || zend_hash_num_elements(pointcut_table) == 0) {
#if PHP_VERSION_ID >= 70400
		return original_zend_std_write_property(object, member, value, cache_slot);
#else
		original_zend_std_write_property(object, member, value, cache_slot);
		return ;
#endif
	}

	zend_hash_internal_pointer_reset_ex(pointcut_table, &pos);

	object_init_ex(&aspekt_object, aspekt_joinpoint_ce_ptr);
	joinpoint = (aspekt_joinpoint_object_t *)Z_OBJ(aspekt_object);

	joinpoint->advice = pointcut_table;
	joinpoint->ex = EG(current_execute_data);
	joinpoint->args = NULL;
	joinpoint->return_value = NULL;
	joinpoint->object = object;
	joinpoint->member = member;
	// To avoid use runtime cache
	joinpoint->cache_slot = NULL;

	ZVAL_COPY(&joinpoint->property_value, value);

	ASPEKT_G(lock_write_property)++;
	do_write_property(0, pointcut_table, &aspekt_object);
	ASPEKT_G(lock_write_property)--;

	zval_ptr_dtor(&aspekt_object);
#if PHP_VERSION_ID >= 70400
	return joinpoint->return_value;
#endif
}
/* }}} */

void aspekt_execute_ex(zend_execute_data *ex) /* {{{ */
{
	zend_function *fbc = NULL;

	fbc = ex->func;

	if (!ASPEKT_G(aspekt_enable) ||
		fbc == NULL ||
		ASPEKT_G(overloaded) ||
		EG(exception) ||
		fbc->common.function_name == NULL ||
		fbc->common.type == ZEND_EVAL_CODE ||
		fbc->common.fn_flags & ZEND_ACC_CLOSURE
	) {
		return original_zend_execute_ex(ex);
	}

	ASPEKT_G(overloaded) = 1;
	func_pointcut_and_execute(ex);
	ASPEKT_G(overloaded) = 0;
}
/* }}} */

void aspekt_execute_internal(zend_execute_data *ex, zval *return_value) /* {{{ */
{
	zend_function *fbc = NULL;

	fbc = ex->func;

	if (!ASPEKT_G(aspekt_enable) ||
		fbc == NULL ||
		ASPEKT_G(overloaded) ||
		EG(exception) ||
		fbc->common.function_name == NULL
	) {
		if (original_zend_execute_internal) {
			return original_zend_execute_internal(ex, return_value);
		}

		return execute_internal(ex, return_value);
	}

	ex->return_value = return_value;

	ASPEKT_G(overloaded) = 1;
	func_pointcut_and_execute(ex);
	ASPEKT_G(overloaded) = 0;
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
