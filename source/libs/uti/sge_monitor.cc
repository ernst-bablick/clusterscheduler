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
 *   Copyright: 2003 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 *  Portions of this software are Copyright (c) 2023-2024 HPC-Gridware GmbH
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <dlfcn.h>

#if defined(LINUX) || defined(SOLARIS)

#  include <malloc.h>

#endif

#include "uti/msg_utilib.h"
#include "uti/sge_dstring.h"
#include "uti/sge_log.h"
#include "uti/sge_monitor.h"
#include "uti/sge_mtutil.h"
#include "uti/sge_rmon_macros.h"
#include "uti/sge_time.h"


/*************************************************
 * 
 * structure definition to hand the threading 
 * information over to the comlib
 *
 **************************************************/

typedef struct {
   const char *name;           /* thread name */
   struct timeval last_wait_time;  /* last wait time, last time when one thread loop finished */
   long warning_timeout; /* how long can the thread be blocked before a warning is shown */
   long error_timeout;   /* how long can the thread be blocked before an error is shown */
   time_t update_time;     /* last update time */
   dstring *output;          /* thread specific info line */
   pthread_mutex_t Output_Mutex;    /* guards one line */
} Output_t;

#define MAX_OUTPUT_LINES 512           /* max number of threads to monitor at the same time */
static bool Output_initialized = false;
static Output_t Output[MAX_OUTPUT_LINES];

/* global mutex used for mallinfo initialisation and also used to access the Info_Line string */
static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

/* a static dstring used as a temporary buffer to build the commlib info string */
static dstring Info_Line = DSTRING_INIT;

/* mallinfo related data */
#if defined(LINUX) || defined(SOLARIS)
static bool mallinfo_initialized = false;
static void *mallinfo_shlib_handle = nullptr;

static struct mallinfo (*mallinfo_func_pointer)() = nullptr;

#endif

/***********************************************
 * static function def. for special extensions
 ***********************************************/

static void ext_gdi_output(dstring *message, void *monitoring_extension, double time);

static void ext_lis_output(dstring *message, void *monitoring_extension, double time);

static void ext_edt_output(dstring *message, void *monitoring_extension, double time);

static void ext_tet_output(dstring *message, void *monitoring_extension, double time);

static void ext_sch_output(dstring *message, void *monitoring_extension, double time);

/************************************************
 * implementation
 ************************************************/


/****** uti/monitor/sge_monitor_free() *****************************************
*  NAME
*     sge_monitor_free() -- frees the monitoring data structure
*
*  SYNOPSIS
*     void sge_monitor_free(monitoring_t *monitor) 
*
*  FUNCTION
*     removes the line for the commlib output, and frees memory in the 
*     monitoring structure
*
*  INPUTS
*     monitoring_t *monitor - monitoring structure
*
*  NOTES
*     MT-NOTE: sge_monitor_free() is MT safe 
*
*******************************************************************************/
void sge_monitor_free(monitoring_t *monitor) {
   DENTER(GDI_LAYER);

   sge_dstring_free(monitor->output_line1);
   sge_dstring_free(monitor->output_line2);
   sge_free(&(monitor->output_line1));
   sge_free(&(monitor->output_line2));
   sge_free(&(monitor->ext_data));

   if (monitor->pos != -1) {
      sge_mutex_lock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));

      Output[monitor->pos].output = nullptr;
      Output[monitor->pos].name = nullptr;
      Output[monitor->pos].warning_timeout = NO_WARNING;
      Output[monitor->pos].error_timeout = NO_ERROR;

      sge_mutex_unlock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));
   }

   monitor->ext_data_size = 0;
   monitor->ext_output = nullptr;
   monitor->ext_type = NONE_EXT;
   monitor->monitor_time = 0;
   monitor->pos = -1;
   monitor->output = false;
   monitor->work_line = nullptr;
   monitor->thread_name = nullptr;

#if defined(LINUX) || defined(SOLARIS)
   sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &global_mutex);
   if (mallinfo_shlib_handle != nullptr) {
      dlclose(mallinfo_shlib_handle);
      mallinfo_shlib_handle = nullptr;
   }
   sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &global_mutex);
#endif

   DRETURN_VOID;
}

