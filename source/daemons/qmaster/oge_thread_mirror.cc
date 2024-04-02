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

#include "uti/sge_rmon_macros.h"

#include "oge_thread_mirror.h"
#include "oge_MirrorReaderDataStore.h"
#include "oge_MirrorListenerDataStore.h"
#include "sge_thread_main.h"

namespace oge {
   static void *
   event_mirror_main(void *arg) {
      auto *mirror_thread = static_cast<oge::MirrorDataStore *>(arg);

      return mirror_thread->main(arg);
   }

   void
   event_mirror_initialize() {
      DENTER(TOP_LAYER);
      oge::MirrorDataStore *mirror_thread;

#if 0
      // create reader mirror
      mirror_thread = new oge::MirrorReaderDataStore();
      Main_Control.mirror_thread_pool.push_back(mirror_thread);
      pthread_create(&mirror_thread->thread, nullptr, event_mirror_main, mirror_thread);
      DPRINTF("added event mirror thread for data store %d\n", mirror_thread->data_store_id);
#endif

      // create listener mirror
      mirror_thread = new oge::MirrorListenerDataStore();
      Main_Control.mirror_thread_pool.push_back(mirror_thread);
      pthread_create(&mirror_thread->thread, nullptr, event_mirror_main, mirror_thread);
      DPRINTF("added event mirror thread for data store %d\n", mirror_thread->data_store_id);

      DRETURN_VOID;
   }

   void
   event_mirror_terminate() {
      DENTER(TOP_LAYER);

      // trigger cancel and wakeup so that all threads can reach the cancellation point
      for (auto mirror_thread: Main_Control.mirror_thread_pool) {
         DPRINTF("triggered shutdown/wake up of mirror thread for data store %d\n", mirror_thread->data_store_id);

         pthread_cancel(mirror_thread->thread);
         mirror_thread->wakeup();
      }

      // wait till each thread returns from its main
      for (auto mirror_thread: Main_Control.mirror_thread_pool) {
         pthread_join(mirror_thread->thread, nullptr);
         delete mirror_thread;
         DPRINTF("termination of thread for data store %d finished\n", mirror_thread->data_store_id);
      }

      // empty the container
      Main_Control.mirror_thread_pool.clear();

      DRETURN_VOID;
   }
}
