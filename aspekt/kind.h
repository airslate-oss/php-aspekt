/*
 * This file is part of the Aspekt.
 *
 * (c) airSlate Inc. <support@airslate.com>
 *
 * For the full copyright and license information, please view
 * the LICENSE file that was distributed with this source code.
 */

#ifndef ASPEKT_KIND_H
#define ASPEKT_KIND_H

#include <Zend/zend_API.h>

#include "../php_aspekt.h"

extern ASPEKT_API zend_class_entry *aspekt_kind_ce_ptr;
extern zend_object_handlers aspekt_kind_handlers;

typedef struct { /* {{{ */
	zend_object std;
} aspekt_kind_object_t;
/* }}} */

ASPEKT_INIT_CLASS(Kind);

static const zend_function_entry aspekt_kind_method_entry[] = { /* {{{ */
	PHP_FE_END
};
/* }}} */

#endif // ASPEKT_KIND_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
