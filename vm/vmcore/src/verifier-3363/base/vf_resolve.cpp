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
 * @author Mikhail Loenko, Vladimir Molotkov
 */  

#include "verifier.h"
#include "context_base.h"
#include "time.h"

namespace CPVerifier {

    /**
    * Function checkes constraint for given class.
    * Function loads classes if it's needed.
    */
    vf_Result
        vf_force_check_constraint( class_handler klass, 
        vf_TypeConstraint *constraint )    // class constraint
    {
        // get target class
        class_handler target = vf_resolve_class( klass, constraint->target, true );
        if( !target ) {
            return VF_ErrorLoadClass;
        }

        //no need to load the source
        if( class_is_interface_(target) ){
            return VF_OK;
        }


        // get stack reference class
        class_handler source = vf_resolve_class( klass, constraint->source, true );
        if( !source ) {
            return VF_ErrorLoadClass;
        }

        // check restriction
        if( !vf_is_extending( source, target ) ) {
            return VF_ErrorIncompatibleArgument;
        }
        return VF_OK;
    } // vf_force_check_constraint


    /**
    * Returns true if 'from' is (not necessarily directly) extending 'to'
    */
    int vf_is_extending(class_handler from, class_handler to) {
        while (from) {
            if( from == to ) return true;
            from = class_get_super_class(from);
        }
        return false;
    }

    /**
    * Function receives class by given class name, loads it if it's needed.
    */
    class_handler
        vf_resolve_class( class_handler k_class,    // current class
        const char *name,         // resolved class name
        bool need_load)      // load flag
    {
        class_handler result;

        // get class loader
        classloader_handler class_loader = class_get_class_loader( k_class );

        result = need_load ? cl_load_class( class_loader, name ) : cl_get_class( class_loader, name );

        //we assume that this pre-defined constant is not a valid class-handler
        assert(CLASS_NOT_LOADED != result);
        
        return result;
    } // vf_resolve_class


} // namespace CPVerifier