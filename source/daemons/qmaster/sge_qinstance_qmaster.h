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
 *  Portions of this software are Copyright (c) 2023-2024 HPC-Gridware GmbH
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include "basis_types.h"
#include "cull/cull.h"
#include "sgeobj/sge_daemonize.h"

bool
qinstance_change_state_on_command(lListElem *this_elem, lList **answer_list,
                                  u_long32 transition, bool force_transition, const char *user, const char *host,
                                  bool is_operator, bool is_owner, monitoring_t *monitor, u_long64 gdi_session);

bool
qinstance_change_state_on_calendar(lListElem *this_elem, const lListElem *calendar, monitoring_t *monitor, u_long64 gdi_session);

bool
qinstance_modify_attribute(lListElem *this_elem, lList **answer_list, const lListElem *cqueue,
                           int attribute_name, int cqueue_attibute_name, int sub_host_name, int sub_value_name,
                           int subsub_key, const char **matching_host_or_group, const char **matching_group,
                           bool *is_ambiguous, bool *has_changed_conf_attr, bool *has_changed_state_attr,
                           const bool initial_modify, bool *need_reinitialize, monitoring_t *monitor,
                           const lList *master_hgroup_list, lList *master_cqueue_list, u_long64 gdi_session);

bool
qinstance_change_state_on_calendar_all(const char *cal_name, u_long32 cal_order,
                                       const lList *state_change_list, monitoring_t *monitor, u_long64 gdi_session);

bool
sge_qmaster_qinstance_state_set_manual_disabled(lListElem *this_elem, bool set_state, u_long64 gdi_session);

bool
sge_qmaster_qinstance_state_set_manual_suspended(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_unknown(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_error(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_susp_on_sub(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_cal_disabled(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_cal_suspended(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_orphaned(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_state_set_ambiguous(lListElem *this_elem, bool set_state, u_long64 gid_session);

bool
sge_qmaster_qinstance_set_initial_state(lListElem *this_elem, u_long64 gdi_session);

bool
qinstance_reinit_consumable_actual_list(lListElem *this_elem, lList **answer_list);
