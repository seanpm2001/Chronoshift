////////////////////////////////////////////////////////////////////////////////
//                            --  REDALERT++ --                               //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Redalert++
//
//          File:: SHASTRAW.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: Stream implementation that hashes data with SHA1 as it
//                 passes through.
//
//       License:: Redalert++ is free software: you can redistribute it and/or
//                 modify it under the terms of the GNU General Public License
//                 as published by the Free Software Foundation, either version
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef SHASTRAW_H
#define SHASTRAW_H

#include "always.h"
#include "sha.h"
#include "straw.h"

class SHAStraw : public Straw
{
public:
    SHAStraw() : m_sha1() {}
    virtual ~SHAStraw() {}

    virtual int Get(void *buffer, int length) override;

    int const Result(void *data) { return m_sha1.Result(data); }

protected:
    SHAEngine m_sha1; // A instance of the SHA-1 Engine class.

};

#endif // SHASTRAW_H
