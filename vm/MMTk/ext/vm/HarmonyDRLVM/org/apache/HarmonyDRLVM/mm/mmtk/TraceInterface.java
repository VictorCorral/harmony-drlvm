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

/*
 * (C) Copyright Department of Computer Science,
 * Australian National University. 2004
 *
 * (C) Copyright IBM Corp. 2001, 2003
 */

package org.apache.HarmonyDRLVM.mm.mmtk;

import org.mmtk.utility.Constants;
import org.vmmagic.unboxed.*;
import org.vmmagic.pragma.*;

/**
 * Class that supports scanning Objects or Arrays for references
 * during tracing, handling those references, and computing death times
 * 
 * $Id: TraceInterface.java,v 1.3 2006/06/05 04:30:57 steveb-oss Exp $
 * 
 * @author <a href="http://www-ali.cs.umass.edu/~hertz">Matthew Hertz</a>
 * @version $Revision: 1.3 $
 * @date $Date: 2006/06/05 04:30:57 $
 */
public final class TraceInterface extends org.mmtk.vm.TraceInterface implements Uninterruptible {

  /***********************************************************************
   *
   * Class variables
   */
  private static byte allocCallMethods[][];

  static {
    /* Build the list of "own methods" */
      /*
    allocCallMethods = new byte[13][];
    allocCallMethods[0] = "postAlloc".getBytes();
    allocCallMethods[1] = "traceAlloc".getBytes();
    allocCallMethods[2] = "allocateScalar".getBytes();
    allocCallMethods[3] = "allocateArray".getBytes();
    allocCallMethods[4] = "clone".getBytes();
    allocCallMethods[5] = "alloc".getBytes();
    allocCallMethods[6] = "buildMultiDimensionalArray".getBytes();
    allocCallMethods[7] = "resolvedNewScalar".getBytes();
    allocCallMethods[8] = "resolvedNewArray".getBytes();
    allocCallMethods[9] = "unresolvedNewScalar".getBytes();
    allocCallMethods[10] = "unresolvedNewArray".getBytes();
    allocCallMethods[11] = "cloneScalar".getBytes();
    allocCallMethods[12] = "cloneArray".getBytes();
    */
  }

  /***********************************************************************
   *
   * Public Methods
   */

  /**
   * Returns if the VM is ready for a garbage collection.
   *
   * @return True if the RVM is ready for GC, false otherwise.
   */
  public final boolean gcEnabled() {
    System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.gcEnabled() was called");
    return false;
  }

  /**
   * Given a method name, determine if it is a "real" method or one
   * used for allocation/tracing.
   *
   * @param name The method name to test as an array of bytes
   * @return True if the method is a "real" method, false otherwise.
   */
  private final boolean isAllocCall(byte[] name) {    
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.isAllocCall() was called");
      return false;
  }

  /**
   * This adjusts the offset into an object to reflect what it would look like
   * if the fields were laid out in memory space immediately after the object
   * pointer.
   *
   * @param isScalar If this is a pointer store to a scalar object
   * @param src The address of the source object
   * @param slot The address within <code>src</code> into which
   * the update will be stored
   * @return The easy to understand offset of the slot
   */
  public final Offset adjustSlotOffset(boolean isScalar, 
                                              ObjectReference src,
                                              Address slot) {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.adjustSlotOffset() was called");
      return Offset.fromInt(0);
  }

  /**
   * This skips over the frames added by the tracing algorithm, outputs 
   * information identifying the method the containts the "new" call triggering
   * the allocation, and returns the address of the first non-trace, non-alloc
   * stack frame.
   *
   *@param typeRef The type reference (tib) of the object just allocated
   *@return The frame pointer address for the method that allocated the object
   */
  public final Address skipOwnFramesAndDump(ObjectReference typeRef)
    throws NoInlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.skipOwnFramesAndDump() was called");
      return Address.fromInt(0);
  }

  /***********************************************************************
   *
   * Wrapper methods
   */

  public final void updateDeathTime(Object obj) throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.updateDeathTime() was called");
  }

  public final void setDeathTime(ObjectReference ref, Word time_) 
    throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.setDeathTime() was called");
  }

  public final void setLink(ObjectReference ref, ObjectReference link) 
    throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.setLink() was called");
  }

  public final void updateTime(Word time_) throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.updateTime() was called");
  }

  public final Word getOID(ObjectReference ref) throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getOID() was called");
      return Word.fromInt(0);
  }

  public final Word getDeathTime(ObjectReference ref) throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getDeathTime() was called");
      return Word.fromInt(0);
  }

  public final ObjectReference getLink(ObjectReference ref)
    throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getLink() was called");
      return ObjectReference.nullReference();
  }

  public final Address getBootImageLink() throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getBootImageLink() was called");
      return Address.fromInt(0);
  }

  public final Word getOID() throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getOID() was called");
      return Word.fromInt(0);
  }

  public final void setOID(Word oid) throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.setOID() was called");
  }

  public final int getHeaderSize() throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getHeaderSize() was called");
      return 0;
  }

  public final int getHeaderEndOffset() throws InlinePragma {
      System.out.println("org.apache.HarmonyDRLVM.mm.mmtk.TraceInterface.getHeaderEndOffset() was called");
      return 0;
  }
}
