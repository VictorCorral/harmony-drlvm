/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
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
 * @file thread_helpers.cpp
 * @brief Set of VM helpers
 *
 * This file contains the set of "VM helpers" which help to optimize monitors performance
 * in the code generated by JIT compiler. Typically, these functions will be called by JIT,
 * but VM also could also use them with care.
 */

#include <open/hythread_ext.h>
#include <open/thread_helpers.h>
#include "open/jthread.h"
#include "object_handles.h"
#include "port_malloc.h"
#include "m2n.h"

#include <assert.h>


/**
  *  Generates tmn_self() call.
  *  The code should not contains safepoint.
  *  The code uses and doesn't restore eax register.
  *
  *  @return tm_self() in eax register
  */
char* gen_hythread_self_helper(char *ss) {
#ifdef HYTHREAD_FAST_TLS
#   ifdef FS14_TLS_USE
        //ss = mov(ss,  eax_opnd,  M_Base_Opnd(fs_reg, 0x14));
        *ss++ = (char)0x64;
        *ss++ = (char)0xa1;
        *ss++ = (char)0x14;
        *ss++ = (char)0x00;
        *ss++ = (char)0x00;
        *ss++ = (char)0x00;
#   else
        unsigned offset = hythread_get_hythread_offset_in_tls();
        // gs register uses for tls acces on linux x86-32
        //ss = mov(ss,  edx_opnd,  M_Base_Opnd(gs_reg, 0x00));
        *ss++ = (char)0x65;
        *ss++ = (char)0x8b;
        *ss++ = (char)0x15;
        *ss++ = (char)0x00;
        *ss++ = (char)0x00;
        *ss++ = (char)0x00;
        *ss++ = (char)0x00;
        ss = mov(ss,  eax_opnd,  M_Base_Opnd(edx_reg, offset));
#   endif
#else
    ss = call(ss, (char *)hythread_self);
#endif
    return ss;
}


/**
  *  Generates fast path of monitor enter
  *  the code should not contains safepoint.
  *
  *  @param[in] ss buffer to put the assembly code to
  *  @param[in] input_param1 register which should point to the object lockword.
  *  If input_param1 == ecx it reduces one register mov.
  *  the code use and do not restore ecx, edx, eax registers
  *
  *  @return 0 if success in eax register
  */
