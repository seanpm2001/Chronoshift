/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 *
 * @brief Low level sprite drawing functions for Dune2 sprite format (used for mouse in early C&C games).
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef MOUSESHAPE_H
#define MOUSESHAPE_H

#include "always.h"

#pragma pack(push, 1)
// Header for each frame in the old SHP format... only used for mouse now.
struct MouseShapeFrameHeader
{
    uint16_t Flags;
    uint8_t Lines;
    uint16_t Width;
    uint8_t Height; // always same as lines?
    uint16_t CompressedSize;
    uint16_t UncompressedSize;
};

// Header for the old SHP format... only used for mouse now. Offset values need +2 to get actual frame pos.
struct MouseShapeHeader
{
    uint16_t FrameCount;
    uint32_t Offset;
};
#pragma pack(pop)

class WWMouseClass;
class GraphicViewPortClass;

int Get_Shape_Uncomp_Size(void *shape);
int Extract_Shape_Count(void *shape);
void *Extract_Shape(void *shape, int frame = 0);
int Get_Shape_Width(void *shape);
int Get_Shape_Height(void *shape);
int Restore_Shape_Height(void *shape);
int Get_Shape_Original_Height(void *shape);

void __cdecl Mouse_Shadow_Buffer(WWMouseClass &mouse, GraphicViewPortClass &viewport, void *buffer, int x_pos, int y_pos, int hspot_x, int hspot_y, BOOL save);
void __cdecl Mouse_Draw(WWMouseClass &mouse, GraphicViewPortClass &viewport, int x_pos, int y_pos);
void *__cdecl Mouse_Set_Cursor(WWMouseClass &mouse, int hspot_x, int hspot_y, void *shape);

#ifndef RAPP_STANDALONE
#include "hooker.h"

namespace MouseShape {

inline void Hook_Me()
{
    Hook_Function(0x005D8F5C, Mouse_Shadow_Buffer);
    Hook_Function(0x005D90A1, Mouse_Draw);
    Hook_Function(0x005D91CF, Mouse_Set_Cursor);
}

}
#endif

#endif