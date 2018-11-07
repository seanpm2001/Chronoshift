/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef FACTORY_H
#define FACTORY_H

#include "always.h"
#include "gameptr.h"
#include "rtti.h"
#include "stage.h"

class HouseClass;
class TechnoClass;
class TechnoTypeClass;
class NoInitClass;

class FactoryClass
{
public:
    FactoryClass();
    ~FactoryClass();
    FactoryClass(const FactoryClass &that);
    FactoryClass(const NoInitClass &noinit) {}

    void *operator new(size_t size);
    void *operator new(size_t size, void *ptr) { return ptr; }
    void operator delete(void *ptr);
#ifndef COMPILER_WATCOM // Watcom doesn't like this, MSVC/GCC does.
    void operator delete(void *ptr, void *place) {}
#endif

    static void Init();

    void AI();
    BOOL Has_Changed();
    BOOL Set(TechnoTypeClass &objecttype, HouseClass &house);
    BOOL Set(int &special, HouseClass &house);
    void Set(TechnoClass &object);
    BOOL Suspend();
    BOOL Start();
    BOOL Abandon();
    int Completion() const { return ProductionTime.Get_Stage(); }
    BOOL Has_Completed() const;
    BOOL Is_Building() const { return ProductionTime.Get_Delay() != 0; }
    TechnoClass *const Get_Object() const { return Object; }
    int Get_Special_Item() const { return SpecialItem; }
    int Cost_Per_Tick() const;
    BOOL Completed();

    void Code_Pointers();
    void Decode_Pointers();

private:
    StageClass ProductionTime;
    RTTIType RTTI;
    int ID;
#ifndef CHRONOSHIFT_NO_BITFIELDS
    // Union/Struct required to get correct packing when compiler packing set to 1.
    union
    {
        struct
        {
            bool IsActive : 1; // 1
            bool IsSuspended : 1; // 2
            bool IsDifferent : 1; // 4
        };
        int m_Bitfield;
    };
#else
    bool IsActive;
    bool IsSuspended;
    bool IsDifferent;
#endif
    int Balance;
    int OriginalBalance;
    TechnoClass *Object; // The techno object we are to produce.
    int SpecialItem;
    HouseClass *Owner; // The house that owns the object.

#ifndef CHRONOSHIFT_STANDALONE
#include "hooker.h"
public:
    static inline void FactoryClass::Hook_Me()
    {
    #ifdef COMPILER_WATCOM
        Hook_Function(0x004BEE10, *FactoryClass::Has_Changed);
        Hook_Function(0x004BEF80, *FactoryClass::Suspend);
        Hook_Function(0x004BF330, *FactoryClass::Hooked_Has_Completed);
        Hook_Function(0x004BF380, *FactoryClass::Hooked_Cost_Per_Tick);
        Hook_Function(0x004BF3B4, *FactoryClass::Completed);
    #endif
    }

    BOOL Hooked_Has_Completed() { return FactoryClass::Has_Completed(); }
    int Hooked_Cost_Per_Tick() { return FactoryClass::Cost_Per_Tick(); }
#endif
};

#ifndef CHRONOSHIFT_STANDALONE
#include "hooker.h"
extern TFixedIHeapClass<FactoryClass> &g_Factories;
#else
extern TFixedIHeapClass<FactoryClass> g_Factories;
#endif

#endif // FACTORY_H