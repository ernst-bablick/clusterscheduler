#ifndef __RMON_WAIT_LIST_H
#define __RMON_WAIT_LIST_H
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

#include <sys/types.h>



#include "rmon_def.h"
#include "rmon_monitoring_level.h"

typedef struct wait_list_type {
   monitoring_level level;
   string programname;
   u_long client;
   struct wait_list_type *next;
} wait_list_type;

extern wait_list_type *wait_list;

int rmon_insert_wl(wait_list_type *);
wait_list_type **rmon_search_wl_for_client(u_long client);
wait_list_type **rmon_search_wl_for_client_and_spy(u_long client, char *programname);
wait_list_type **rmon_search_wl_for_spy(char *programname);
wait_list_type *rmon_unchain_wl(wait_list_type **wp);
int rmon_delete_wl(wait_list_type **wlp);
void rmon_print_wl(wait_list_type *);
void rmon_print_wait(wait_list_type *);

#endif /* __RMON_WAIT_LIST_H */



