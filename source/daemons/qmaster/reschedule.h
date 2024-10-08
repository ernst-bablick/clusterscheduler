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

#include "sge_qmaster_timed_event.h"
#include "uti/sge_monitor.h"
#include "sgeobj/sge_daemonize.h"

#define RESCHEDULE_SKIP_JR_REMOVE      0x00000000
#define RESCHEDULE_SKIP_JR_SEND_ACK    0x00000001
#define RESCHEDULE_SKIP_JR             0x00000002
#define RESCHEDULE_HANDLE_JR_REMOVE    0x00000004
#define RESCHEDULE_HANDLE_JR_WAIT      0x00000008

void
reschedule_unknown_event(te_event_t anEvent, monitoring_t *monitor);

u_long32
skip_restarted_job(lListElem *host, lListElem *job_report, u_long32 job_number, u_long32 task_number);

int
reschedule_jobs(lListElem *ep, u_long32 force, lList **answer, monitoring_t *monitor,
                bool is_manual, u_long64 gdi_session);

int
reschedule_job(lListElem *jep, lListElem *jatep, lListElem *qep, u_long32 force,
               lList **answer, monitoring_t *monitor, bool is_manual, u_long64 gdi_session);

lListElem *
add_to_reschedule_unknown_list(lListElem *hostr, u_long32 job_number, u_long32 task_number,
                               u_long32 state, u_long64 gdi_session);

void
delete_from_reschedule_unknown_list(lListElem *host, u_long64 gdi_session);

void
update_reschedule_unknown_list(lListElem *host, u_long64 gdi_session);

void
update_reschedule_unknown_list_for_job(lListElem *host, u_long32 job_number, u_long32 task_number);

void
update_reschedule_unknown_timout_values(const char *config_name);

void
reschedule_unknown_trigger(lListElem *hep);

void
reschedule_add_additional_time(u_long64 time);

void
remove_from_reschedule_unknown_list(lListElem *host, u_long32 job_number, u_long32 task_number, u_long64 gdi_session);

void
remove_from_reschedule_unknown_lists(u_long32 job_number, u_long32 task_number, u_long64 gdi_session);
