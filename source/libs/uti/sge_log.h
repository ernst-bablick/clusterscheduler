#ifndef __SGE_LOG_H
#define __SGE_LOG_H
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

#include <stdio.h>

#ifndef WIN32NATIVE
#	include <syslog.h>
#else 
#	include "win32nativetypes.h"
#endif 

#ifndef __BASIS_TYPES_H
#   include "basis_types.h"
#endif

#include "msg_utilib.h"

typedef void (*trace_func_type)(const char *);

#if defined(SGE_MT)
void log_init_mt(void);
#endif


char *log_state_get_log_buffer(void);
u_long32 log_state_get_log_level(void);
const char*log_state_get_log_file(void);
int log_state_get_log_verbose(void);
int log_state_get_log_gui(void);
trace_func_type log_state_get_log_trace_func(void);
int log_state_get_log_as_admin_user(void);

void log_state_set_log_level(u_long32);
void log_state_set_log_file(char *file);
void log_state_set_log_verbose(int i);
void log_state_set_log_gui(int i);
void log_state_set_log_trace_func(trace_func_type );
void log_state_set_log_as_admin_user(int i);

/* extern stringTlong SGE_EVENT; */
#define SGE_EVENT log_state_get_log_buffer()

#ifdef NO_SGE_COMPILE_DEBUG
#   define SGE_FUNC ""
#endif

#define SGE_LOG(level,msg) sge_log(level, msg, __FILE__, SGE_FUNC, __LINE__ );

#ifdef WIN32NATIVE
#undef ERROR
/* One windows header file defines ERROR as 0. This haeder is included by
   winsock2.h. Until we do not need this constant it is save to make an undef 
   here. */
#endif 

#if defined(__INSURE__)
#   define CRITICAL(x) (sprintf x,sge_log(LOG_CRIT,   SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#   define ERROR(x)    (sprintf x,sge_log(LOG_ERR,    SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#   define WARNING(x)  (sprintf x,sge_log(LOG_WARNING,SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#   define NOTICE(x)   (sprintf x,sge_log(LOG_NOTICE, SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#   define INFO(x)     (sprintf x,sge_log(LOG_INFO,   SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#   define DEBUG(x)    (sprintf x,sge_log(LOG_DEBUG,  SGE_EVENT,__FILE__,SGE_FUNC,__LINE__)) ? 1 : 0
#else

/****** uti/log/CRITICAL() ****************************************************
*  NAME
*     CRITICAL() -- Log a critical message 
*
*  SYNOPSIS
*     #define CRITICAL(params)
*     void CRITICAL(char *buffer, const char* formatstring, ...) 
*
*  FUNCTION
*     Log a critical message 
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#ifdef __SGE_COMPILE_WITH_GETTEXT__
#   define CRITICAL(x) (sge_set_message_id_output(1), \
                        sprintf x, \
                        sge_set_message_id_output(0), \
                        sge_log(LOG_CRIT, SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#else
#   define CRITICAL(x) (sprintf x, \
                        sge_log(LOG_CRIT, SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#endif

/****** uti/log/ERROR() *******************************************************
*  NAME
*     ERROR() -- Log an error message 
*
*  SYNOPSIS
*     #define ERROR(params)
*     void ERROR(char *buffer, const char* formatstring, ...) 
*
*  FUNCTION
*     Log a error message 
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#ifdef __SGE_COMPILE_WITH_GETTEXT__
#   define ERROR(x)    (sge_set_message_id_output(1),                          \
                        sprintf x,                                             \
                        sge_set_message_id_output(0),                          \
                        sge_log(LOG_ERR,SGE_EVENT,__FILE__,SGE_FUNC,__LINE__), 1) ? 1 : 0
#else
#   define ERROR(x)    (sprintf x,                                             \
                        sge_log(LOG_ERR,SGE_EVENT,__FILE__,SGE_FUNC,__LINE__), 1) ? 1 : 0
#endif

/****** uti/log/WARNING() ******************************************************
*  NAME
*     WARNING() -- Log an warning message
*
*  SYNOPSIS
*     #define WARNING(params)
*     void WARNING(char *buffer, const char* formatstring, ...)
*
*  FUNCTION
*     Log a warning message
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#ifdef __SGE_COMPILE_WITH_GETTEXT__
#   define WARNING(x)  (sge_set_message_id_output(1), \
                        sprintf x,       \
                        sge_set_message_id_output(0), \
                        sge_log(LOG_WARNING,SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#else
#   define WARNING(x)  ( sprintf x,       \
                        sge_log(LOG_WARNING,SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#endif

/****** uti/log/NOTICE() ******************************************************
*  NAME
*     NOTICE() -- Log a notice message
*
*  SYNOPSIS
*     #define NOTICE(params)
*     void NOTICE(char *buffer, const char* formatstring, ...)
*
*  FUNCTION
*     Log a notice message
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#   define NOTICE(x)   (sprintf x,  \
                        sge_log(LOG_NOTICE, SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0

/****** uti/log/INFO() ********************************************************
*  NAME
*     INFO() -- Log an info message
*
*  SYNOPSIS
*     #define INFO(params)
*     void INFO(char *buffer, const char* formatstring, ...)
*
*  FUNCTION
*     Log an info message
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#   define INFO(x)     (sprintf x,  \
                        sge_log(LOG_INFO,   SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0

/****** uti/log/DEBUG() ******************************************************
*  NAME
*     DEBUG() -- Log a debug message
*
*  SYNOPSIS
*     #define DEBUG(params)
*     void DEBUG(char *buffer, const char* formatstring, ...)
*
*  FUNCTION
*     Log a debug message
*
*  INPUTS
*     buffer       - e.g SGE_EVENT
*     formatstring - printf formatstring
*     ...
******************************************************************************/ 
#ifdef __SGE_COMPILE_WITH_GETTEXT__
#   define DEBUG(x)    (sge_set_message_id_output(1),  \
                        sprintf x,  \
                        sge_set_message_id_output(0), \
                        sge_log(LOG_DEBUG,  SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#else
#   define DEBUG(x)    (sprintf x,  \
                        sge_log(LOG_DEBUG,  SGE_EVENT,__FILE__,SGE_FUNC,__LINE__) ,1) ? 1 : 0
#endif
#endif

/****** uti/log/SGE_ASSERT() **************************************************
*  NAME
*     SGE_ASSERT() -- Log a message and exit if assertion is false 
*
*  SYNOPSIS
*     #define SGE_ASSERT(expression)
*     void SGE_ASSERT(int expression) 
*
*  FUNCTION
*     Log a critical message and exit if assertion is false.
*
*  INPUTS
*     expression   - a logical expression 
******************************************************************************/
#if defined(_AIX) || defined(SVR3)
#  define SGE_ASSERT(x) \
   if (x) { \
      ; \
   } else { \
      sge_log(LOG_CRIT, # x ,__FILE__,SGE_FUNC,__LINE__); \
      sge_log(LOG_CRIT, MSG_UNREC_ERROR,__FILE__,SGE_FUNC,__LINE__); \
      abort(); \
   }
#else
#  define SGE_ASSERT(x) \
   if (x) { \
      ; \
   } else { \
      sge_log(LOG_CRIT, MSG_UNREC_ERROR,__FILE__,SGE_FUNC,__LINE__); \
      abort(); \
   }
#endif

int sge_log(int log_level, const char *mesg, const char *file__, 
            const char *func__, int line__);

#endif /* __SGE_LOG_H */

