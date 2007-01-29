/*
 *  Copyright 2005-2006 The Apache Software Foundation or its licensors, as applicable.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @author Xiao-Feng Li, 2006/10/05
 */

#ifndef _COLLECTOR_ALLOC_H_
#define _COLLECTOR_ALLOC_H_

#include "gc_thread.h"

void* mos_alloc(unsigned size, Allocator *allocator);

/* NOS forward obj to MOS in MINOR_COLLECTION */
FORCE_INLINE Partial_Reveal_Object* collector_forward_object(Collector* collector, Partial_Reveal_Object* p_obj)
{
  Obj_Info_Type oi = get_obj_info_raw(p_obj);

  /* forwarded by somebody else */
  if ((POINTER_SIZE_INT)oi & FORWARD_BIT){
     return NULL;
  }
  
  /* otherwise, try to alloc it. mos should always has enough space to hold nos during collection */
  unsigned int size = vm_object_size(p_obj);

  Partial_Reveal_Object* p_targ_obj = thread_local_alloc(size, (Allocator*)collector);
  if(!p_targ_obj)
    p_targ_obj = (Partial_Reveal_Object*)mos_alloc(size, (Allocator*)collector);
    
  if(p_targ_obj == NULL){
    /* failed to forward an obj */
    collector->result = FALSE;
    return NULL;
  }
    
  /* else, take the obj by setting the forwarding flag atomically 
     we don't put a simple bit in vt because we need compute obj size later. */
  if ((void*)oi != atomic_casptr((volatile void**)get_obj_info_addr(p_obj), (void*)((POINTER_SIZE_INT)p_targ_obj|FORWARD_BIT), (void*)oi)) {
    /* forwarded by other, we need unalloc the allocated obj. We may waste some space if the allocation switched
       block. The remaining part of the switched block cannot be revivied for next allocation of 
       object that has smaller size than this one. */
    assert( obj_is_fw_in_oi(p_obj));
    thread_local_unalloc(size, (Allocator*)collector);
    return NULL;
  }

  /* we forwarded the object */
  memcpy(p_targ_obj, p_obj, size);

  /* we need clear the bit to give major collection a clean status. */
  if(gc_is_gen_mode())
    set_obj_info(p_targ_obj, oi&DUAL_MARKBITS_MASK);

#ifdef MARK_BIT_FLIPPING 
  /* we need set MARK_BIT to indicate this object is processed for nongen forwarding */
  else
    set_obj_info(p_targ_obj, oi|FLIP_MARK_BIT);

#endif

  return p_targ_obj;  
 
}

#endif /* _COLLECTOR_ALLOC_H_ */