char* gen_monitorenter_fast_path_helper(char *ss, const R_Opnd & input_param1) {

    if (&input_param1 != &ecx_opnd) {
        ss = mov(ss, ecx_opnd,  input_param1);
    }
#ifdef ASM_MONITOR_HELPER

    //get self_id
    ss = gen_hythread_self_helper(ss);
    ss = mov(ss,  edx_opnd,  M_Base_Opnd(eax_reg,
            hythread_get_thread_id_offset()));                          // mov edx,dword [eax+off]

    ss = mov(ss, eax_opnd, M_Base_Opnd(ecx_reg, 2), size_16);           // mov ax,word[ecx+2]
	ss = alu(ss, cmp_opc,  edx_opnd,  eax_opnd, size_16);           	// cmp dx,ax
    ss = branch8(ss, Condition_NZ,  Imm_Opnd(size_8, 0));               // jnz check_zero
    char *check_zero = ((char *)ss) - 1;
												                        //; ax==dx it's safe to do inc
	ss = mov(ss, eax_opnd, M_Base_Opnd(ecx_reg, 1), size_8);            // mov al, byte[ecx+1]
	                                                                    //rec_inc:

    ss = alu(ss, add_opc,  eax_opnd,  Imm_Opnd(size_8, 0x8), size_8);   // add al,0x8
    ss = branch8(ss, Condition_C,  Imm_Opnd(size_8, 0));                // jc failed
    char *failed1 = ((char *)ss) - 1;

	ss = mov(ss,  M_Base_Opnd(ecx_reg, 1), eax_opnd, size_8);           // mov byte[ecx+1],al
    ss = ret(ss,  Imm_Opnd(4));                                         // ret 4

    signed offset = (signed)ss - (signed)check_zero - 1;
    *check_zero = (char)offset;                                        //check_zero:

    ss = test(ss,  eax_opnd,  eax_opnd, size_16);                      //  test ax,ax
    ss = branch8(ss, Condition_NZ,  Imm_Opnd(size_8, 0));              //  jnz failed
    char *failed2 = ((char *)ss) - 1;

    ss = prefix(ss, lock_prefix);                                      //; here ax==0.
    ss = cmpxchg(ss,  M_Base_Opnd(ecx_reg, 2),  edx_opnd, size_16);    //  lock cmpxchg16 [ecx+2],dx
    ss = branch8(ss, Condition_NZ,  Imm_Opnd(size_8, 0));              //  jnz failed
    char *failed3 = ((char *)ss) - 1;


#ifdef LOCK_RESERVATION
	ss = mov(ss, eax_opnd, M_Base_Opnd(ecx_reg, 1), size_8);            //  mov al, byte[ecx+1]
    ss = test(ss,  eax_opnd,  Imm_Opnd(size_8, 0x4), size_8);           //  test al,0x4
    ss = branch8(ss, Condition_NZ,  Imm_Opnd(size_8, 0));               //  jnz finish
    char *finish = ((char *)ss) - 1;

    ss = alu(ss, add_opc,  eax_opnd,  Imm_Opnd(size_8, 0x8), size_8);   // add al,0x8
	ss = mov(ss,  M_Base_Opnd(ecx_reg, 1), eax_opnd, size_8);           // mov byte[ecx+1],al

    offset = (signed)ss - (signed)finish - 1;
    *finish = (char)offset;                            		            //finish:

#endif
    ss = ret(ss,  Imm_Opnd(4));                                         // ret 4

    offset = (signed)ss - (signed)failed1 - 1;
    *failed1 = (char)offset;                                 	        //failed:

    offset = (signed)ss - (signed)failed2 - 1;
    *failed2 = (char)offset;

    offset = (signed)ss - (signed)failed3 - 1;
    *failed3 = (char)offset;

#endif //ASM_MONITOR_HELPER
    // the second attempt to lock monitor
    ss = push(ss,  ecx_opnd);
    ss = call(ss, (char *)hythread_thin_monitor_try_enter);
    ss = alu(ss, add_opc, esp_opnd, Imm_Opnd(4)); // pop parameters

    return ss;
}

static IDATA rt_jthread_monitor_enter(ManagedObject*  monitor) {
    const unsigned handles_size = (unsigned)(sizeof(ObjectHandlesNew)+sizeof(ManagedObject*)*4);
    ObjectHandlesNew* handels = (ObjectHandlesNew *)STD_ALLOCA(handles_size);
    handels->capacity = 4;
    handels->size = 0;
    handels->next = NULL;

    m2n_set_local_handles(m2n_get_last_frame(), (ObjectHandles *) handels);

    ObjectHandle monitorJavaObj = oh_allocate_local_handle();
    monitorJavaObj->object = monitor;

    IDATA result = jthread_monitor_enter(monitorJavaObj);

    free_local_object_handles2(m2n_get_local_handles(m2n_get_last_frame()));
    m2n_set_local_handles(m2n_get_last_frame(), NULL);

    return result;
}

/**
  *  Generates slow path of monitor enter.
  *  This code could block on monitor and contains safepoint.
  *  The appropriate m2n frame should be generated and
  *
  *  @param[in] ss buffer to put the assembly code to
  *  @param[in] input_param1 register should point to the jobject(handle)
  *  If input_param1 == eax it reduces one register mov.
  *  the code use and do not restore ecx, edx, eax registers
  *  @return 0 if success in eax register
  */
char* gen_monitorenter_slow_path_helper(char *ss, const R_Opnd & input_param1) {
    if (&input_param1 != &eax_opnd) {
        ss = mov(ss, eax_opnd,  input_param1);
    }

    ss = push(ss, eax_opnd); // push the address of the handle
    ss = call(ss, (char *)rt_jthread_monitor_enter);
    ss = alu(ss, add_opc, esp_opnd, Imm_Opnd(4)); // pop parameters
    return ss;
}

/**
  *  Generates monitor exit.
  *  The code should not contain safepoints.
  *
  *  @param[in] ss buffer to put the assembly code to
  *  @param[in] input_param1 register should point to the lockword in object header.
  *  If input_param1 == ecx it reduce one register mov.
  *  The code use and do not restore eax registers.
  *  @return 0 if success in eax register
  */
