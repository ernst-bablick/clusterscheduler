/*___INFO__MARK_BEGIN_NEW__*/
/***************************************************************************
 *  
 *  Copyright 2024 HPC-Gridware GmbH
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  
 ***************************************************************************/
/*___INFO__MARK_END_NEW__*/

#include <pthread.h>

#include "ocs_event_master.h"

u_long64
oge_get_next_unique_event_id() {
   static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
   static u_long64 id = 0LL;

   pthread_mutex_lock(&mutex);
   u_long64 ret = id++;
   pthread_mutex_unlock(&mutex);
   return ret;
}