/****** uti/monitor/sge_monitor_init() *****************************************
*  NAME
*     sge_monitor_init() -- init the monitoring structure
*
*  SYNOPSIS
*     void sge_monitor_init(monitoring_t *monitor, const char *thread_name, 
*     extension_t ext, thread_warning_t warning_timeout, thread_error_t 
*     error_timeout) 
*
*  FUNCTION
*     Sets the default values and inits the structure, finds the line pos
*     for the comlib output
*
*  INPUTS
*     monitoring_t *monitor            - monitoring structure
*     const char *thread_name          - the thread name
*     extension_t ext                  - the extension time (-> enum)
*     thread_warning_t warning_timeout - the warning timeout (-> enum)
*     thread_error_t error_timeout     - the error timeout (-> enum)
*
*  NOTES
*     MT-NOTE: sge_monitor_init() is MT safe 
*
*******************************************************************************/
void
sge_monitor_init(monitoring_t *monitor, const char *thread_name, extension_t ext,
                 long warning_timeout, long error_timeout) {
   DENTER(GDI_LAYER);

   sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &global_mutex);
   if (!Output_initialized) {
      Output_initialized = true;
      for (int i = 0; i < MAX_OUTPUT_LINES; i++) {
         memset(&Output[i], 0, sizeof(Output_t));
         Output[i].Output_Mutex = PTHREAD_MUTEX_INITIALIZER;
      }
   }
   sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &global_mutex);

   /*
    * initialize the mallinfo function pointer if it is available
    */
#if defined(LINUX) || defined(SOLARIS)
   sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &global_mutex);
   if (!mallinfo_initialized) {
      const char *function_name = "mallinfo";

      mallinfo_initialized = true;
#  ifdef RTLD_NODELETE
      mallinfo_shlib_handle = dlopen(nullptr, RTLD_LAZY | RTLD_NODELETE);
#  else
      mallinfo_shlib_handle = dlopen(nullptr, RTLD_LAZY );
#  endif /* RTLD_NODELETE */

      if (mallinfo_shlib_handle != nullptr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
         mallinfo_func_pointer = (struct mallinfo (*)()) dlsym(mallinfo_shlib_handle, function_name);
#pragma GCC diagnostic pop
      }
   }
   sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &global_mutex);
#endif

   monitor->thread_name = thread_name;

   monitor->output_line1 = (dstring *) sge_malloc(sizeof(dstring));
   monitor->output_line2 = (dstring *) sge_malloc(sizeof(dstring));

   if (monitor->output_line1 == nullptr || monitor->output_line2 == nullptr) {
      CRITICAL(SFNMAX, MSG_UTI_MONITOR_MEMERROR);
      exit(1);
   }

   memset(monitor->output_line1, 0, sizeof(dstring));
   memset(monitor->output_line2, 0, sizeof(dstring));

   sge_dstring_append(monitor->output_line1, thread_name);
   sge_dstring_append(monitor->output_line1, MSG_UTI_MONITOR_NODATA);
   sge_dstring_clear(monitor->output_line2);
   monitor->work_line = monitor->output_line2;

   switch (ext) {
      case SCH_EXT :
         monitor->ext_data = sge_malloc(sizeof(m_sch_t));
         if (monitor->ext_data != nullptr) {
            monitor->ext_type = SCH_EXT;
            monitor->ext_data_size = sizeof(m_sch_t);
            monitor->ext_output = &ext_sch_output;
         } else {
            monitor->ext_type = NONE_EXT;
            ERROR(SFNMAX, MSG_UTI_MONITOR_MEMERROREXT);
         }
         break;

      case GDI_EXT :
         monitor->ext_data = sge_malloc(sizeof(m_gdi_t));
         if (monitor->ext_data != nullptr) {
            monitor->ext_type = GDI_EXT;
            monitor->ext_data_size = sizeof(m_gdi_t);
            monitor->ext_output = &ext_gdi_output;
         } else {
            monitor->ext_type = NONE_EXT;
            ERROR(SFNMAX, MSG_UTI_MONITOR_MEMERROREXT);
         }
         break;

      case LIS_EXT :
         monitor->ext_data = sge_malloc(sizeof(m_lis_t));
         if (monitor->ext_data != nullptr) {
            monitor->ext_type = LIS_EXT;
            monitor->ext_data_size = sizeof(m_lis_t);
            monitor->ext_output = &ext_lis_output;
         } else {
            monitor->ext_type = NONE_EXT;
            ERROR(SFNMAX, MSG_UTI_MONITOR_MEMERROREXT);
         }
         break;

      case EMAT_EXT :
         monitor->ext_data = sge_malloc(sizeof(m_edt_t));
         if (monitor->ext_data != nullptr) {
            monitor->ext_type = EMAT_EXT;
            monitor->ext_data_size = sizeof(m_edt_t);
            monitor->ext_output = &ext_edt_output;
         } else {
            monitor->ext_type = NONE_EXT;
            ERROR(SFNMAX, MSG_UTI_MONITOR_MEMERROREXT);
         }
         break;

      case TET_EXT :
         monitor->ext_data = sge_malloc(sizeof(m_tet_t));
         if (monitor->ext_data != nullptr) {
            monitor->ext_type = TET_EXT;
            monitor->ext_data_size = sizeof(m_tet_t);
            monitor->ext_output = &ext_tet_output;
         } else {
            monitor->ext_type = NONE_EXT;
            ERROR(SFNMAX, MSG_UTI_MONITOR_MEMERROREXT);
         }
         break;

      case NONE_EXT :
         monitor->ext_type = NONE_EXT;
         break;

      default :
         monitor->ext_type = NONE_EXT;
         ERROR(MSG_UTI_MONITOR_UNSUPPORTEDEXT_D, ext);
   }

   if (monitor->ext_type == NONE_EXT) {
      monitor->ext_data_size = 0;
      monitor->ext_data = nullptr;
      monitor->ext_output = nullptr;
   }

   sge_monitor_reset(monitor);

   {
      int i;
      struct timeval time{};
      monitor->pos = -1;

      gettimeofday(&time, nullptr);

      for (i = 0; i < MAX_OUTPUT_LINES; i++) {
         sge_mutex_lock("sge_monitor_init", __func__, __LINE__, &(Output[i].Output_Mutex));
         if (Output[i].name == nullptr) {
            monitor->pos = i;
            Output[i].output = monitor->output_line1;
            Output[i].name = thread_name;
            Output[i].warning_timeout = warning_timeout;
            Output[i].error_timeout = error_timeout;
            Output[i].update_time = time.tv_sec;
            sge_mutex_unlock("sge_monitor_init", __func__, __LINE__, &(Output[i].Output_Mutex));
            break;
         }
         sge_mutex_unlock("sge_monitor_init", __func__, __LINE__, &(Output[i].Output_Mutex));
      }

      sge_set_last_wait_time(monitor, time);
   }

   if (monitor->pos == -1) {
      ERROR(MSG_UTI_MONITOR_NOLINES_S, monitor->thread_name);
   }

   DRETURN_VOID;
}