char* gen_monitor_exit_helper(char *ss, const R_Opnd & input_param1) {
    if (&input_param1 != &ecx_opnd) {
        ss = mov(ss, ecx_opnd,  input_param1);
    }
#ifdef ASM_MONITOR_HELPER
	ss = mov(ss,  eax_opnd, M_Base_Opnd(ecx_reg, 0));               //  mov eax,dword[ecx]
	ss = test(ss, eax_opnd, Imm_Opnd(0x80000000), size_32);         //  test eax,0x80000000
	ss = branch8(ss, Condition_NZ,  Imm_Opnd(size_8, 0));           //  jnz fat
	char *fat = ((char *)ss) - 1;
	ss = mov(ss, eax_opnd, M_Base_Opnd(ecx_reg, 1), size_8);        //  mov al, byte[ecx+1]

    ss = alu(ss, sub_opc,  eax_opnd, Imm_Opnd(size_8,0x8),size_8);  //  sub al, 0x8
	ss = branch8(ss, Condition_C,  Imm_Opnd(size_8, 0));            //  jc zero_rec
	char *zero_rec = ((char *)ss) - 1;
	ss = mov(ss,  M_Base_Opnd(ecx_reg, 1), eax_opnd, size_8);       //  mov byte[ecx+1],al
    ss = ret(ss,  Imm_Opnd(4));                                     //  ret 4

    signed offset = (signed)ss - (signed)zero_rec - 1;              //zero_rec:
    *zero_rec = (char)offset;

    ss = mov(ss, M_Base_Opnd(ecx_reg, 2), Imm_Opnd(size_16, 0), size_16);// mov word[ecx+2],0
	ss = ret(ss,  Imm_Opnd(4));                                     // ret 4

    offset = (signed)ss - (signed)fat - 1;                         //fat:
    *fat = (char)offset;

#endif

    ss = push(ss,  ecx_opnd);
    ss = call(ss, (char *)hythread_thin_monitor_exit);
    ss = alu(ss, add_opc, esp_opnd, Imm_Opnd(4)); // pop parameters
    return ss;
}

/**
  *  Generates slow path of monitor exit.
  *  This code could block on monitor and contains safepoint.
  *  The appropriate m2n frame should be generated and
  *
  *  @param[in] ss buffer to put the assembly code to
  *  @param[in] input_param1 register should point to the jobject(handle)
  *  If input_param1 == eax it reduces one register mov.
  *  the code use and do not restore ecx, edx, eax registers
  *  @return 0 if success in eax register
  */
char* gen_monitorexit_slow_path_helper(char *ss, const R_Opnd & input_param1) {
    if (&input_param1 != &eax_opnd) {
        ss = mov(ss, eax_opnd,  input_param1);
    }

    ss = push(ss, eax_opnd); // push the address of the handle
    ss = call(ss, (char *)jthread_monitor_exit);
    ss = alu(ss, add_opc, esp_opnd, Imm_Opnd(4)); // pop parameters
    return ss;
}

/**
  * Generates fast accessor to the TLS for the given key.<br>
  * Example:
  * <pre><code>
  * get_thread_ptr = get_tls_helper(vm_thread_block_key);
  * ...
  * self = get_thread_ptr();
  * </code></pre>
  *
  * @param[in] key TLS key
  * @return fast accessor to key, if one exist
  */
fast_tls_func* get_tls_helper(hythread_tls_key_t key) {
    //     return tm_self_tls->thread_local_storage[key];
    unsigned key_offset =
        (unsigned) &(((HyThread_public *) (0))->thread_local_storage[key]);

    const int stub_size = 126;
    char *stub = (char *)malloc(stub_size);
    memset(stub, 0xcc /*int 3*/, stub_size);

    char *ss = stub;

    ss = gen_hythread_self_helper(ss);
    ss = mov(ss,  eax_opnd,  M_Base_Opnd(eax_reg, key_offset));
    ss = ret(ss,  Imm_Opnd(0));

    assert((ss - stub) < stub_size);

    return (fast_tls_func*) stub;
}
