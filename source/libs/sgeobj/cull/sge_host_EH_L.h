#ifndef SGE_EH_L_H
#define SGE_EH_L_H
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

#include "cull/cull.h"
#include "sgeobj/cull/sge_boundaries.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief @todo add summary
*
* @todo add description
*
*    SGE_HOST(EH_name) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_scaling_list) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_consumable_config_list) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_usage_scaling_list) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_load_list) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_lt_heard_from) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_processors) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_acl) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_xacl) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_prj) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_xprj) - @todo add summary
*    @todo add description
*
*    SGE_DOUBLE(EH_sort_value) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_reuse_me) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_tagged) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_load_correction_factor) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_seq_no) - @todo add summary
*    @todo add description
*
*    SGE_STRING(EH_real_name) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_sge_load) - @todo add summary
*    @todo add description
*
*    SGE_DOUBLE(EH_sge_ticket_pct) - @todo add summary
*    @todo add description
*
*    SGE_DOUBLE(EH_sge_load_pct) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_featureset_id) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_scaled_usage_list) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_scaled_usage_pct_list) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_num_running_jobs) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_load_report_interval) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_resource_utilization) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_cached_complexes) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_cache_version) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_master_host) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_reschedule_unknown) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_reschedule_unknown_list) - @todo add summary
*    @todo add description
*
*    SGE_ULONG(EH_report_seqno) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_report_variables) - @todo add summary
*    @todo add description
*
*    SGE_LIST(EH_merged_report_variables) - @todo add summary
*    @todo add description
*
*/

enum {
   EH_name = EH_LOWERBOUND,
   EH_scaling_list,
   EH_consumable_config_list,
   EH_usage_scaling_list,
   EH_load_list,
   EH_lt_heard_from,
   EH_processors,
   EH_acl,
   EH_xacl,
   EH_prj,
   EH_xprj,
   EH_sort_value,
   EH_reuse_me,
   EH_tagged,
   EH_load_correction_factor,
   EH_seq_no,
   EH_real_name,
   EH_sge_load,
   EH_sge_ticket_pct,
   EH_sge_load_pct,
   EH_featureset_id,
   EH_scaled_usage_list,
   EH_scaled_usage_pct_list,
   EH_num_running_jobs,
   EH_load_report_interval,
   EH_resource_utilization,
   EH_cached_complexes,
   EH_cache_version,
   EH_master_host,
   EH_reschedule_unknown,
   EH_reschedule_unknown_list,
   EH_report_seqno,
   EH_report_variables,
   EH_merged_report_variables
};

LISTDEF(EH_Type)
   SGE_HOST(EH_name, CULL_PRIMARY_KEY | CULL_UNIQUE | CULL_HASH | CULL_SPOOL)
   SGE_LIST(EH_scaling_list, HS_Type, CULL_SPOOL)
   SGE_LIST(EH_consumable_config_list, CE_Type, CULL_SPOOL)
   SGE_LIST(EH_usage_scaling_list, HS_Type, CULL_SPOOL)
   SGE_LIST(EH_load_list, HL_Type, CULL_SPOOL)
   SGE_ULONG(EH_lt_heard_from, CULL_DEFAULT)
   SGE_ULONG(EH_processors, CULL_SPOOL)
   SGE_LIST(EH_acl, US_Type, CULL_SPOOL)
   SGE_LIST(EH_xacl, US_Type, CULL_SPOOL)
   SGE_LIST(EH_prj, PR_Type, CULL_SPOOL)
   SGE_LIST(EH_xprj, PR_Type, CULL_SPOOL)
   SGE_DOUBLE(EH_sort_value, CULL_DEFAULT)
   SGE_ULONG(EH_reuse_me, CULL_DEFAULT)
   SGE_ULONG(EH_tagged, CULL_DEFAULT)
   SGE_ULONG(EH_load_correction_factor, CULL_DEFAULT)
   SGE_ULONG(EH_seq_no, CULL_DEFAULT)
   SGE_STRING(EH_real_name, CULL_DEFAULT)
   SGE_ULONG(EH_sge_load, CULL_DEFAULT)
   SGE_DOUBLE(EH_sge_ticket_pct, CULL_DEFAULT)
   SGE_DOUBLE(EH_sge_load_pct, CULL_DEFAULT)
   SGE_ULONG(EH_featureset_id, CULL_DEFAULT)
   SGE_LIST(EH_scaled_usage_list, UA_Type, CULL_DEFAULT)
   SGE_LIST(EH_scaled_usage_pct_list, UA_Type, CULL_DEFAULT)
   SGE_ULONG(EH_num_running_jobs, CULL_DEFAULT)
   SGE_ULONG(EH_load_report_interval, CULL_DEFAULT)
   SGE_LIST(EH_resource_utilization, RUE_Type, CULL_DEFAULT)
   SGE_LIST(EH_cached_complexes, CE_Type, CULL_DEFAULT)
   SGE_ULONG(EH_cache_version, CULL_DEFAULT)
   SGE_ULONG(EH_master_host, CULL_DEFAULT)
   SGE_ULONG(EH_reschedule_unknown, CULL_DEFAULT)
   SGE_LIST(EH_reschedule_unknown_list, RU_Type, CULL_DEFAULT)
   SGE_ULONG(EH_report_seqno, CULL_DEFAULT)
   SGE_LIST(EH_report_variables, STU_Type, CULL_SPOOL)
   SGE_LIST(EH_merged_report_variables, STU_Type, CULL_DEFAULT)
LISTEND

NAMEDEF(EHN)
   NAME("EH_name")
   NAME("EH_scaling_list")
   NAME("EH_consumable_config_list")
   NAME("EH_usage_scaling_list")
   NAME("EH_load_list")
   NAME("EH_lt_heard_from")
   NAME("EH_processors")
   NAME("EH_acl")
   NAME("EH_xacl")
   NAME("EH_prj")
   NAME("EH_xprj")
   NAME("EH_sort_value")
   NAME("EH_reuse_me")
   NAME("EH_tagged")
   NAME("EH_load_correction_factor")
   NAME("EH_seq_no")
   NAME("EH_real_name")
   NAME("EH_sge_load")
   NAME("EH_sge_ticket_pct")
   NAME("EH_sge_load_pct")
   NAME("EH_featureset_id")
   NAME("EH_scaled_usage_list")
   NAME("EH_scaled_usage_pct_list")
   NAME("EH_num_running_jobs")
   NAME("EH_load_report_interval")
   NAME("EH_resource_utilization")
   NAME("EH_cached_complexes")
   NAME("EH_cache_version")
   NAME("EH_master_host")
   NAME("EH_reschedule_unknown")
   NAME("EH_reschedule_unknown_list")
   NAME("EH_report_seqno")
   NAME("EH_report_variables")
   NAME("EH_merged_report_variables")
NAMEEND

#define EH_SIZE sizeof(EHN)/sizeof(char *)

#ifdef __cplusplus
}
#endif

#endif