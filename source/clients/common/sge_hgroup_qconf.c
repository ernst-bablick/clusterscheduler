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

#include "sge.h"
#include "sgermon.h"
#include "sge_conf.h"
#include "spool/classic/read_write_host_group.h"
#include "sge_log.h"
#include "sge_gdi.h"
#include "sge_unistd.h"

#include "sge_answer.h"
#include "sge_edit.h"
#include "sge_hgroup.h"
#include "sge_hgroup_qconf.h"

#include "msg_common.h"

bool 
hgroup_add_del_mod_via_gdi(lListElem *this_elem, lList **answer_list,
                           u_long32 gdi_command)
{
   bool ret = false;

   DENTER(TOP_LAYER, "hgroup_add_del_mod_via_gdi");
   if (this_elem != NULL) {
      lList *hgroup_list = NULL;
      lList *gdi_answer_list = NULL;

      hgroup_list = lCreateList("", HGRP_Type);
      lAppendElem(hgroup_list, this_elem);
      gdi_answer_list = sge_gdi(SGE_HGROUP_LIST, gdi_command,
                                &hgroup_list, NULL, NULL);
      answer_list_replace(answer_list, &gdi_answer_list);
   }
   DEXIT;
   return ret;
}

lListElem *hgroup_get_via_gdi(lList **answer_list, const char *name) 
{
   lListElem *ret = NULL;

   DENTER(TOP_LAYER, "hgroup_get_via_gdi");
   if (name != NULL) {
      lList *gdi_answer_list = NULL;
      lEnumeration *what = NULL;
      lCondition *where = NULL;
      lList *houstgroup_list = NULL;
      dstring string = DSTRING_INIT;

      correct_hgroup_name(&string, name);
      what = lWhat("%T(ALL)", HGRP_Type);
      where = lWhere("%T(%I==%s)", HGRP_Type, HGRP_name, 
                     sge_dstring_get_string(&string));
      gdi_answer_list = sge_gdi(SGE_HGROUP_LIST, SGE_GDI_GET, 
                                &houstgroup_list, where, what);
      what = lFreeWhat(what);
      where = lFreeWhere(where);

      if (!answer_list_has_error(&gdi_answer_list)) {
         ret = lFirst(houstgroup_list);
      } else {
         answer_list_replace(answer_list, &gdi_answer_list);
      }
      sge_dstring_free(&string);
   } 
   DEXIT;
   return ret;
}

bool hgroup_provide_modify_context(lListElem **this_elem, lList **answer_list)
{
   bool ret = false;
   int status = 0;
   
   DENTER(TOP_LAYER, "hgroup_provide_modify_context");
   if (this_elem != NULL && *this_elem) {
      char *filename = write_host_group(2, 1, *this_elem); 
 
      status = sge_edit(filename);
      if (status >= 0) {
         lListElem *hgroup;

         hgroup = cull_read_in_host_group(NULL, filename, 1, 0, 0, NULL);
         if (hgroup != NULL) {
            *this_elem = lFreeElem(*this_elem);
            *this_elem = hgroup; 
            ret = true;
         } else {
            answer_list_add(answer_list, MSG_FILE_ERRORREADINGINFILE,
                            STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         }
      } else {
         answer_list_add(answer_list, MSG_PARSE_EDITFAILED,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
      }
      unlink(filename);
   } 
   DEXIT;
   return ret;
}

bool hgroup_add(lList **answer_list, const char *name) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_add");
   if (name != NULL) {
      lListElem *hgroup = hgroup_create(answer_list, name, NULL);

      if (hgroup == NULL) {
         ret = false;
      }
      if (ret) {
         ret &= hgroup_provide_modify_context(&hgroup, answer_list);
      }
      if (ret) {
         ret &= hgroup_add_del_mod_via_gdi(hgroup, answer_list, 
                                              SGE_GDI_ADD); 
      } 
   }  
  
   DEXIT;
   return ret; 
}

bool hgroup_add_from_file(lList **answer_list, const char *filename) 
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_add");
   if (filename != NULL) {
      lListElem *hgroup;

      hgroup = cull_read_in_host_group(NULL, filename, 1, 0, 0, NULL); 
      if (hgroup == NULL) {
         ret = false;
      }
      if (ret) {
         ret &= hgroup_add_del_mod_via_gdi(hgroup, answer_list, SGE_GDI_ADD); 
      } 
   }  
  
   DEXIT;
   return ret; 
}

bool hgroup_modify(lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_modify");
   if (name != NULL) {
      lListElem *hgroup = hgroup_get_via_gdi(answer_list, name);

      if (hgroup == NULL) {
         /* EB: TODO move to msg file */
         sprintf(SGE_EVENT, "Host group "SFQ" does not exist\n", name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         ret = false;
      }
      if (ret) {
         ret &= hgroup_provide_modify_context(&hgroup, answer_list);
      }
      if (ret) {
         ret &= hgroup_add_del_mod_via_gdi(hgroup, answer_list, SGE_GDI_MOD);
      }
      if (hgroup) {
         hgroup = lFreeElem(hgroup);
      }
   }

   DEXIT;
   return ret;
}

bool hgroup_modify_from_file(lList **answer_list, const char *filename)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_modify");
   if (filename != NULL) {
      lListElem *hgroup;

      hgroup = cull_read_in_host_group(NULL, filename, 1, 0, 0, NULL); 
      if (hgroup == NULL) {
         /* EB: TODO move to msg file */
         sprintf(SGE_EVENT, "Host group file "SFQ" is not correct\n", filename);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR);
         ret = false;
      }
      if (ret) {
         ret &= hgroup_add_del_mod_via_gdi(hgroup, answer_list, SGE_GDI_MOD);
      }
      if (hgroup) {
         hgroup = lFreeElem(hgroup);
      }
   }

   DEXIT;
   return ret;
}

bool hgroup_delete(lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_delete");
   if (name != NULL) {
      lListElem *hgroup = hgroup_create(answer_list, name, NULL); 
   
      if (hgroup != NULL) {
         ret &= hgroup_add_del_mod_via_gdi(hgroup, answer_list, SGE_GDI_DEL); 
      }
   }
   DEXIT;
   return ret;
}

bool hgroup_show(lList **answer_list, const char *name)
{
   bool ret = true;

   DENTER(TOP_LAYER, "hgroup_show");
   if (name != NULL) {
      lListElem *hgroup = hgroup_get_via_gdi(answer_list, name); 
   
      if (hgroup != NULL) {
         write_host_group(0, 0, hgroup);
         hgroup = lFreeElem(hgroup);
      } else {
         /* EB: TODO move to msg file */
         sprintf(SGE_EVENT, "Host group "SFQ" does not exist\n", name);
         answer_list_add(answer_list, SGE_EVENT,
                         STATUS_ERROR1, ANSWER_QUALITY_ERROR); 
         ret = false;
      }
   }
   DEXIT;
   return ret;
}