/****** uti/monitor/sge_monitor_status() ***************************************
*  NAME
*     sge_monitor_status() -- generates the status for qping / commlib
*
*  SYNOPSIS
*     u_long32 sge_monitor_status(char **info_message, u_long32 monitor_time) 
*
*  FUNCTION
*     This method creates the health monitoring output and returns the monitoring
*     info to the commlib. 
*
*  INPUTS
*     char **info_message   - info_message pointer, has to point to a nullptr string
*     u_long32 monitor_time - the configured monitoring interval
*
*  RESULT
*     u_long32 - 0 : everything is okay
*                1 : warning
*                2 : error
*                3 : init problems
*
*  NOTES
*     MT-NOTE: sge_monitor_status() is MT safe 
*
*******************************************************************************/
u_long32 sge_monitor_status(char **info_message, u_long32 monitor_time) {
   u_long32 ret = 0;
   char date[40];
   dstring ddate;

   DENTER(GDI_LAYER);

   if (info_message == nullptr) {
      DRETURN(3);
   }

   sge_dstring_init(&ddate, date, sizeof(date));

   sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &global_mutex);

   sge_dstring_clear(&Info_Line);

   {/* this is the qping info section, it checks if each thread is still alive */
      int i;
      int error_count = 0;
      int warning_count = 0;
      struct timeval now{};
      gettimeofday(&now, nullptr);

      for (i = 0; i < MAX_OUTPUT_LINES; i++) {

         sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &(Output[i].Output_Mutex));
         if (Output[i].name != nullptr) {
            char state = 'R';
            double time = now.tv_usec - Output[i].last_wait_time.tv_usec;

            time = now.tv_sec - Output[i].last_wait_time.tv_sec + (time / 1000000);
            if (Output[i].error_timeout != NO_ERROR && Output[i].error_timeout < time) {
               state = 'E';
               error_count++;
            } else if (Output[i].warning_timeout != NO_WARNING && Output[i].warning_timeout < time) {
               state = 'W';
               warning_count++;
            }
            sge_dstring_sprintf_append(&Info_Line, MSG_UTI_MONITOR_INFO_SCF, Output[i].name, state, time);
         }
         sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &(Output[i].Output_Mutex));
      }

      if (error_count > 0) {
         sge_dstring_append(&Info_Line, MSG_UTI_MONITOR_ERROR);
         ret = 2;
      } else if (warning_count > 0) {
         sge_dstring_append(&Info_Line, MSG_UTI_MONITOR_WARNING);
         ret = 1;
      } else {
         sge_dstring_append(&Info_Line, MSG_UTI_MONITOR_OK);
      }
      sge_dstring_append(&Info_Line, "\n");
   }

