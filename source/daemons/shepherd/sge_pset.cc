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

#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#if defined(SOLARIS64) || defined(SOLARISAMD64)
#   include <sys/pset.h>
#endif

#if defined(LINUX)
#   include <dlfcn.h>
#endif

#include "uti/sge_uidgid.h"
#include "uti/sge_nprocs.h"
#include "uti/config_file.h"
#include "uti/sge_stdio.h"

#include "sge_pset.h"
#include "basis_types.h"
#include "execution_states.h"
#include "err_trace.h"

#define PROC_SET_OK            0
#define PROC_SET_WARNING       1
#define PROC_SET_ERROR        -1
#define PROC_SET_BUSY         -2

#if defined(SOLARIS64) || defined(SOLARISAMD64)
static int free_processor_set(char *err_str);
static int set_processor_range(char *crange, int proc_set_num, char *err_str);
#endif

void sge_pset_create_processor_set() 
{
#if defined(SOLARIS64) || defined(SOLARISAMD64)
   char err_str[2*SGE_PATH_MAX+128];

   /* processor set stuff */
   if (strcasecmp("UNDEFINED",get_conf_val("processors"))) {
      int ret;

      sge_switch2start_user();
      if ((ret=set_processor_range(get_conf_val("processors"),
                 (int) strtol(get_conf_val("job_id"), nullptr, 10),
                 err_str)) != PROC_SET_OK) {
         sge_switch2admin_user();
         if (ret == PROC_SET_WARNING) /* not critical - e.g. not root */
            shepherd_trace("warning: processor set not set in set_processor_range");
         else { /* critical --> use err_str to indicate error */
            shepherd_trace("critical error in set_processor_range - bailing out");
            shepherd_state = SSTATE_PROCSET_NOTSET;
            shepherd_error(1, err_str);
         }
      } else {
         sge_switch2admin_user();
      }
   }
#endif

}

void sge_pset_free_processor_set()
{
#if defined(SOLARIS64) || defined(SOLARISAMD64)
   /* processor set stuff */
   if (strcasecmp("UNDEFINED",get_conf_val("processors"))) {
      char err_str[2*SGE_PATH_MAX+128];
      int ret;

      sge_switch2start_user();
      if ((ret=free_processor_set(err_str)) != PROC_SET_OK) {
         sge_switch2admin_user();
         switch (ret) {
         case PROC_SET_WARNING: /* not critical - e.g. not root */
            shepherd_trace("warning: processor set not freed in free_processor_set - "
                           "did no exist, probably");
            break;
         case PROC_SET_ERROR: /* critical - err_str indicates error */
            shepherd_trace("critical error in free_processor_set - bailing out");
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         case PROC_SET_BUSY: /* still processes running in processor set */
            shepherd_trace("error in releasing processor set");
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         default: /* should not occur */
            sprintf(err_str,
               "internal error after free_processor_set - ret=%d", ret);
            shepherd_state = SSTATE_PROCSET_NOTFREED;
            shepherd_error(1, err_str);
            break;
         }
      } else {
         sge_switch2admin_user();
      }
   }
#endif
}

