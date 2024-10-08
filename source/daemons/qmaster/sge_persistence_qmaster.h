#pragma once
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Portions of this software are Copyright (c) 2024 HPC-Gridware GmbH
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "sgeobj/sge_event.h"
#include "sge_qmaster_timed_event.h"
#include "uti/sge_monitor.h"
#include "sgeobj/sge_daemonize.h"


bool
sge_initialize_persistence(lList **answer_list);

bool
sge_shutdown_persistence(lList **answer_list);

void
sge_initialize_persistance_timer();

void
spooling_trigger_handler(te_event_t anEvent, monitoring_t *monitor);

bool
sge_event_spool(lList **answer_list, u_long64 timestamp, ev_event type,
                u_long32 intkey1, u_long32 intkey2, const char *strkey, 
                const char *strkey2, const char *session, lListElem *object, 
                lListElem *sub_object1, lListElem *sub_object2, 
                bool send_event, bool spool, u_long64 gdi_session);