#if defined(LINUX) || defined(SOLARIS)
   if (mallinfo_func_pointer != nullptr) {
      struct mallinfo mallinfo_data = mallinfo_func_pointer();

      sge_dstring_sprintf_append(&Info_Line, MSG_UTI_MONITOR_SCHEXT_UUUUUUUUUU,
                                 mallinfo_data.arena,
                                 mallinfo_data.ordblks,
                                 mallinfo_data.smblks,
                                 mallinfo_data.hblks,
                                 mallinfo_data.hblkhd,
                                 mallinfo_data.usmblks,
                                 mallinfo_data.fsmblks,
                                 mallinfo_data.uordblks,
                                 mallinfo_data.fordblks,
                                 mallinfo_data.keepcost);
      sge_dstring_append(&Info_Line, "\n");
   }
#endif


   if (monitor_time != 0) { /* generates the output monitoring output data */
      int i;
      sge_dstring_append(&Info_Line, MSG_UTI_MONITOR_COLON);
      sge_dstring_append(&Info_Line, "\n");

      for (i = 0; i < MAX_OUTPUT_LINES; i++) {
         sge_mutex_lock("sge_monitor_status", __func__, __LINE__, &(Output[i].Output_Mutex));
         if (Output[i].name != nullptr) {
            append_time(Output[i].update_time, &Info_Line, false);
            sge_dstring_append(&Info_Line, " | ");
            sge_dstring_append_dstring(&Info_Line, Output[i].output);
            sge_dstring_append(&Info_Line, "\n");
         }
         sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &(Output[i].Output_Mutex));
      }
   } else {
      sge_dstring_append(&Info_Line, MSG_UTI_MONITOR_DISABLED);
      sge_dstring_append(&Info_Line, "\n");
   }

   *info_message = strdup(sge_dstring_get_string(&Info_Line));

   sge_mutex_unlock("sge_monitor_status", __func__, __LINE__, &global_mutex);
   DRETURN(ret);
}


/****** uti/monitor/sge_set_last_wait_time() ***********************************
*  NAME
*     sge_set_last_wait_time() -- updates the last wait time (health monitoring)
*
*  SYNOPSIS
*     void sge_set_last_wait_time(monitoring_t *monitor, struct timeval after) 
*
*  FUNCTION
*     Updates the last wait time, which is used for the health monitoring to 
*     determine if a thread has a problem or not.
*
*  INPUTS
*     monitoring_t *monitor    - monitoring structure
*     struct timeval wait_time - current time 
*
*  NOTES
*     MT-NOTE: sge_set_last_wait_time() is MT safe 
*
*******************************************************************************/
void sge_set_last_wait_time(monitoring_t *monitor, struct timeval wait_time) {
   DENTER(GDI_LAYER);

   if (monitor->pos != -1) {
      sge_mutex_lock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));

      Output[monitor->pos].last_wait_time = wait_time;

      sge_mutex_unlock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));
   }
   DRETURN_VOID;
}


