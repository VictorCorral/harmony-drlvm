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
 * @author Nikolay Kuznetsov
 * @version $Revision: 1.1.2.7 $
 */  

/**
 * @file thread_native_tls.c
 * @brief Hythread TLS related functions
 */

#include <open/hythread_ext.h>
#include "thread_private.h"


/** @name Thread local storage support
 */
//@{

int16 tm_tls_capacity = 16;
int16 tm_tls_size = 0;

static void tls_finalizer_placeholder(void *args) {}


/**
 * Allocate a thread local storage (TLS) key.
 * 
 * Create and return a new, unique key for thread local storage.  
 * 
 * @note The hande returned will be >=0, so it is safe to test the handle against 0 to see if it's been
 * allocated yet.
 * 
 * @param[out] handle pointer to a key to be initialized with a key value
 * @return 0 on success or negative value if a key could not be allocated (i.e. all TLS has been allocated)
 * 
 * @see hythread_tls_free, hythread_tls_set
 */
IDATA VMCALL hythread_tls_alloc(hythread_tls_key_t *handle) {
    return hythread_tls_alloc_with_finalizer(handle, tls_finalizer_placeholder);
}

/**
 * Allocate a thread local storage (TLS) key.
 * 
 * Create and return a new, unique key for thread local storage.  
 * 
 * @note The hande returned will be >=0, so it is safe to test the handle against 0 to see if it's been
 * allocated yet.
 * 
 * @param[out] handle pointer to a key to be initialized with a key value
 * @param[in] finalizer a finalizer function which will be invoked when a thread is
 * detached or terminates if the thread's TLS entry for this key is non-NULL
 * @return 0 on success or negative value if a key could not be allocated (i.e. all TLS has been allocated)
 * 
 * @see hythread_tls_free, hythread_tls_set
 */
IDATA VMCALL hythread_tls_alloc_with_finalizer(hythread_tls_key_t *handle, hythread_tls_finalizer_t finalizer) {
    if (tm_tls_size < tm_tls_capacity - 1) {
        *handle = ++tm_tls_size;
        return TM_ERROR_NONE;
    }

    return -1;
}

/**
 * Returns a thread's TLS value.
 */
void* VMCALL hythread_tls_get(hythread_t thread, hythread_tls_key_t key) {
    return thread->thread_local_storage[key];
}

/**
 * Set a thread's TLS value.
 *
 * @param[in] thread a thread 
 * @param[in] key key to have TLS value set (any value returned by hythread_alloc)
 * @param[in] data value to be stored in TLS
 * @return 0 on success or negative value on failure
 *  
 * @see hythread_tls_alloc, hythread_tls_free, hythread_tls_get
 */
IDATA VMCALL hythread_tls_set(hythread_t thread, hythread_tls_key_t key, void *data) {
    assert(thread->thread_local_storage);
    thread->thread_local_storage[key] = data;
    return TM_ERROR_NONE;
}

/**
 * Release a TLS key.
 * 
 * Release a TLS key previously allocated by hythread_tls_alloc.
 * 
 * @param[in] key TLS key to be freed
 * @return 0 on success or negative value on failure
 *
 * @see hythread_tls_alloc, hythread_tls_set
 */
IDATA VMCALL hythread_tls_free(hythread_tls_key_t key) {
    //@TODO: implement entries deletion in TLS
    return TM_ERROR_NONE;
}

//@}