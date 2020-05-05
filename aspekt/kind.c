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

#include "kind.h"

zend_object_handlers aspekt_kind_handlers;
ASPEKT_API zend_class_entry *aspekt_kind_ce_ptr;

static zend_always_inline aspekt_kind_object_t *fetch_object(zend_object *obj) /* {{{ */
{
	return (aspekt_kind_object_t *)((char *)(obj) - XtOffsetOf(aspekt_kind_object_t, std));
}
/* }}} */

static zend_object *create_object(zend_class_entry *ce_ptr) /* {{{ */
{
	aspekt_kind_object_t *intern;

	intern = ecalloc(1, sizeof(aspekt_kind_object_t) + zend_object_properties_size(ce_ptr));

	zend_object_std_init(&intern->std, ce_ptr);
	intern->std.handlers = &aspekt_kind_handlers;

	return &intern->std;
}
/* }}} */

static void object_free(zend_object *object) /* {{{ */
{
	aspekt_kind_object_t *intern = fetch_object(object);
	zend_object_std_dtor(&intern->std);
}
/* }}} */

/* {{{ aspekt_Kind_init
   Create and register 'Aspekt\Kind' class. */
ASPEKT_INIT_CLASS(Kind) {
	ASPEKT_REGISTER_CLASS(Aspekt, Kind, aspekt, kind, aspekt_kind_method_entry, ZEND_ACC_FINAL);

	aspekt_kind_ce_ptr->create_object = create_object;

	memcpy(&aspekt_kind_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* offset of real object header (usually zero) */
	aspekt_kind_handlers.offset = (int) XtOffsetOf(aspekt_kind_object_t, std);

	/* general object functions */
	aspekt_kind_handlers.free_obj  = object_free;
	aspekt_kind_handlers.dtor_obj  = zend_objects_destroy_object;
	aspekt_kind_handlers.clone_obj = NULL; /* has no clone implementation */

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AROUND"), ASPEKT_KIND_AROUND);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("BEFORE"), ASPEKT_KIND_BEFORE);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AFTER"), ASPEKT_KIND_AFTER);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("READ"), ASPEKT_KIND_READ);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("WRITE"), ASPEKT_KIND_WRITE);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("PROPERTY"), ASPEKT_KIND_PROPERTY);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("METHOD"), ASPEKT_KIND_METHOD);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("FUNCTION"), ASPEKT_KIND_FUNCTION);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AROUND_READ_PROPERTY"), ASPEKT_KIND_AROUND_READ_PROPERTY);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("BEFORE_READ_PROPERTY"), ASPEKT_KIND_BEFORE_READ_PROPERTY);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AFTER_READ_PROPERTY"), ASPEKT_KIND_AFTER_READ_PROPERTY);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AROUND_WRITE_PROPERTY"), ASPEKT_KIND_AROUND_WRITE_PROPERTY);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("BEFORE_WRITE_PROPERTY"), ASPEKT_KIND_BEFORE_WRITE_PROPERTY);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AFTER_WRITE_PROPERTY"), ASPEKT_KIND_AFTER_WRITE_PROPERTY);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AROUND_METHOD"), ASPEKT_KIND_AROUND_METHOD);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("BEFORE_METHOD"), ASPEKT_KIND_BEFORE_METHOD);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AFTER_METHOD"), ASPEKT_KIND_AFTER_METHOD);

	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AROUND_FUNCTION"), ASPEKT_KIND_AROUND_FUNCTION);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("BEFORE_FUNCTION"), ASPEKT_KIND_BEFORE_FUNCTION);
	aspekt_declare_class_constant_long(aspekt_kind_ce_ptr, ZEND_STRL("AFTER_FUNCTION"), ASPEKT_KIND_AFTER_FUNCTION);

	return SUCCESS;
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