/****** uti/monitor/sge_monitor_output() ***************************************
*  NAME
*     sge_monitor_output() -- outputs the result into the message file
*
*  SYNOPSIS
*     void sge_monitor_output(monitoring_t *monitor) 
*
*  FUNCTION
*     This function computes the output line from the gathered statistics.
*     The output is only generated, when the the output flag in the
*     monitoring structure is set.
*
*     The monitoring line is printed to the message file in the profiling
*     class and it made available for the qping -f output. For the qping
*     output, it stores the the generation time, though that qping can
*     show, when the message was generated.
*
*     If an extension is set, it calls the appropriate output function for
*     it.
*
*  INPUTS
*     monitoring_t *monitor - the monitoring info
*
*  NOTES
*     MT-NOTE: sge_monitor_output() is MT safe 
*
*******************************************************************************/
void sge_monitor_output(monitoring_t *monitor) {
   DENTER(GDI_LAYER);

   if ((monitor != nullptr) && monitor->output) {
      struct timeval after{};
      double time;

      gettimeofday(&after, nullptr);
      time = after.tv_usec - monitor->now.tv_usec;
      time = after.tv_sec - monitor->now.tv_sec + (time / 1000000);

      sge_dstring_clear(monitor->work_line);

      sge_dstring_sprintf_append(monitor->work_line, MSG_UTI_MONITOR_DEFLINE_SF,
                                 monitor->thread_name, monitor->message_in_count / time);

      if (monitor->ext_type != NONE_EXT) {
         sge_dstring_append(monitor->work_line, " (");
         monitor->ext_output(monitor->work_line, monitor->ext_data, time);
         sge_dstring_append(monitor->work_line, ")");
      }

      sge_dstring_sprintf_append(monitor->work_line, MSG_UTI_MONITOR_DEFLINE_FFFFF,
                                 monitor->message_out_count / time,
                                 monitor->message_in_count ? (time - monitor->idle) / monitor->message_in_count : 0,
                                 monitor->idle / time * 100, monitor->wait / time * 100, time);

      /* only log into the message file, if the user wants it */
      if (monitor->log_monitor_mes) {
         sge_log(LOG_PROF, sge_dstring_get_string(monitor->work_line), __FILE__, __LINE__);
      }

      if (monitor->pos != -1) {
         sge_mutex_lock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));
         dstring *tmp = Output[monitor->pos].output;
         Output[monitor->pos].output = monitor->work_line;
         Output[monitor->pos].update_time = after.tv_sec;
         sge_mutex_unlock("sge_monitor_init", __func__, __LINE__, &(Output[monitor->pos].Output_Mutex));

         monitor->work_line = tmp;
      }

      sge_monitor_reset(monitor);
   }

   DRETURN_VOID;
}


/****** uti/monitor/sge_monitor_reset() ****************************************
*  NAME
*     sge_monitor_reset() --  resets the monitoring data
*
*  SYNOPSIS
*     void sge_monitor_reset(monitoring_t *monitor) 
*
*  FUNCTION
*     Resets the data structure including the extension. No data in the
*     extension is preserved.
*
*  INPUTS
*     monitoring_t *monitor - monitoring structure
*
*  NOTES
*     MT-NOTE: sge_monitor_reset() is MT safe 
*
*******************************************************************************/
void sge_monitor_reset(monitoring_t *monitor) {
   DENTER(GDI_LAYER);

   monitor->monitor_time = 0;
   monitor->now.tv_sec = 0;
   monitor->now.tv_usec = 0;
   monitor->output = false;
   monitor->message_in_count = 0;
   monitor->message_out_count = 0;
   monitor->idle = 0.0;
   monitor->wait = 0.0;

   if (monitor->ext_type != NONE_EXT) {
      memset(monitor->ext_data, 0, monitor->ext_data_size);
   }

   DRETURN_VOID;
}

/****************************************
 * implementation section for extensions
 ****************************************/

/****** uti/monitor/ext_sch_output() *******************************************
*  NAME
*     ext_sch_output() -- generates a string from the scheduler extension
*
*  SYNOPSIS
*     static void 
*     ext_sch_output(char *message, int size, void 
*                    *monitoring_extension, double time) 
*
*  FUNCTION
*     generates a string from the extension and returns it.
*
*  INPUTS
*     char *message              - initialized string buffer
*     int size                   - buffer size
*     void *monitoring_extension - the extension structure
*     double time                - length of the measurement interval
*
*  NOTES
*     MT-NOTE: ext_gdi_output() is MT safe 
*******************************************************************************/
static void ext_sch_output(dstring *message, void *monitoring_extension, double time) {
   sge_dstring_sprintf_append(message, "");
}

