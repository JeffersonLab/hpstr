// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME EventDict

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "/u/re/alspellm/work/src/hpstr/event/include/EventDef.h"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_CalCluster(void *p = 0);
   static void *newArray_CalCluster(Long_t size, void *p);
   static void delete_CalCluster(void *p);
   static void deleteArray_CalCluster(void *p);
   static void destruct_CalCluster(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::CalCluster*)
   {
      ::CalCluster *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::CalCluster >(0);
      static ::ROOT::TGenericClassInfo 
         instance("CalCluster", ::CalCluster::Class_Version(), "CalCluster.h", 23,
                  typeid(::CalCluster), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::CalCluster::Dictionary, isa_proxy, 4,
                  sizeof(::CalCluster) );
      instance.SetNew(&new_CalCluster);
      instance.SetNewArray(&newArray_CalCluster);
      instance.SetDelete(&delete_CalCluster);
      instance.SetDeleteArray(&deleteArray_CalCluster);
      instance.SetDestructor(&destruct_CalCluster);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::CalCluster*)
   {
      return GenerateInitInstanceLocal((::CalCluster*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::CalCluster*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_CalHit(void *p = 0);
   static void *newArray_CalHit(Long_t size, void *p);
   static void delete_CalHit(void *p);
   static void deleteArray_CalHit(void *p);
   static void destruct_CalHit(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::CalHit*)
   {
      ::CalHit *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::CalHit >(0);
      static ::ROOT::TGenericClassInfo 
         instance("CalHit", ::CalHit::Class_Version(), "CalHit.h", 15,
                  typeid(::CalHit), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::CalHit::Dictionary, isa_proxy, 4,
                  sizeof(::CalHit) );
      instance.SetNew(&new_CalHit);
      instance.SetNewArray(&newArray_CalHit);
      instance.SetDelete(&delete_CalHit);
      instance.SetDeleteArray(&deleteArray_CalHit);
      instance.SetDestructor(&destruct_CalHit);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::CalHit*)
   {
      return GenerateInitInstanceLocal((::CalHit*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::CalHit*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_EventHeader(void *p = 0);
   static void *newArray_EventHeader(Long_t size, void *p);
   static void delete_EventHeader(void *p);
   static void deleteArray_EventHeader(void *p);
   static void destruct_EventHeader(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::EventHeader*)
   {
      ::EventHeader *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::EventHeader >(0);
      static ::ROOT::TGenericClassInfo 
         instance("EventHeader", ::EventHeader::Class_Version(), "EventHeader.h", 22,
                  typeid(::EventHeader), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::EventHeader::Dictionary, isa_proxy, 4,
                  sizeof(::EventHeader) );
      instance.SetNew(&new_EventHeader);
      instance.SetNewArray(&newArray_EventHeader);
      instance.SetDelete(&delete_EventHeader);
      instance.SetDeleteArray(&deleteArray_EventHeader);
      instance.SetDestructor(&destruct_EventHeader);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::EventHeader*)
   {
      return GenerateInitInstanceLocal((::EventHeader*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::EventHeader*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_VTPData(void *p = 0);
   static void *newArray_VTPData(Long_t size, void *p);
   static void delete_VTPData(void *p);
   static void deleteArray_VTPData(void *p);
   static void destruct_VTPData(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData*)
   {
      ::VTPData *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::VTPData >(0);
      static ::ROOT::TGenericClassInfo 
         instance("VTPData", ::VTPData::Class_Version(), "VTPData.h", 19,
                  typeid(::VTPData), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::VTPData::Dictionary, isa_proxy, 4,
                  sizeof(::VTPData) );
      instance.SetNew(&new_VTPData);
      instance.SetNewArray(&newArray_VTPData);
      instance.SetDelete(&delete_VTPData);
      instance.SetDeleteArray(&deleteArray_VTPData);
      instance.SetDestructor(&destruct_VTPData);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData*)
   {
      return GenerateInitInstanceLocal((::VTPData*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLbHeader_Dictionary();
   static void VTPDatacLcLbHeader_TClassManip(TClass*);
   static void *new_VTPDatacLcLbHeader(void *p = 0);
   static void *newArray_VTPDatacLcLbHeader(Long_t size, void *p);
   static void delete_VTPDatacLcLbHeader(void *p);
   static void deleteArray_VTPDatacLcLbHeader(void *p);
   static void destruct_VTPDatacLcLbHeader(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::bHeader*)
   {
      ::VTPData::bHeader *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::bHeader));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::bHeader", "VTPData.h", 22,
                  typeid(::VTPData::bHeader), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLbHeader_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::bHeader) );
      instance.SetNew(&new_VTPDatacLcLbHeader);
      instance.SetNewArray(&newArray_VTPDatacLcLbHeader);
      instance.SetDelete(&delete_VTPDatacLcLbHeader);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLbHeader);
      instance.SetDestructor(&destruct_VTPDatacLcLbHeader);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::bHeader*)
   {
      return GenerateInitInstanceLocal((::VTPData::bHeader*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::bHeader*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLbHeader_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::bHeader*)0x0)->GetClass();
      VTPDatacLcLbHeader_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLbHeader_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLbTail_Dictionary();
   static void VTPDatacLcLbTail_TClassManip(TClass*);
   static void *new_VTPDatacLcLbTail(void *p = 0);
   static void *newArray_VTPDatacLcLbTail(Long_t size, void *p);
   static void delete_VTPDatacLcLbTail(void *p);
   static void deleteArray_VTPDatacLcLbTail(void *p);
   static void destruct_VTPDatacLcLbTail(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::bTail*)
   {
      ::VTPData::bTail *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::bTail));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::bTail", "VTPData.h", 32,
                  typeid(::VTPData::bTail), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLbTail_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::bTail) );
      instance.SetNew(&new_VTPDatacLcLbTail);
      instance.SetNewArray(&newArray_VTPDatacLcLbTail);
      instance.SetDelete(&delete_VTPDatacLcLbTail);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLbTail);
      instance.SetDestructor(&destruct_VTPDatacLcLbTail);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::bTail*)
   {
      return GenerateInitInstanceLocal((::VTPData::bTail*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::bTail*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLbTail_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::bTail*)0x0)->GetClass();
      VTPDatacLcLbTail_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLbTail_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLeHeader_Dictionary();
   static void VTPDatacLcLeHeader_TClassManip(TClass*);
   static void *new_VTPDatacLcLeHeader(void *p = 0);
   static void *newArray_VTPDatacLcLeHeader(Long_t size, void *p);
   static void delete_VTPDatacLcLeHeader(void *p);
   static void deleteArray_VTPDatacLcLeHeader(void *p);
   static void destruct_VTPDatacLcLeHeader(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::eHeader*)
   {
      ::VTPData::eHeader *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::eHeader));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::eHeader", "VTPData.h", 40,
                  typeid(::VTPData::eHeader), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLeHeader_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::eHeader) );
      instance.SetNew(&new_VTPDatacLcLeHeader);
      instance.SetNewArray(&newArray_VTPDatacLcLeHeader);
      instance.SetDelete(&delete_VTPDatacLcLeHeader);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLeHeader);
      instance.SetDestructor(&destruct_VTPDatacLcLeHeader);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::eHeader*)
   {
      return GenerateInitInstanceLocal((::VTPData::eHeader*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::eHeader*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLeHeader_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::eHeader*)0x0)->GetClass();
      VTPDatacLcLeHeader_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLeHeader_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsCluster_Dictionary();
   static void VTPDatacLcLhpsCluster_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsCluster(void *p = 0);
   static void *newArray_VTPDatacLcLhpsCluster(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsCluster(void *p);
   static void deleteArray_VTPDatacLcLhpsCluster(void *p);
   static void destruct_VTPDatacLcLhpsCluster(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsCluster*)
   {
      ::VTPData::hpsCluster *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsCluster));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsCluster", "VTPData.h", 49,
                  typeid(::VTPData::hpsCluster), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsCluster_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsCluster) );
      instance.SetNew(&new_VTPDatacLcLhpsCluster);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsCluster);
      instance.SetDelete(&delete_VTPDatacLcLhpsCluster);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsCluster);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsCluster);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsCluster*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsCluster*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsCluster*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsCluster_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsCluster*)0x0)->GetClass();
      VTPDatacLcLhpsCluster_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsCluster_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsSingleTrig_Dictionary();
   static void VTPDatacLcLhpsSingleTrig_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsSingleTrig(void *p = 0);
   static void *newArray_VTPDatacLcLhpsSingleTrig(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsSingleTrig(void *p);
   static void deleteArray_VTPDatacLcLhpsSingleTrig(void *p);
   static void destruct_VTPDatacLcLhpsSingleTrig(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsSingleTrig*)
   {
      ::VTPData::hpsSingleTrig *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsSingleTrig));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsSingleTrig", "VTPData.h", 63,
                  typeid(::VTPData::hpsSingleTrig), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsSingleTrig_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsSingleTrig) );
      instance.SetNew(&new_VTPDatacLcLhpsSingleTrig);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsSingleTrig);
      instance.SetDelete(&delete_VTPDatacLcLhpsSingleTrig);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsSingleTrig);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsSingleTrig);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsSingleTrig*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsSingleTrig*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsSingleTrig*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsSingleTrig_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsSingleTrig*)0x0)->GetClass();
      VTPDatacLcLhpsSingleTrig_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsSingleTrig_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsPairTrig_Dictionary();
   static void VTPDatacLcLhpsPairTrig_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsPairTrig(void *p = 0);
   static void *newArray_VTPDatacLcLhpsPairTrig(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsPairTrig(void *p);
   static void deleteArray_VTPDatacLcLhpsPairTrig(void *p);
   static void destruct_VTPDatacLcLhpsPairTrig(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsPairTrig*)
   {
      ::VTPData::hpsPairTrig *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsPairTrig));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsPairTrig", "VTPData.h", 83,
                  typeid(::VTPData::hpsPairTrig), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsPairTrig_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsPairTrig) );
      instance.SetNew(&new_VTPDatacLcLhpsPairTrig);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsPairTrig);
      instance.SetDelete(&delete_VTPDatacLcLhpsPairTrig);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsPairTrig);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsPairTrig);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsPairTrig*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsPairTrig*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsPairTrig*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsPairTrig_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsPairTrig*)0x0)->GetClass();
      VTPDatacLcLhpsPairTrig_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsPairTrig_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsCalibTrig_Dictionary();
   static void VTPDatacLcLhpsCalibTrig_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsCalibTrig(void *p = 0);
   static void *newArray_VTPDatacLcLhpsCalibTrig(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsCalibTrig(void *p);
   static void deleteArray_VTPDatacLcLhpsCalibTrig(void *p);
   static void destruct_VTPDatacLcLhpsCalibTrig(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsCalibTrig*)
   {
      ::VTPData::hpsCalibTrig *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsCalibTrig));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsCalibTrig", "VTPData.h", 99,
                  typeid(::VTPData::hpsCalibTrig), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsCalibTrig_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsCalibTrig) );
      instance.SetNew(&new_VTPDatacLcLhpsCalibTrig);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsCalibTrig);
      instance.SetDelete(&delete_VTPDatacLcLhpsCalibTrig);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsCalibTrig);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsCalibTrig);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsCalibTrig*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsCalibTrig*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsCalibTrig*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsCalibTrig_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsCalibTrig*)0x0)->GetClass();
      VTPDatacLcLhpsCalibTrig_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsCalibTrig_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsClusterMult_Dictionary();
   static void VTPDatacLcLhpsClusterMult_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsClusterMult(void *p = 0);
   static void *newArray_VTPDatacLcLhpsClusterMult(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsClusterMult(void *p);
   static void deleteArray_VTPDatacLcLhpsClusterMult(void *p);
   static void destruct_VTPDatacLcLhpsClusterMult(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsClusterMult*)
   {
      ::VTPData::hpsClusterMult *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsClusterMult));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsClusterMult", "VTPData.h", 113,
                  typeid(::VTPData::hpsClusterMult), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsClusterMult_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsClusterMult) );
      instance.SetNew(&new_VTPDatacLcLhpsClusterMult);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsClusterMult);
      instance.SetDelete(&delete_VTPDatacLcLhpsClusterMult);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsClusterMult);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsClusterMult);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsClusterMult*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsClusterMult*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsClusterMult*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsClusterMult_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsClusterMult*)0x0)->GetClass();
      VTPDatacLcLhpsClusterMult_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsClusterMult_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *VTPDatacLcLhpsFEETrig_Dictionary();
   static void VTPDatacLcLhpsFEETrig_TClassManip(TClass*);
   static void *new_VTPDatacLcLhpsFEETrig(void *p = 0);
   static void *newArray_VTPDatacLcLhpsFEETrig(Long_t size, void *p);
   static void delete_VTPDatacLcLhpsFEETrig(void *p);
   static void deleteArray_VTPDatacLcLhpsFEETrig(void *p);
   static void destruct_VTPDatacLcLhpsFEETrig(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::VTPData::hpsFEETrig*)
   {
      ::VTPData::hpsFEETrig *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::VTPData::hpsFEETrig));
      static ::ROOT::TGenericClassInfo 
         instance("VTPData::hpsFEETrig", "VTPData.h", 126,
                  typeid(::VTPData::hpsFEETrig), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &VTPDatacLcLhpsFEETrig_Dictionary, isa_proxy, 4,
                  sizeof(::VTPData::hpsFEETrig) );
      instance.SetNew(&new_VTPDatacLcLhpsFEETrig);
      instance.SetNewArray(&newArray_VTPDatacLcLhpsFEETrig);
      instance.SetDelete(&delete_VTPDatacLcLhpsFEETrig);
      instance.SetDeleteArray(&deleteArray_VTPDatacLcLhpsFEETrig);
      instance.SetDestructor(&destruct_VTPDatacLcLhpsFEETrig);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::VTPData::hpsFEETrig*)
   {
      return GenerateInitInstanceLocal((::VTPData::hpsFEETrig*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::VTPData::hpsFEETrig*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *VTPDatacLcLhpsFEETrig_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::VTPData::hpsFEETrig*)0x0)->GetClass();
      VTPDatacLcLhpsFEETrig_TClassManip(theClass);
   return theClass;
   }

   static void VTPDatacLcLhpsFEETrig_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static void *new_TriggerData(void *p = 0);
   static void *newArray_TriggerData(Long_t size, void *p);
   static void delete_TriggerData(void *p);
   static void deleteArray_TriggerData(void *p);
   static void destruct_TriggerData(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TriggerData*)
   {
      ::TriggerData *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TriggerData >(0);
      static ::ROOT::TGenericClassInfo 
         instance("TriggerData", ::TriggerData::Class_Version(), "TriggerData.h", 26,
                  typeid(::TriggerData), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TriggerData::Dictionary, isa_proxy, 4,
                  sizeof(::TriggerData) );
      instance.SetNew(&new_TriggerData);
      instance.SetNewArray(&newArray_TriggerData);
      instance.SetDelete(&delete_TriggerData);
      instance.SetDeleteArray(&deleteArray_TriggerData);
      instance.SetDestructor(&destruct_TriggerData);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TriggerData*)
   {
      return GenerateInitInstanceLocal((::TriggerData*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TriggerData*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_TSData(void *p = 0);
   static void *newArray_TSData(Long_t size, void *p);
   static void delete_TSData(void *p);
   static void deleteArray_TSData(void *p);
   static void destruct_TSData(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TSData*)
   {
      ::TSData *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TSData >(0);
      static ::ROOT::TGenericClassInfo 
         instance("TSData", ::TSData::Class_Version(), "TSData.h", 23,
                  typeid(::TSData), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TSData::Dictionary, isa_proxy, 4,
                  sizeof(::TSData) );
      instance.SetNew(&new_TSData);
      instance.SetNewArray(&newArray_TSData);
      instance.SetDelete(&delete_TSData);
      instance.SetDeleteArray(&deleteArray_TSData);
      instance.SetDestructor(&destruct_TSData);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TSData*)
   {
      return GenerateInitInstanceLocal((::TSData*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TSData*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static TClass *TSDatacLcLtsHeader_Dictionary();
   static void TSDatacLcLtsHeader_TClassManip(TClass*);
   static void *new_TSDatacLcLtsHeader(void *p = 0);
   static void *newArray_TSDatacLcLtsHeader(Long_t size, void *p);
   static void delete_TSDatacLcLtsHeader(void *p);
   static void deleteArray_TSDatacLcLtsHeader(void *p);
   static void destruct_TSDatacLcLtsHeader(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TSData::tsHeader*)
   {
      ::TSData::tsHeader *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TSData::tsHeader));
      static ::ROOT::TGenericClassInfo 
         instance("TSData::tsHeader", "TSData.h", 34,
                  typeid(::TSData::tsHeader), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TSDatacLcLtsHeader_Dictionary, isa_proxy, 4,
                  sizeof(::TSData::tsHeader) );
      instance.SetNew(&new_TSDatacLcLtsHeader);
      instance.SetNewArray(&newArray_TSDatacLcLtsHeader);
      instance.SetDelete(&delete_TSDatacLcLtsHeader);
      instance.SetDeleteArray(&deleteArray_TSDatacLcLtsHeader);
      instance.SetDestructor(&destruct_TSDatacLcLtsHeader);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TSData::tsHeader*)
   {
      return GenerateInitInstanceLocal((::TSData::tsHeader*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TSData::tsHeader*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TSDatacLcLtsHeader_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::TSData::tsHeader*)0x0)->GetClass();
      TSDatacLcLtsHeader_TClassManip(theClass);
   return theClass;
   }

   static void TSDatacLcLtsHeader_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static TClass *TSDatacLcLtsBits_Dictionary();
   static void TSDatacLcLtsBits_TClassManip(TClass*);
   static void *new_TSDatacLcLtsBits(void *p = 0);
   static void *newArray_TSDatacLcLtsBits(Long_t size, void *p);
   static void delete_TSDatacLcLtsBits(void *p);
   static void deleteArray_TSDatacLcLtsBits(void *p);
   static void destruct_TSDatacLcLtsBits(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TSData::tsBits*)
   {
      ::TSData::tsBits *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(::TSData::tsBits));
      static ::ROOT::TGenericClassInfo 
         instance("TSData::tsBits", "TSData.h", 41,
                  typeid(::TSData::tsBits), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &TSDatacLcLtsBits_Dictionary, isa_proxy, 4,
                  sizeof(::TSData::tsBits) );
      instance.SetNew(&new_TSDatacLcLtsBits);
      instance.SetNewArray(&newArray_TSDatacLcLtsBits);
      instance.SetDelete(&delete_TSDatacLcLtsBits);
      instance.SetDeleteArray(&deleteArray_TSDatacLcLtsBits);
      instance.SetDestructor(&destruct_TSDatacLcLtsBits);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TSData::tsBits*)
   {
      return GenerateInitInstanceLocal((::TSData::tsBits*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TSData::tsBits*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *TSDatacLcLtsBits_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const ::TSData::tsBits*)0x0)->GetClass();
      TSDatacLcLtsBits_TClassManip(theClass);
   return theClass;
   }

   static void TSDatacLcLtsBits_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static void *new_Particle(void *p = 0);
   static void *newArray_Particle(Long_t size, void *p);
   static void delete_Particle(void *p);
   static void deleteArray_Particle(void *p);
   static void destruct_Particle(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Particle*)
   {
      ::Particle *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Particle >(0);
      static ::ROOT::TGenericClassInfo 
         instance("Particle", ::Particle::Class_Version(), "Particle.h", 19,
                  typeid(::Particle), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Particle::Dictionary, isa_proxy, 4,
                  sizeof(::Particle) );
      instance.SetNew(&new_Particle);
      instance.SetNewArray(&newArray_Particle);
      instance.SetDelete(&delete_Particle);
      instance.SetDeleteArray(&deleteArray_Particle);
      instance.SetDestructor(&destruct_Particle);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Particle*)
   {
      return GenerateInitInstanceLocal((::Particle*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Particle*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Track(void *p = 0);
   static void *newArray_Track(Long_t size, void *p);
   static void delete_Track(void *p);
   static void deleteArray_Track(void *p);
   static void destruct_Track(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Track*)
   {
      ::Track *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Track >(0);
      static ::ROOT::TGenericClassInfo 
         instance("Track", ::Track::Class_Version(), "Track.h", 29,
                  typeid(::Track), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Track::Dictionary, isa_proxy, 4,
                  sizeof(::Track) );
      instance.SetNew(&new_Track);
      instance.SetNewArray(&newArray_Track);
      instance.SetDelete(&delete_Track);
      instance.SetDeleteArray(&deleteArray_Track);
      instance.SetDestructor(&destruct_Track);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Track*)
   {
      return GenerateInitInstanceLocal((::Track*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Track*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_Vertex(void *p = 0);
   static void *newArray_Vertex(Long_t size, void *p);
   static void delete_Vertex(void *p);
   static void deleteArray_Vertex(void *p);
   static void destruct_Vertex(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Vertex*)
   {
      ::Vertex *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Vertex >(0);
      static ::ROOT::TGenericClassInfo 
         instance("Vertex", ::Vertex::Class_Version(), "Vertex.h", 20,
                  typeid(::Vertex), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Vertex::Dictionary, isa_proxy, 4,
                  sizeof(::Vertex) );
      instance.SetNew(&new_Vertex);
      instance.SetNewArray(&newArray_Vertex);
      instance.SetDelete(&delete_Vertex);
      instance.SetDeleteArray(&deleteArray_Vertex);
      instance.SetDestructor(&destruct_Vertex);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Vertex*)
   {
      return GenerateInitInstanceLocal((::Vertex*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Vertex*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_TrackerHit(void *p = 0);
   static void *newArray_TrackerHit(Long_t size, void *p);
   static void delete_TrackerHit(void *p);
   static void deleteArray_TrackerHit(void *p);
   static void destruct_TrackerHit(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TrackerHit*)
   {
      ::TrackerHit *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TrackerHit >(0);
      static ::ROOT::TGenericClassInfo 
         instance("TrackerHit", ::TrackerHit::Class_Version(), "TrackerHit.h", 22,
                  typeid(::TrackerHit), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TrackerHit::Dictionary, isa_proxy, 4,
                  sizeof(::TrackerHit) );
      instance.SetNew(&new_TrackerHit);
      instance.SetNewArray(&newArray_TrackerHit);
      instance.SetDelete(&delete_TrackerHit);
      instance.SetDeleteArray(&deleteArray_TrackerHit);
      instance.SetDestructor(&destruct_TrackerHit);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TrackerHit*)
   {
      return GenerateInitInstanceLocal((::TrackerHit*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TrackerHit*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_RawSvtHit(void *p = 0);
   static void *newArray_RawSvtHit(Long_t size, void *p);
   static void delete_RawSvtHit(void *p);
   static void deleteArray_RawSvtHit(void *p);
   static void destruct_RawSvtHit(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RawSvtHit*)
   {
      ::RawSvtHit *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RawSvtHit >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RawSvtHit", ::RawSvtHit::Class_Version(), "RawSvtHit.h", 21,
                  typeid(::RawSvtHit), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RawSvtHit::Dictionary, isa_proxy, 4,
                  sizeof(::RawSvtHit) );
      instance.SetNew(&new_RawSvtHit);
      instance.SetNewArray(&newArray_RawSvtHit);
      instance.SetDelete(&delete_RawSvtHit);
      instance.SetDeleteArray(&deleteArray_RawSvtHit);
      instance.SetDestructor(&destruct_RawSvtHit);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RawSvtHit*)
   {
      return GenerateInitInstanceLocal((::RawSvtHit*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::RawSvtHit*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr CalCluster::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *CalCluster::Class_Name()
{
   return "CalCluster";
}

//______________________________________________________________________________
const char *CalCluster::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CalCluster*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int CalCluster::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CalCluster*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *CalCluster::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CalCluster*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *CalCluster::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CalCluster*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr CalHit::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *CalHit::Class_Name()
{
   return "CalHit";
}

//______________________________________________________________________________
const char *CalHit::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CalHit*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int CalHit::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CalHit*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *CalHit::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CalHit*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *CalHit::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CalHit*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr EventHeader::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *EventHeader::Class_Name()
{
   return "EventHeader";
}

//______________________________________________________________________________
const char *EventHeader::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::EventHeader*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int EventHeader::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::EventHeader*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *EventHeader::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::EventHeader*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *EventHeader::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::EventHeader*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr VTPData::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *VTPData::Class_Name()
{
   return "VTPData";
}

//______________________________________________________________________________
const char *VTPData::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::VTPData*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int VTPData::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::VTPData*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *VTPData::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::VTPData*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *VTPData::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::VTPData*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr TriggerData::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *TriggerData::Class_Name()
{
   return "TriggerData";
}

//______________________________________________________________________________
const char *TriggerData::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TriggerData*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int TriggerData::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TriggerData*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TriggerData::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TriggerData*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TriggerData::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TriggerData*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr TSData::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *TSData::Class_Name()
{
   return "TSData";
}

//______________________________________________________________________________
const char *TSData::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TSData*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int TSData::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TSData*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TSData::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TSData*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TSData::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TSData*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Particle::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *Particle::Class_Name()
{
   return "Particle";
}

//______________________________________________________________________________
const char *Particle::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Particle*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int Particle::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Particle*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Particle::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Particle*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Particle::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Particle*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Track::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *Track::Class_Name()
{
   return "Track";
}

//______________________________________________________________________________
const char *Track::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Track*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int Track::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Track*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Track::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Track*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Track::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Track*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr Vertex::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *Vertex::Class_Name()
{
   return "Vertex";
}

//______________________________________________________________________________
const char *Vertex::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Vertex*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int Vertex::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Vertex*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Vertex::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Vertex*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Vertex::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Vertex*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr TrackerHit::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *TrackerHit::Class_Name()
{
   return "TrackerHit";
}

//______________________________________________________________________________
const char *TrackerHit::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TrackerHit*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int TrackerHit::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TrackerHit*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TrackerHit::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TrackerHit*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TrackerHit::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TrackerHit*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr RawSvtHit::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RawSvtHit::Class_Name()
{
   return "RawSvtHit";
}

//______________________________________________________________________________
const char *RawSvtHit::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RawSvtHit*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RawSvtHit::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RawSvtHit*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RawSvtHit::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RawSvtHit*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RawSvtHit::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RawSvtHit*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void CalCluster::Streamer(TBuffer &R__b)
{
   // Stream an object of class CalCluster.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(CalCluster::Class(),this);
   } else {
      R__b.WriteClassBuffer(CalCluster::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_CalCluster(void *p) {
      return  p ? new(p) ::CalCluster : new ::CalCluster;
   }
   static void *newArray_CalCluster(Long_t nElements, void *p) {
      return p ? new(p) ::CalCluster[nElements] : new ::CalCluster[nElements];
   }
   // Wrapper around operator delete
   static void delete_CalCluster(void *p) {
      delete ((::CalCluster*)p);
   }
   static void deleteArray_CalCluster(void *p) {
      delete [] ((::CalCluster*)p);
   }
   static void destruct_CalCluster(void *p) {
      typedef ::CalCluster current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::CalCluster

//______________________________________________________________________________
void CalHit::Streamer(TBuffer &R__b)
{
   // Stream an object of class CalHit.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(CalHit::Class(),this);
   } else {
      R__b.WriteClassBuffer(CalHit::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_CalHit(void *p) {
      return  p ? new(p) ::CalHit : new ::CalHit;
   }
   static void *newArray_CalHit(Long_t nElements, void *p) {
      return p ? new(p) ::CalHit[nElements] : new ::CalHit[nElements];
   }
   // Wrapper around operator delete
   static void delete_CalHit(void *p) {
      delete ((::CalHit*)p);
   }
   static void deleteArray_CalHit(void *p) {
      delete [] ((::CalHit*)p);
   }
   static void destruct_CalHit(void *p) {
      typedef ::CalHit current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::CalHit

//______________________________________________________________________________
void EventHeader::Streamer(TBuffer &R__b)
{
   // Stream an object of class EventHeader.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(EventHeader::Class(),this);
   } else {
      R__b.WriteClassBuffer(EventHeader::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_EventHeader(void *p) {
      return  p ? new(p) ::EventHeader : new ::EventHeader;
   }
   static void *newArray_EventHeader(Long_t nElements, void *p) {
      return p ? new(p) ::EventHeader[nElements] : new ::EventHeader[nElements];
   }
   // Wrapper around operator delete
   static void delete_EventHeader(void *p) {
      delete ((::EventHeader*)p);
   }
   static void deleteArray_EventHeader(void *p) {
      delete [] ((::EventHeader*)p);
   }
   static void destruct_EventHeader(void *p) {
      typedef ::EventHeader current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::EventHeader

//______________________________________________________________________________
void VTPData::Streamer(TBuffer &R__b)
{
   // Stream an object of class VTPData.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(VTPData::Class(),this);
   } else {
      R__b.WriteClassBuffer(VTPData::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPData(void *p) {
      return  p ? new(p) ::VTPData : new ::VTPData;
   }
   static void *newArray_VTPData(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData[nElements] : new ::VTPData[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPData(void *p) {
      delete ((::VTPData*)p);
   }
   static void deleteArray_VTPData(void *p) {
      delete [] ((::VTPData*)p);
   }
   static void destruct_VTPData(void *p) {
      typedef ::VTPData current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLbHeader(void *p) {
      return  p ? new(p) ::VTPData::bHeader : new ::VTPData::bHeader;
   }
   static void *newArray_VTPDatacLcLbHeader(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::bHeader[nElements] : new ::VTPData::bHeader[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLbHeader(void *p) {
      delete ((::VTPData::bHeader*)p);
   }
   static void deleteArray_VTPDatacLcLbHeader(void *p) {
      delete [] ((::VTPData::bHeader*)p);
   }
   static void destruct_VTPDatacLcLbHeader(void *p) {
      typedef ::VTPData::bHeader current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::bHeader

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLbTail(void *p) {
      return  p ? new(p) ::VTPData::bTail : new ::VTPData::bTail;
   }
   static void *newArray_VTPDatacLcLbTail(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::bTail[nElements] : new ::VTPData::bTail[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLbTail(void *p) {
      delete ((::VTPData::bTail*)p);
   }
   static void deleteArray_VTPDatacLcLbTail(void *p) {
      delete [] ((::VTPData::bTail*)p);
   }
   static void destruct_VTPDatacLcLbTail(void *p) {
      typedef ::VTPData::bTail current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::bTail

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLeHeader(void *p) {
      return  p ? new(p) ::VTPData::eHeader : new ::VTPData::eHeader;
   }
   static void *newArray_VTPDatacLcLeHeader(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::eHeader[nElements] : new ::VTPData::eHeader[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLeHeader(void *p) {
      delete ((::VTPData::eHeader*)p);
   }
   static void deleteArray_VTPDatacLcLeHeader(void *p) {
      delete [] ((::VTPData::eHeader*)p);
   }
   static void destruct_VTPDatacLcLeHeader(void *p) {
      typedef ::VTPData::eHeader current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::eHeader

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsCluster(void *p) {
      return  p ? new(p) ::VTPData::hpsCluster : new ::VTPData::hpsCluster;
   }
   static void *newArray_VTPDatacLcLhpsCluster(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsCluster[nElements] : new ::VTPData::hpsCluster[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsCluster(void *p) {
      delete ((::VTPData::hpsCluster*)p);
   }
   static void deleteArray_VTPDatacLcLhpsCluster(void *p) {
      delete [] ((::VTPData::hpsCluster*)p);
   }
   static void destruct_VTPDatacLcLhpsCluster(void *p) {
      typedef ::VTPData::hpsCluster current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsCluster

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsSingleTrig(void *p) {
      return  p ? new(p) ::VTPData::hpsSingleTrig : new ::VTPData::hpsSingleTrig;
   }
   static void *newArray_VTPDatacLcLhpsSingleTrig(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsSingleTrig[nElements] : new ::VTPData::hpsSingleTrig[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsSingleTrig(void *p) {
      delete ((::VTPData::hpsSingleTrig*)p);
   }
   static void deleteArray_VTPDatacLcLhpsSingleTrig(void *p) {
      delete [] ((::VTPData::hpsSingleTrig*)p);
   }
   static void destruct_VTPDatacLcLhpsSingleTrig(void *p) {
      typedef ::VTPData::hpsSingleTrig current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsSingleTrig

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsPairTrig(void *p) {
      return  p ? new(p) ::VTPData::hpsPairTrig : new ::VTPData::hpsPairTrig;
   }
   static void *newArray_VTPDatacLcLhpsPairTrig(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsPairTrig[nElements] : new ::VTPData::hpsPairTrig[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsPairTrig(void *p) {
      delete ((::VTPData::hpsPairTrig*)p);
   }
   static void deleteArray_VTPDatacLcLhpsPairTrig(void *p) {
      delete [] ((::VTPData::hpsPairTrig*)p);
   }
   static void destruct_VTPDatacLcLhpsPairTrig(void *p) {
      typedef ::VTPData::hpsPairTrig current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsPairTrig

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsCalibTrig(void *p) {
      return  p ? new(p) ::VTPData::hpsCalibTrig : new ::VTPData::hpsCalibTrig;
   }
   static void *newArray_VTPDatacLcLhpsCalibTrig(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsCalibTrig[nElements] : new ::VTPData::hpsCalibTrig[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsCalibTrig(void *p) {
      delete ((::VTPData::hpsCalibTrig*)p);
   }
   static void deleteArray_VTPDatacLcLhpsCalibTrig(void *p) {
      delete [] ((::VTPData::hpsCalibTrig*)p);
   }
   static void destruct_VTPDatacLcLhpsCalibTrig(void *p) {
      typedef ::VTPData::hpsCalibTrig current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsCalibTrig

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsClusterMult(void *p) {
      return  p ? new(p) ::VTPData::hpsClusterMult : new ::VTPData::hpsClusterMult;
   }
   static void *newArray_VTPDatacLcLhpsClusterMult(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsClusterMult[nElements] : new ::VTPData::hpsClusterMult[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsClusterMult(void *p) {
      delete ((::VTPData::hpsClusterMult*)p);
   }
   static void deleteArray_VTPDatacLcLhpsClusterMult(void *p) {
      delete [] ((::VTPData::hpsClusterMult*)p);
   }
   static void destruct_VTPDatacLcLhpsClusterMult(void *p) {
      typedef ::VTPData::hpsClusterMult current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsClusterMult

namespace ROOT {
   // Wrappers around operator new
   static void *new_VTPDatacLcLhpsFEETrig(void *p) {
      return  p ? new(p) ::VTPData::hpsFEETrig : new ::VTPData::hpsFEETrig;
   }
   static void *newArray_VTPDatacLcLhpsFEETrig(Long_t nElements, void *p) {
      return p ? new(p) ::VTPData::hpsFEETrig[nElements] : new ::VTPData::hpsFEETrig[nElements];
   }
   // Wrapper around operator delete
   static void delete_VTPDatacLcLhpsFEETrig(void *p) {
      delete ((::VTPData::hpsFEETrig*)p);
   }
   static void deleteArray_VTPDatacLcLhpsFEETrig(void *p) {
      delete [] ((::VTPData::hpsFEETrig*)p);
   }
   static void destruct_VTPDatacLcLhpsFEETrig(void *p) {
      typedef ::VTPData::hpsFEETrig current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::VTPData::hpsFEETrig

//______________________________________________________________________________
void TriggerData::Streamer(TBuffer &R__b)
{
   // Stream an object of class TriggerData.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TriggerData::Class(),this);
   } else {
      R__b.WriteClassBuffer(TriggerData::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TriggerData(void *p) {
      return  p ? new(p) ::TriggerData : new ::TriggerData;
   }
   static void *newArray_TriggerData(Long_t nElements, void *p) {
      return p ? new(p) ::TriggerData[nElements] : new ::TriggerData[nElements];
   }
   // Wrapper around operator delete
   static void delete_TriggerData(void *p) {
      delete ((::TriggerData*)p);
   }
   static void deleteArray_TriggerData(void *p) {
      delete [] ((::TriggerData*)p);
   }
   static void destruct_TriggerData(void *p) {
      typedef ::TriggerData current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TriggerData

//______________________________________________________________________________
void TSData::Streamer(TBuffer &R__b)
{
   // Stream an object of class TSData.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TSData::Class(),this);
   } else {
      R__b.WriteClassBuffer(TSData::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TSData(void *p) {
      return  p ? new(p) ::TSData : new ::TSData;
   }
   static void *newArray_TSData(Long_t nElements, void *p) {
      return p ? new(p) ::TSData[nElements] : new ::TSData[nElements];
   }
   // Wrapper around operator delete
   static void delete_TSData(void *p) {
      delete ((::TSData*)p);
   }
   static void deleteArray_TSData(void *p) {
      delete [] ((::TSData*)p);
   }
   static void destruct_TSData(void *p) {
      typedef ::TSData current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TSData

namespace ROOT {
   // Wrappers around operator new
   static void *new_TSDatacLcLtsHeader(void *p) {
      return  p ? new(p) ::TSData::tsHeader : new ::TSData::tsHeader;
   }
   static void *newArray_TSDatacLcLtsHeader(Long_t nElements, void *p) {
      return p ? new(p) ::TSData::tsHeader[nElements] : new ::TSData::tsHeader[nElements];
   }
   // Wrapper around operator delete
   static void delete_TSDatacLcLtsHeader(void *p) {
      delete ((::TSData::tsHeader*)p);
   }
   static void deleteArray_TSDatacLcLtsHeader(void *p) {
      delete [] ((::TSData::tsHeader*)p);
   }
   static void destruct_TSDatacLcLtsHeader(void *p) {
      typedef ::TSData::tsHeader current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TSData::tsHeader

namespace ROOT {
   // Wrappers around operator new
   static void *new_TSDatacLcLtsBits(void *p) {
      return  p ? new(p) ::TSData::tsBits : new ::TSData::tsBits;
   }
   static void *newArray_TSDatacLcLtsBits(Long_t nElements, void *p) {
      return p ? new(p) ::TSData::tsBits[nElements] : new ::TSData::tsBits[nElements];
   }
   // Wrapper around operator delete
   static void delete_TSDatacLcLtsBits(void *p) {
      delete ((::TSData::tsBits*)p);
   }
   static void deleteArray_TSDatacLcLtsBits(void *p) {
      delete [] ((::TSData::tsBits*)p);
   }
   static void destruct_TSDatacLcLtsBits(void *p) {
      typedef ::TSData::tsBits current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TSData::tsBits

//______________________________________________________________________________
void Particle::Streamer(TBuffer &R__b)
{
   // Stream an object of class Particle.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Particle::Class(),this);
   } else {
      R__b.WriteClassBuffer(Particle::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Particle(void *p) {
      return  p ? new(p) ::Particle : new ::Particle;
   }
   static void *newArray_Particle(Long_t nElements, void *p) {
      return p ? new(p) ::Particle[nElements] : new ::Particle[nElements];
   }
   // Wrapper around operator delete
   static void delete_Particle(void *p) {
      delete ((::Particle*)p);
   }
   static void deleteArray_Particle(void *p) {
      delete [] ((::Particle*)p);
   }
   static void destruct_Particle(void *p) {
      typedef ::Particle current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Particle

//______________________________________________________________________________
void Track::Streamer(TBuffer &R__b)
{
   // Stream an object of class Track.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Track::Class(),this);
   } else {
      R__b.WriteClassBuffer(Track::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Track(void *p) {
      return  p ? new(p) ::Track : new ::Track;
   }
   static void *newArray_Track(Long_t nElements, void *p) {
      return p ? new(p) ::Track[nElements] : new ::Track[nElements];
   }
   // Wrapper around operator delete
   static void delete_Track(void *p) {
      delete ((::Track*)p);
   }
   static void deleteArray_Track(void *p) {
      delete [] ((::Track*)p);
   }
   static void destruct_Track(void *p) {
      typedef ::Track current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Track

//______________________________________________________________________________
void Vertex::Streamer(TBuffer &R__b)
{
   // Stream an object of class Vertex.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Vertex::Class(),this);
   } else {
      R__b.WriteClassBuffer(Vertex::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Vertex(void *p) {
      return  p ? new(p) ::Vertex : new ::Vertex;
   }
   static void *newArray_Vertex(Long_t nElements, void *p) {
      return p ? new(p) ::Vertex[nElements] : new ::Vertex[nElements];
   }
   // Wrapper around operator delete
   static void delete_Vertex(void *p) {
      delete ((::Vertex*)p);
   }
   static void deleteArray_Vertex(void *p) {
      delete [] ((::Vertex*)p);
   }
   static void destruct_Vertex(void *p) {
      typedef ::Vertex current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Vertex

//______________________________________________________________________________
void TrackerHit::Streamer(TBuffer &R__b)
{
   // Stream an object of class TrackerHit.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TrackerHit::Class(),this);
   } else {
      R__b.WriteClassBuffer(TrackerHit::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TrackerHit(void *p) {
      return  p ? new(p) ::TrackerHit : new ::TrackerHit;
   }
   static void *newArray_TrackerHit(Long_t nElements, void *p) {
      return p ? new(p) ::TrackerHit[nElements] : new ::TrackerHit[nElements];
   }
   // Wrapper around operator delete
   static void delete_TrackerHit(void *p) {
      delete ((::TrackerHit*)p);
   }
   static void deleteArray_TrackerHit(void *p) {
      delete [] ((::TrackerHit*)p);
   }
   static void destruct_TrackerHit(void *p) {
      typedef ::TrackerHit current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TrackerHit

//______________________________________________________________________________
void RawSvtHit::Streamer(TBuffer &R__b)
{
   // Stream an object of class RawSvtHit.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(RawSvtHit::Class(),this);
   } else {
      R__b.WriteClassBuffer(RawSvtHit::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RawSvtHit(void *p) {
      return  p ? new(p) ::RawSvtHit : new ::RawSvtHit;
   }
   static void *newArray_RawSvtHit(Long_t nElements, void *p) {
      return p ? new(p) ::RawSvtHit[nElements] : new ::RawSvtHit[nElements];
   }
   // Wrapper around operator delete
   static void delete_RawSvtHit(void *p) {
      delete ((::RawSvtHit*)p);
   }
   static void deleteArray_RawSvtHit(void *p) {
      delete [] ((::RawSvtHit*)p);
   }
   static void destruct_RawSvtHit(void *p) {
      typedef ::RawSvtHit current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::RawSvtHit

namespace ROOT {
   static TClass *vectorlEfloatgR_Dictionary();
   static void vectorlEfloatgR_TClassManip(TClass*);
   static void *new_vectorlEfloatgR(void *p = 0);
   static void *newArray_vectorlEfloatgR(Long_t size, void *p);
   static void delete_vectorlEfloatgR(void *p);
   static void deleteArray_vectorlEfloatgR(void *p);
   static void destruct_vectorlEfloatgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<float>*)
   {
      vector<float> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<float>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<float>", -2, "vector", 339,
                  typeid(vector<float>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEfloatgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<float>) );
      instance.SetNew(&new_vectorlEfloatgR);
      instance.SetNewArray(&newArray_vectorlEfloatgR);
      instance.SetDelete(&delete_vectorlEfloatgR);
      instance.SetDeleteArray(&deleteArray_vectorlEfloatgR);
      instance.SetDestructor(&destruct_vectorlEfloatgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<float> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<float>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEfloatgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<float>*)0x0)->GetClass();
      vectorlEfloatgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEfloatgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEfloatgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<float> : new vector<float>;
   }
   static void *newArray_vectorlEfloatgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<float>[nElements] : new vector<float>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEfloatgR(void *p) {
      delete ((vector<float>*)p);
   }
   static void deleteArray_vectorlEfloatgR(void *p) {
      delete [] ((vector<float>*)p);
   }
   static void destruct_vectorlEfloatgR(void *p) {
      typedef vector<float> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<float>

namespace ROOT {
   static TClass *vectorlEVertexgR_Dictionary();
   static void vectorlEVertexgR_TClassManip(TClass*);
   static void *new_vectorlEVertexgR(void *p = 0);
   static void *newArray_vectorlEVertexgR(Long_t size, void *p);
   static void delete_vectorlEVertexgR(void *p);
   static void deleteArray_vectorlEVertexgR(void *p);
   static void destruct_vectorlEVertexgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<Vertex>*)
   {
      vector<Vertex> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<Vertex>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<Vertex>", -2, "vector", 339,
                  typeid(vector<Vertex>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVertexgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<Vertex>) );
      instance.SetNew(&new_vectorlEVertexgR);
      instance.SetNewArray(&newArray_vectorlEVertexgR);
      instance.SetDelete(&delete_vectorlEVertexgR);
      instance.SetDeleteArray(&deleteArray_vectorlEVertexgR);
      instance.SetDestructor(&destruct_vectorlEVertexgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<Vertex> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<Vertex>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVertexgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<Vertex>*)0x0)->GetClass();
      vectorlEVertexgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVertexgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVertexgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Vertex> : new vector<Vertex>;
   }
   static void *newArray_vectorlEVertexgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Vertex>[nElements] : new vector<Vertex>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVertexgR(void *p) {
      delete ((vector<Vertex>*)p);
   }
   static void deleteArray_vectorlEVertexgR(void *p) {
      delete [] ((vector<Vertex>*)p);
   }
   static void destruct_vectorlEVertexgR(void *p) {
      typedef vector<Vertex> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<Vertex>

namespace ROOT {
   static TClass *vectorlEVertexmUgR_Dictionary();
   static void vectorlEVertexmUgR_TClassManip(TClass*);
   static void *new_vectorlEVertexmUgR(void *p = 0);
   static void *newArray_vectorlEVertexmUgR(Long_t size, void *p);
   static void delete_vectorlEVertexmUgR(void *p);
   static void deleteArray_vectorlEVertexmUgR(void *p);
   static void destruct_vectorlEVertexmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<Vertex*>*)
   {
      vector<Vertex*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<Vertex*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<Vertex*>", -2, "vector", 339,
                  typeid(vector<Vertex*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVertexmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<Vertex*>) );
      instance.SetNew(&new_vectorlEVertexmUgR);
      instance.SetNewArray(&newArray_vectorlEVertexmUgR);
      instance.SetDelete(&delete_vectorlEVertexmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlEVertexmUgR);
      instance.SetDestructor(&destruct_vectorlEVertexmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<Vertex*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<Vertex*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVertexmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<Vertex*>*)0x0)->GetClass();
      vectorlEVertexmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVertexmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVertexmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Vertex*> : new vector<Vertex*>;
   }
   static void *newArray_vectorlEVertexmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Vertex*>[nElements] : new vector<Vertex*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVertexmUgR(void *p) {
      delete ((vector<Vertex*>*)p);
   }
   static void deleteArray_vectorlEVertexmUgR(void *p) {
      delete [] ((vector<Vertex*>*)p);
   }
   static void destruct_vectorlEVertexmUgR(void *p) {
      typedef vector<Vertex*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<Vertex*>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsSingleTriggR_Dictionary();
   static void vectorlEVTPDatacLcLhpsSingleTriggR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsSingleTriggR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsSingleTriggR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsSingleTriggR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsSingleTriggR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsSingleTriggR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsSingleTrig>*)
   {
      vector<VTPData::hpsSingleTrig> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsSingleTrig>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsSingleTrig>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsSingleTrig>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsSingleTriggR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsSingleTrig>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsSingleTriggR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsSingleTriggR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsSingleTriggR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsSingleTriggR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsSingleTriggR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsSingleTrig> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsSingleTrig>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsSingleTriggR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsSingleTrig>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsSingleTriggR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsSingleTriggR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsSingleTriggR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsSingleTrig> : new vector<VTPData::hpsSingleTrig>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsSingleTriggR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsSingleTrig>[nElements] : new vector<VTPData::hpsSingleTrig>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsSingleTriggR(void *p) {
      delete ((vector<VTPData::hpsSingleTrig>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsSingleTriggR(void *p) {
      delete [] ((vector<VTPData::hpsSingleTrig>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsSingleTriggR(void *p) {
      typedef vector<VTPData::hpsSingleTrig> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsSingleTrig>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsPairTriggR_Dictionary();
   static void vectorlEVTPDatacLcLhpsPairTriggR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsPairTriggR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsPairTriggR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsPairTriggR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsPairTriggR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsPairTriggR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsPairTrig>*)
   {
      vector<VTPData::hpsPairTrig> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsPairTrig>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsPairTrig>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsPairTrig>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsPairTriggR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsPairTrig>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsPairTriggR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsPairTriggR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsPairTriggR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsPairTriggR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsPairTriggR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsPairTrig> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsPairTrig>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsPairTriggR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsPairTrig>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsPairTriggR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsPairTriggR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsPairTriggR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsPairTrig> : new vector<VTPData::hpsPairTrig>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsPairTriggR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsPairTrig>[nElements] : new vector<VTPData::hpsPairTrig>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsPairTriggR(void *p) {
      delete ((vector<VTPData::hpsPairTrig>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsPairTriggR(void *p) {
      delete [] ((vector<VTPData::hpsPairTrig>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsPairTriggR(void *p) {
      typedef vector<VTPData::hpsPairTrig> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsPairTrig>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsFEETriggR_Dictionary();
   static void vectorlEVTPDatacLcLhpsFEETriggR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsFEETriggR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsFEETriggR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsFEETriggR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsFEETriggR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsFEETriggR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsFEETrig>*)
   {
      vector<VTPData::hpsFEETrig> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsFEETrig>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsFEETrig>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsFEETrig>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsFEETriggR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsFEETrig>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsFEETriggR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsFEETriggR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsFEETriggR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsFEETriggR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsFEETriggR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsFEETrig> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsFEETrig>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsFEETriggR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsFEETrig>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsFEETriggR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsFEETriggR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsFEETriggR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsFEETrig> : new vector<VTPData::hpsFEETrig>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsFEETriggR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsFEETrig>[nElements] : new vector<VTPData::hpsFEETrig>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsFEETriggR(void *p) {
      delete ((vector<VTPData::hpsFEETrig>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsFEETriggR(void *p) {
      delete [] ((vector<VTPData::hpsFEETrig>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsFEETriggR(void *p) {
      typedef vector<VTPData::hpsFEETrig> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsFEETrig>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsClusterMultgR_Dictionary();
   static void vectorlEVTPDatacLcLhpsClusterMultgR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsClusterMultgR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsClusterMultgR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsClusterMultgR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsClusterMultgR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsClusterMultgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsClusterMult>*)
   {
      vector<VTPData::hpsClusterMult> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsClusterMult>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsClusterMult>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsClusterMult>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsClusterMultgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsClusterMult>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsClusterMultgR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsClusterMultgR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsClusterMultgR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsClusterMultgR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsClusterMultgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsClusterMult> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsClusterMult>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsClusterMultgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsClusterMult>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsClusterMultgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsClusterMultgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsClusterMultgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsClusterMult> : new vector<VTPData::hpsClusterMult>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsClusterMultgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsClusterMult>[nElements] : new vector<VTPData::hpsClusterMult>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsClusterMultgR(void *p) {
      delete ((vector<VTPData::hpsClusterMult>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsClusterMultgR(void *p) {
      delete [] ((vector<VTPData::hpsClusterMult>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsClusterMultgR(void *p) {
      typedef vector<VTPData::hpsClusterMult> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsClusterMult>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsClustergR_Dictionary();
   static void vectorlEVTPDatacLcLhpsClustergR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsClustergR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsClustergR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsClustergR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsClustergR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsClustergR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsCluster>*)
   {
      vector<VTPData::hpsCluster> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsCluster>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsCluster>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsCluster>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsClustergR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsCluster>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsClustergR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsClustergR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsClustergR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsClustergR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsClustergR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsCluster> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsCluster>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsClustergR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsCluster>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsClustergR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsClustergR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsClustergR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsCluster> : new vector<VTPData::hpsCluster>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsClustergR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsCluster>[nElements] : new vector<VTPData::hpsCluster>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsClustergR(void *p) {
      delete ((vector<VTPData::hpsCluster>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsClustergR(void *p) {
      delete [] ((vector<VTPData::hpsCluster>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsClustergR(void *p) {
      typedef vector<VTPData::hpsCluster> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsCluster>

namespace ROOT {
   static TClass *vectorlEVTPDatacLcLhpsCalibTriggR_Dictionary();
   static void vectorlEVTPDatacLcLhpsCalibTriggR_TClassManip(TClass*);
   static void *new_vectorlEVTPDatacLcLhpsCalibTriggR(void *p = 0);
   static void *newArray_vectorlEVTPDatacLcLhpsCalibTriggR(Long_t size, void *p);
   static void delete_vectorlEVTPDatacLcLhpsCalibTriggR(void *p);
   static void deleteArray_vectorlEVTPDatacLcLhpsCalibTriggR(void *p);
   static void destruct_vectorlEVTPDatacLcLhpsCalibTriggR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<VTPData::hpsCalibTrig>*)
   {
      vector<VTPData::hpsCalibTrig> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<VTPData::hpsCalibTrig>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<VTPData::hpsCalibTrig>", -2, "vector", 339,
                  typeid(vector<VTPData::hpsCalibTrig>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEVTPDatacLcLhpsCalibTriggR_Dictionary, isa_proxy, 4,
                  sizeof(vector<VTPData::hpsCalibTrig>) );
      instance.SetNew(&new_vectorlEVTPDatacLcLhpsCalibTriggR);
      instance.SetNewArray(&newArray_vectorlEVTPDatacLcLhpsCalibTriggR);
      instance.SetDelete(&delete_vectorlEVTPDatacLcLhpsCalibTriggR);
      instance.SetDeleteArray(&deleteArray_vectorlEVTPDatacLcLhpsCalibTriggR);
      instance.SetDestructor(&destruct_vectorlEVTPDatacLcLhpsCalibTriggR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<VTPData::hpsCalibTrig> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<VTPData::hpsCalibTrig>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEVTPDatacLcLhpsCalibTriggR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<VTPData::hpsCalibTrig>*)0x0)->GetClass();
      vectorlEVTPDatacLcLhpsCalibTriggR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEVTPDatacLcLhpsCalibTriggR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEVTPDatacLcLhpsCalibTriggR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsCalibTrig> : new vector<VTPData::hpsCalibTrig>;
   }
   static void *newArray_vectorlEVTPDatacLcLhpsCalibTriggR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<VTPData::hpsCalibTrig>[nElements] : new vector<VTPData::hpsCalibTrig>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEVTPDatacLcLhpsCalibTriggR(void *p) {
      delete ((vector<VTPData::hpsCalibTrig>*)p);
   }
   static void deleteArray_vectorlEVTPDatacLcLhpsCalibTriggR(void *p) {
      delete [] ((vector<VTPData::hpsCalibTrig>*)p);
   }
   static void destruct_vectorlEVTPDatacLcLhpsCalibTriggR(void *p) {
      typedef vector<VTPData::hpsCalibTrig> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<VTPData::hpsCalibTrig>

namespace ROOT {
   static TClass *vectorlETrackerHitgR_Dictionary();
   static void vectorlETrackerHitgR_TClassManip(TClass*);
   static void *new_vectorlETrackerHitgR(void *p = 0);
   static void *newArray_vectorlETrackerHitgR(Long_t size, void *p);
   static void delete_vectorlETrackerHitgR(void *p);
   static void deleteArray_vectorlETrackerHitgR(void *p);
   static void destruct_vectorlETrackerHitgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TrackerHit>*)
   {
      vector<TrackerHit> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TrackerHit>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TrackerHit>", -2, "vector", 339,
                  typeid(vector<TrackerHit>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETrackerHitgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TrackerHit>) );
      instance.SetNew(&new_vectorlETrackerHitgR);
      instance.SetNewArray(&newArray_vectorlETrackerHitgR);
      instance.SetDelete(&delete_vectorlETrackerHitgR);
      instance.SetDeleteArray(&deleteArray_vectorlETrackerHitgR);
      instance.SetDestructor(&destruct_vectorlETrackerHitgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TrackerHit> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TrackerHit>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETrackerHitgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TrackerHit>*)0x0)->GetClass();
      vectorlETrackerHitgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETrackerHitgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETrackerHitgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TrackerHit> : new vector<TrackerHit>;
   }
   static void *newArray_vectorlETrackerHitgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TrackerHit>[nElements] : new vector<TrackerHit>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETrackerHitgR(void *p) {
      delete ((vector<TrackerHit>*)p);
   }
   static void deleteArray_vectorlETrackerHitgR(void *p) {
      delete [] ((vector<TrackerHit>*)p);
   }
   static void destruct_vectorlETrackerHitgR(void *p) {
      typedef vector<TrackerHit> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TrackerHit>

namespace ROOT {
   static TClass *vectorlETrackerHitmUgR_Dictionary();
   static void vectorlETrackerHitmUgR_TClassManip(TClass*);
   static void *new_vectorlETrackerHitmUgR(void *p = 0);
   static void *newArray_vectorlETrackerHitmUgR(Long_t size, void *p);
   static void delete_vectorlETrackerHitmUgR(void *p);
   static void deleteArray_vectorlETrackerHitmUgR(void *p);
   static void destruct_vectorlETrackerHitmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TrackerHit*>*)
   {
      vector<TrackerHit*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TrackerHit*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TrackerHit*>", -2, "vector", 339,
                  typeid(vector<TrackerHit*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETrackerHitmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TrackerHit*>) );
      instance.SetNew(&new_vectorlETrackerHitmUgR);
      instance.SetNewArray(&newArray_vectorlETrackerHitmUgR);
      instance.SetDelete(&delete_vectorlETrackerHitmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETrackerHitmUgR);
      instance.SetDestructor(&destruct_vectorlETrackerHitmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TrackerHit*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TrackerHit*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETrackerHitmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TrackerHit*>*)0x0)->GetClass();
      vectorlETrackerHitmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETrackerHitmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETrackerHitmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TrackerHit*> : new vector<TrackerHit*>;
   }
   static void *newArray_vectorlETrackerHitmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TrackerHit*>[nElements] : new vector<TrackerHit*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETrackerHitmUgR(void *p) {
      delete ((vector<TrackerHit*>*)p);
   }
   static void deleteArray_vectorlETrackerHitmUgR(void *p) {
      delete [] ((vector<TrackerHit*>*)p);
   }
   static void destruct_vectorlETrackerHitmUgR(void *p) {
      typedef vector<TrackerHit*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TrackerHit*>

namespace ROOT {
   static TClass *vectorlETrackgR_Dictionary();
   static void vectorlETrackgR_TClassManip(TClass*);
   static void *new_vectorlETrackgR(void *p = 0);
   static void *newArray_vectorlETrackgR(Long_t size, void *p);
   static void delete_vectorlETrackgR(void *p);
   static void deleteArray_vectorlETrackgR(void *p);
   static void destruct_vectorlETrackgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<Track>*)
   {
      vector<Track> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<Track>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<Track>", -2, "vector", 339,
                  typeid(vector<Track>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETrackgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<Track>) );
      instance.SetNew(&new_vectorlETrackgR);
      instance.SetNewArray(&newArray_vectorlETrackgR);
      instance.SetDelete(&delete_vectorlETrackgR);
      instance.SetDeleteArray(&deleteArray_vectorlETrackgR);
      instance.SetDestructor(&destruct_vectorlETrackgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<Track> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<Track>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETrackgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<Track>*)0x0)->GetClass();
      vectorlETrackgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETrackgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETrackgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Track> : new vector<Track>;
   }
   static void *newArray_vectorlETrackgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Track>[nElements] : new vector<Track>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETrackgR(void *p) {
      delete ((vector<Track>*)p);
   }
   static void deleteArray_vectorlETrackgR(void *p) {
      delete [] ((vector<Track>*)p);
   }
   static void destruct_vectorlETrackgR(void *p) {
      typedef vector<Track> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<Track>

namespace ROOT {
   static TClass *vectorlETrackmUgR_Dictionary();
   static void vectorlETrackmUgR_TClassManip(TClass*);
   static void *new_vectorlETrackmUgR(void *p = 0);
   static void *newArray_vectorlETrackmUgR(Long_t size, void *p);
   static void delete_vectorlETrackmUgR(void *p);
   static void deleteArray_vectorlETrackmUgR(void *p);
   static void destruct_vectorlETrackmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<Track*>*)
   {
      vector<Track*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<Track*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<Track*>", -2, "vector", 339,
                  typeid(vector<Track*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETrackmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<Track*>) );
      instance.SetNew(&new_vectorlETrackmUgR);
      instance.SetNewArray(&newArray_vectorlETrackmUgR);
      instance.SetDelete(&delete_vectorlETrackmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETrackmUgR);
      instance.SetDestructor(&destruct_vectorlETrackmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<Track*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<Track*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETrackmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<Track*>*)0x0)->GetClass();
      vectorlETrackmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETrackmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETrackmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Track*> : new vector<Track*>;
   }
   static void *newArray_vectorlETrackmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<Track*>[nElements] : new vector<Track*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETrackmUgR(void *p) {
      delete ((vector<Track*>*)p);
   }
   static void deleteArray_vectorlETrackmUgR(void *p) {
      delete [] ((vector<Track*>*)p);
   }
   static void destruct_vectorlETrackmUgR(void *p) {
      typedef vector<Track*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<Track*>

namespace ROOT {
   static TClass *vectorlETSDatacLcLtsBitsgR_Dictionary();
   static void vectorlETSDatacLcLtsBitsgR_TClassManip(TClass*);
   static void *new_vectorlETSDatacLcLtsBitsgR(void *p = 0);
   static void *newArray_vectorlETSDatacLcLtsBitsgR(Long_t size, void *p);
   static void delete_vectorlETSDatacLcLtsBitsgR(void *p);
   static void deleteArray_vectorlETSDatacLcLtsBitsgR(void *p);
   static void destruct_vectorlETSDatacLcLtsBitsgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TSData::tsBits>*)
   {
      vector<TSData::tsBits> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TSData::tsBits>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TSData::tsBits>", -2, "vector", 339,
                  typeid(vector<TSData::tsBits>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETSDatacLcLtsBitsgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TSData::tsBits>) );
      instance.SetNew(&new_vectorlETSDatacLcLtsBitsgR);
      instance.SetNewArray(&newArray_vectorlETSDatacLcLtsBitsgR);
      instance.SetDelete(&delete_vectorlETSDatacLcLtsBitsgR);
      instance.SetDeleteArray(&deleteArray_vectorlETSDatacLcLtsBitsgR);
      instance.SetDestructor(&destruct_vectorlETSDatacLcLtsBitsgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TSData::tsBits> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TSData::tsBits>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETSDatacLcLtsBitsgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TSData::tsBits>*)0x0)->GetClass();
      vectorlETSDatacLcLtsBitsgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETSDatacLcLtsBitsgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETSDatacLcLtsBitsgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TSData::tsBits> : new vector<TSData::tsBits>;
   }
   static void *newArray_vectorlETSDatacLcLtsBitsgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TSData::tsBits>[nElements] : new vector<TSData::tsBits>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETSDatacLcLtsBitsgR(void *p) {
      delete ((vector<TSData::tsBits>*)p);
   }
   static void deleteArray_vectorlETSDatacLcLtsBitsgR(void *p) {
      delete [] ((vector<TSData::tsBits>*)p);
   }
   static void destruct_vectorlETSDatacLcLtsBitsgR(void *p) {
      typedef vector<TSData::tsBits> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TSData::tsBits>

namespace ROOT {
   static TClass *vectorlETObjectgR_Dictionary();
   static void vectorlETObjectgR_TClassManip(TClass*);
   static void *new_vectorlETObjectgR(void *p = 0);
   static void *newArray_vectorlETObjectgR(Long_t size, void *p);
   static void delete_vectorlETObjectgR(void *p);
   static void deleteArray_vectorlETObjectgR(void *p);
   static void destruct_vectorlETObjectgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TObject>*)
   {
      vector<TObject> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TObject>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TObject>", -2, "vector", 339,
                  typeid(vector<TObject>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETObjectgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TObject>) );
      instance.SetNew(&new_vectorlETObjectgR);
      instance.SetNewArray(&newArray_vectorlETObjectgR);
      instance.SetDelete(&delete_vectorlETObjectgR);
      instance.SetDeleteArray(&deleteArray_vectorlETObjectgR);
      instance.SetDestructor(&destruct_vectorlETObjectgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TObject> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TObject>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETObjectgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TObject>*)0x0)->GetClass();
      vectorlETObjectgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETObjectgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETObjectgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TObject> : new vector<TObject>;
   }
   static void *newArray_vectorlETObjectgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TObject>[nElements] : new vector<TObject>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETObjectgR(void *p) {
      delete ((vector<TObject>*)p);
   }
   static void deleteArray_vectorlETObjectgR(void *p) {
      delete [] ((vector<TObject>*)p);
   }
   static void destruct_vectorlETObjectgR(void *p) {
      typedef vector<TObject> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TObject>

namespace ROOT {
   static TClass *vectorlETObjectmUgR_Dictionary();
   static void vectorlETObjectmUgR_TClassManip(TClass*);
   static void *new_vectorlETObjectmUgR(void *p = 0);
   static void *newArray_vectorlETObjectmUgR(Long_t size, void *p);
   static void delete_vectorlETObjectmUgR(void *p);
   static void deleteArray_vectorlETObjectmUgR(void *p);
   static void destruct_vectorlETObjectmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TObject*>*)
   {
      vector<TObject*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TObject*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TObject*>", -2, "vector", 339,
                  typeid(vector<TObject*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETObjectmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TObject*>) );
      instance.SetNew(&new_vectorlETObjectmUgR);
      instance.SetNewArray(&newArray_vectorlETObjectmUgR);
      instance.SetDelete(&delete_vectorlETObjectmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETObjectmUgR);
      instance.SetDestructor(&destruct_vectorlETObjectmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TObject*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<TObject*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETObjectmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TObject*>*)0x0)->GetClass();
      vectorlETObjectmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETObjectmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETObjectmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TObject*> : new vector<TObject*>;
   }
   static void *newArray_vectorlETObjectmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TObject*>[nElements] : new vector<TObject*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETObjectmUgR(void *p) {
      delete ((vector<TObject*>*)p);
   }
   static void deleteArray_vectorlETObjectmUgR(void *p) {
      delete [] ((vector<TObject*>*)p);
   }
   static void destruct_vectorlETObjectmUgR(void *p) {
      typedef vector<TObject*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TObject*>

namespace ROOT {
   static TClass *vectorlERawSvtHitgR_Dictionary();
   static void vectorlERawSvtHitgR_TClassManip(TClass*);
   static void *new_vectorlERawSvtHitgR(void *p = 0);
   static void *newArray_vectorlERawSvtHitgR(Long_t size, void *p);
   static void delete_vectorlERawSvtHitgR(void *p);
   static void deleteArray_vectorlERawSvtHitgR(void *p);
   static void destruct_vectorlERawSvtHitgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RawSvtHit>*)
   {
      vector<RawSvtHit> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RawSvtHit>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RawSvtHit>", -2, "vector", 339,
                  typeid(vector<RawSvtHit>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERawSvtHitgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RawSvtHit>) );
      instance.SetNew(&new_vectorlERawSvtHitgR);
      instance.SetNewArray(&newArray_vectorlERawSvtHitgR);
      instance.SetDelete(&delete_vectorlERawSvtHitgR);
      instance.SetDeleteArray(&deleteArray_vectorlERawSvtHitgR);
      instance.SetDestructor(&destruct_vectorlERawSvtHitgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RawSvtHit> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<RawSvtHit>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERawSvtHitgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RawSvtHit>*)0x0)->GetClass();
      vectorlERawSvtHitgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERawSvtHitgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERawSvtHitgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RawSvtHit> : new vector<RawSvtHit>;
   }
   static void *newArray_vectorlERawSvtHitgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RawSvtHit>[nElements] : new vector<RawSvtHit>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERawSvtHitgR(void *p) {
      delete ((vector<RawSvtHit>*)p);
   }
   static void deleteArray_vectorlERawSvtHitgR(void *p) {
      delete [] ((vector<RawSvtHit>*)p);
   }
   static void destruct_vectorlERawSvtHitgR(void *p) {
      typedef vector<RawSvtHit> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RawSvtHit>

namespace ROOT {
   static TClass *vectorlERawSvtHitmUgR_Dictionary();
   static void vectorlERawSvtHitmUgR_TClassManip(TClass*);
   static void *new_vectorlERawSvtHitmUgR(void *p = 0);
   static void *newArray_vectorlERawSvtHitmUgR(Long_t size, void *p);
   static void delete_vectorlERawSvtHitmUgR(void *p);
   static void deleteArray_vectorlERawSvtHitmUgR(void *p);
   static void destruct_vectorlERawSvtHitmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RawSvtHit*>*)
   {
      vector<RawSvtHit*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RawSvtHit*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RawSvtHit*>", -2, "vector", 339,
                  typeid(vector<RawSvtHit*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERawSvtHitmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RawSvtHit*>) );
      instance.SetNew(&new_vectorlERawSvtHitmUgR);
      instance.SetNewArray(&newArray_vectorlERawSvtHitmUgR);
      instance.SetDelete(&delete_vectorlERawSvtHitmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlERawSvtHitmUgR);
      instance.SetDestructor(&destruct_vectorlERawSvtHitmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RawSvtHit*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<RawSvtHit*>*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERawSvtHitmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RawSvtHit*>*)0x0)->GetClass();
      vectorlERawSvtHitmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERawSvtHitmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERawSvtHitmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RawSvtHit*> : new vector<RawSvtHit*>;
   }
   static void *newArray_vectorlERawSvtHitmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RawSvtHit*>[nElements] : new vector<RawSvtHit*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERawSvtHitmUgR(void *p) {
      delete ((vector<RawSvtHit*>*)p);
   }
   static void deleteArray_vectorlERawSvtHitmUgR(void *p) {
      delete [] ((vector<RawSvtHit*>*)p);
   }
   static void destruct_vectorlERawSvtHitmUgR(void *p) {
      typedef vector<RawSvtHit*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RawSvtHit*>

namespace {
  void TriggerDictionaryInitialization_libevent_Impl() {
    static const char* headers[] = {
"/u/re/alspellm/work/src/hpstr/event/include/EventDef.h",
0
    };
    static const char* includePaths[] = {
"/u/re/alspellm/work/src/hpstr/event/include",
"/nfs/slac/g/hps3/users/bravo/src/root/buildV61204/include",
"/nfs/slac/g/hps3/users/bravo/src/root/buildV61204/externals/usr/local/include",
"/nfs/slac/g/hps3/users/alspellm/src/LCIO/install/include",
"/nfs/slac/g/hps3/users/bravo/src/root/buildV61204/include",
"/nfs/slac/g/hps3/users/alspellm/src/hpstr/build/event/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libevent dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate("$clingAutoload$Vertex.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  Vertex;
namespace std{template <typename _Tp> class __attribute__((annotate("$clingAutoload$bits/allocator.h")))  __attribute__((annotate("$clingAutoload$string")))  allocator;
}
class __attribute__((annotate("$clingAutoload$Track.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  Track;
class __attribute__((annotate("$clingAutoload$TrackerHit.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  TrackerHit;
class __attribute__((annotate("$clingAutoload$RawSvtHit.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  RawSvtHit;
class __attribute__((annotate("$clingAutoload$TObject.h")))  TObject;
class __attribute__((annotate("$clingAutoload$CalCluster.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  CalCluster;
class __attribute__((annotate("$clingAutoload$CalHit.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  CalHit;
class __attribute__((annotate("$clingAutoload$EventHeader.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  EventHeader;
class __attribute__((annotate("$clingAutoload$VTPData.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  VTPData;
class __attribute__((annotate("$clingAutoload$TriggerData.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  TriggerData;
class __attribute__((annotate("$clingAutoload$TSData.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  TSData;
class __attribute__((annotate("$clingAutoload$Particle.h")))  __attribute__((annotate("$clingAutoload$/u/re/alspellm/work/src/hpstr/event/include/EventDef.h")))  Particle;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libevent dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "/u/re/alspellm/work/src/hpstr/event/include/EventDef.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"CalCluster", payloadCode, "@",
"CalHit", payloadCode, "@",
"EventHeader", payloadCode, "@",
"Particle", payloadCode, "@",
"RawSvtHit", payloadCode, "@",
"TSData", payloadCode, "@",
"TSData::tsBits", payloadCode, "@",
"TSData::tsHeader", payloadCode, "@",
"Track", payloadCode, "@",
"TrackerHit", payloadCode, "@",
"TriggerData", payloadCode, "@",
"VTPData", payloadCode, "@",
"VTPData::bHeader", payloadCode, "@",
"VTPData::bTail", payloadCode, "@",
"VTPData::eHeader", payloadCode, "@",
"VTPData::hpsCalibTrig", payloadCode, "@",
"VTPData::hpsCluster", payloadCode, "@",
"VTPData::hpsClusterMult", payloadCode, "@",
"VTPData::hpsFEETrig", payloadCode, "@",
"VTPData::hpsPairTrig", payloadCode, "@",
"VTPData::hpsSingleTrig", payloadCode, "@",
"Vertex", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libevent",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libevent_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libevent_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libevent() {
  TriggerDictionaryInitialization_libevent_Impl();
}
