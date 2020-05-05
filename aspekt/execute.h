/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef ASPEKT_EXECUTE_H
#define ASPEKT_EXECUTE_H

#include <Zend/zend_types.h>

extern zend_object_read_property_t original_zend_std_read_property;
extern zend_object_write_property_t original_zend_std_write_property;
extern zend_object_get_property_ptr_ptr_t original_zend_std_get_property_ptr_ptr;

extern void (*original_zend_execute_ex)(zend_execute_data *execute_data);
extern void (*original_zend_execute_internal)(zend_execute_data *execute_data, zval *return_value);

void aspekt_execute_ex(zend_execute_data *execute_data);
void aspekt_execute_internal(zend_execute_data *execute_data, zval *return_value);

void do_func_execute(HashPosition pos, zend_array *pointcut_table, zend_execute_data *ex, zval *object);
void do_read_property(HashPosition pos, zend_array *pointcut_table, zval *aspekt_object);
void do_write_property(HashPosition pos, zend_array *pointcut_table, zval *aspekt_object);

#if PHP_VERSION_ID >= 70400
zval *aspekt_write_property(zval *object, zval *member, zval *value, void **cache_slot);
#else
void aspekt_write_property(zval *object, zval *member, zval *value, void **cache_slot);
#endif

zval *aspekt_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv);
zval *aspekt_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot);

#endif // ASPEKT_EXECUTE_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
