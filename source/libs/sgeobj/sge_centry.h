#ifndef __SGE_CENTRY_H 
#define __SGE_CENTRY_H 
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sge_complexL.h"

extern lList *Master_CEntry_List;

const char *
map_op2str(u_long32 op);

const char *
map_type2str(u_long32 type);

const char *
map_req2str(u_long32 op);

lListElem *
centry_create(lList **answer_list, 
              const char *name);

bool
centry_is_referenced(const lListElem *this_elem, 
                     lList **answer_list,
                     const lList *master_queue_list,
                     const lList *master_exechost_list,
                     const lList *master_sconf_list);

bool
centry_print_resource_to_dstring(const lListElem *this_elem, 
                                 dstring *string);

lList **
centry_list_get_master_list(void);

lListElem *
centry_list_locate(const lList *this_list, 
                   const char *name);

bool
centry_list_sort(lList *this_list);

bool
centry_list_init_double(lList *this_list);

int
centry_list_fill_request(lList *centry_list, lList *master_centry_list,
                         bool allow_non_requestable, bool allow_empty_boolean,
                         bool allow_neg_consumable);

bool
centry_list_are_queues_requestable(const lList *this_list);

#endif /* __SGE_CENTRY_H */