#if defined(SOLARIS64) || defined(SOLARISAMD64)
/****** shepherd/pset/set_processor_range() ***********************************
*  NAME
*     set_processor_range() -- sets processor range according to string 
*
*  SYNOPSIS
*     int set_processor_range(char *crange, 
*                             int proc_set_num, 
*                             char *err_str) 
*
*  FUNCTION
*     Sets processor range according to string specification.
*     The unique processor set number will be stored in the file
*     "processor_set_number" located in the current working directory.
*
*     Format:
*        n|[n][-[m]],...  , n,m  being int >= 0.
*        no blanks are allowed in between (this is supposed to be 
*        handled by the queue configuration parsing routine)
*
*  INPUTS
*     char *crange     - String specifier of the range. Will be 
*                        modified via strtok internally.
*     int proc_set_num - The base for a unique processor set number.
*                        This number is already supposed to be unique.
*                        for the job (currently the job_id).
*                        set_processor_range() manipulates it to make
*                        sure that it is a unique processor set number. 
*     char *err_str    - The error message string to be used by the
*                        calling routine retuns value != PROC_SET_OK 
*                        Also used for trace messages internally.
*                        Passed to invoked subroutines.
*
*  RESULT
*     int - error state
*        PROC_SET_OK      - Ok
*        PROC_SET_ERROR   - A critical error occurred; either during 
*                           execution of sysmp() calls or as returned 
*                           from range2proc_vec().
*        PROC_SET_WARNING - A non-critical error occurred (e.g. the 
*                           procedure is executed as unpriveliged user)
******************************************************************************/
static int set_processor_range(char *crange, int proc_set_num, char *err_str) 
{
   FILE *fp;

#if defined(SOLARIS64) || defined(SOLARISAMD64)
   /*
    * We do not create a processor set here
    * The system administrator is responsible to do this
    * We read one id from crange. This is the processor-set id we should use.
    */
   if (crange) {
      char *tok, *next;

      if ((tok=strtok(crange, " \t\n"))) {
         proc_set_num = (int) strtol(tok, &next, 10);
         if (next == tok) {
            sprintf(err_str, "wrong processor set id format: %20.20s", crange);
            shepherd_trace(err_str);
            return PROC_SET_ERROR;
         }
      } 
   }
#endif

   /* dump to file for later use */
   if ((fp = fopen("processor_set_number","w"))) {
      fprintf(fp,"%d\n",proc_set_num);
      FCLOSE(fp);
   } else {
      shepherd_trace("MPPS_CREATE: failed creating file processor_set_number");
      return PROC_SET_ERROR;
   }

#if defined(SOLARIS64) || defined(SOLARISAMD64)
   if (proc_set_num) {
      int local_ret;

      sprintf(err_str,"pset_bind: try to use processorset %d", proc_set_num);
      shepherd_trace(err_str);
      if (pset_bind(proc_set_num, P_PID, P_MYID, nullptr)) {
         switch (errno) {
         case EFAULT:
            shepherd_trace("pset_bind: The location pointed to by opset was not"
               " nullptr and not writable by the user");
            local_ret = PROC_SET_ERROR;
            break;
         case EINVAL:
            shepherd_trace("pset_bind: invalid processor set was specified");
            local_ret = PROC_SET_ERROR;
            break;
         case EPERM:
            shepherd_trace("pset_bind: The effective user of the calling "
               "process is not super-user");
            local_ret = PROC_SET_ERROR;
            break;
         default:
            sprintf(err_str,"pset_bind: unexpected error - errno=%d", errno);
            shepherd_trace(err_str);
            local_ret = PROC_SET_ERROR;
            break;
         }
         return local_ret;
      }
   }
#endif

   return PROC_SET_OK;
FCLOSE_ERROR:
   shepherd_trace("MPPS_CREATE: failed creating file processor_set_number");
   return PROC_SET_ERROR;
}

/****** shepherd/pset/free_processor_set() ************************************
*  NAME
*     free_processor_set() -- Release the previously occupied proc set. 
*
*  SYNOPSIS
*     int free_processor_set(char *err_str) 
*
*  FUNCTION
*     Release the previously occupied processor set. The unique 
*     processor set number is read from the file "processor_set_number"
*     which has to be located in the current working directory.
*
*  INPUTS
*     char *err_str - The error message string to be used by the calling
*                     routine if return value != PROC_SET_OK. Also used
*                     for trace messages internally 
*
*  RESULT
*     int - Error state
*        PROC_SET_OK      - Ok
*        PROC_SET_BUSY    - The processor set is still in use, i.e.
*                           processes originating from the job have not 
*                           finished.
*        PROC_SET_ERROR   - A critical error occurred. During execution
*                           of sysmp() calls.
*        PROC_SET_WARNING - A non-critical error occurred (e.g. the
*                           procedure is executed as unpriviliged user)
******************************************************************************/
static int free_processor_set(char *err_str) 
{
   FILE *fp;
   int proc_set_num;

   /* read unique processor set number from file */
   if ((fp = fopen("processor_set_number","r"))) {
      fscanf(fp, "%d", &proc_set_num);
      FCLOSE_IGNORE_ERROR(fp);
   } else {
      shepherd_trace("MPPS_CREATE: failed reading from file processor_set_number");
      return PROC_SET_ERROR;
   }

#if defined(SOLARIS64) || defined(SOLARISAMD64)
   /*
    * We do not release a processor set here
    * The system administrator is responsible to do this
    */
#endif
   return PROC_SET_OK;
}

#endif 