/****** uti/monitor/ext_gdi_output() *******************************************
*  NAME
*     ext_gdi_output() -- generates a string from the GDI extension
*
*  SYNOPSIS
*     static void ext_gdi_output(char *message, int size, void 
*     *monitoring_extension, double time) 
*
*  FUNCTION
*     generates a string from the extension and returns it.
*
*  INPUTS
*     char *message              - initialized string buffer
*     int size                   - buffer size
*     void *monitoring_extension - the extension structure
*     double time                - length of the measurement interval
*
*  NOTES
*     MT-NOTE: ext_gdi_output() is MT safe 
*******************************************************************************/
static void ext_gdi_output(dstring *message, void *monitoring_extension, double time) {
   auto *gdi_ext = (m_gdi_t *) monitoring_extension;

   sge_dstring_sprintf_append(message, MSG_UTI_MONITOR_GDIEXT_FFFFFFFFFFFFIII,
                              gdi_ext->eload_count / time, gdi_ext->ejob_count / time,
                              gdi_ext->econf_count / time, gdi_ext->eproc_count / time,
                              gdi_ext->eack_count / time,
                              gdi_ext->gdi_add_count / time, gdi_ext->gdi_get_count / time,
                              gdi_ext->gdi_mod_count / time, gdi_ext->gdi_del_count / time,
                              gdi_ext->gdi_cp_count / time, gdi_ext->gdi_trig_count / time,
                              gdi_ext->gdi_perm_count / time,
                              sge_u32c(gdi_ext->queue_length),
                              sge_u32c(gdi_ext->rqueue_length),
                              sge_u32c(gdi_ext->wrqueue_length)
                              );
}

/****** uti/monitor/ext_lis_output() *******************************************
*  NAME
*     ext_lis_output() -- generates a string from the listener extension 
*
*  SYNOPSIS
*     static void 
*     ext_lis_output(char *message, int size, void 
*                    *monitoring_extension, double time) 
*
*  FUNCTION
*     generates a string from the extension and returns it.
*
*  INPUTS
*     char *message              - initialized string buffer
*     int size                   - buffer size
*     void *monitoring_extension - the extension structure
*     double time                - length of the measurement interval
*
*  NOTES
*     MT-NOTE: ext_lis_output() is MT safe 
*******************************************************************************/
static void ext_lis_output(dstring *message, void *monitoring_extension, double time) {
   auto *lis_ext = (m_lis_t *) monitoring_extension;

   sge_dstring_sprintf_append(message, MSG_UTI_MONITOR_LISEXT_FFFFFFF,
                              lis_ext->inc_gdi / time,
                              lis_ext->inc_ack / time,
                              lis_ext->inc_ece / time,
                              lis_ext->inc_rep / time,
                              lis_ext->gdi_get_count / time,
                              lis_ext->gdi_trig_count / time,
                              lis_ext->gdi_perm_count / time
                              );
}


/****** uti/monitor/ext_edt_output() *******************************************
*  NAME
*     ext_edt_output() -- generates a string from the GDI extension
*
*  SYNOPSIS
*     static void ext_edt_output(char *message, int size, void 
*     *monitoring_extension, double time) 
*
*  FUNCTION
*     generates a string from the extension and returns it.
*
*  INPUTS
*     char *message              - initialized string buffer
*     int size                   - buffer size
*     void *monitoring_extension - the extension structure
*     double time                - length of the measurement interval
*
*  NOTES
*     MT-NOTE: ext_edt_output() is MT safe 
*
*******************************************************************************/
static void ext_edt_output(dstring *message, void *monitoring_extension, double time) {
   auto *edt_ext = (m_edt_t *) monitoring_extension;

   sge_dstring_sprintf_append(message, MSG_UTI_MONITOR_EDTEXT_FFFFFFFF,
                              ((double) edt_ext->client_count) / edt_ext->count,
                              edt_ext->mod_client_count / time,
                              edt_ext->ack_count / time,
                              ((double) edt_ext->blocked_client_count) / edt_ext->count,
                              ((double) edt_ext->busy_client_count) / edt_ext->count,

                              edt_ext->new_event_count / time,
                              edt_ext->added_event_count / time,
                              edt_ext->skip_event_count / time);
}

/****** uti/monitor/ext_tet_output() *******************************************
*  NAME
*     ext_tet_output() -- generates a string from the GDI extension
*
*  SYNOPSIS
*     static void ext_edt_output(char *message, int size, void 
*     *monitoring_extension, double time) 
*
*  FUNCTION
*     generates a string from the extension and returns it.
*
*  INPUTS
*     dstring *message           - initialized string buffer
*     void *monitoring_extension - the extension structure
*     double time                - length of the measurement interval
*
*  NOTES
*     MT-NOTE: ext_tet_output() is MT safe 
*
*******************************************************************************/

static void ext_tet_output(dstring *message, void *monitoring_extension, double time) {
   auto *tet_ext = (m_tet_t *) monitoring_extension;

   sge_dstring_sprintf_append(message, MSG_UTI_MONITOR_TETEXT_FF,
                              ((double) tet_ext->event_count) / tet_ext->count,
                              tet_ext->exec_count / time
   );
}

