/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Part of IOMap stack handling map loading and logic.
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "map.h"
#include "abs.h"
#include "coord.h"
#include "globals.h"
#include "minmax.h"
#include "rules.h"
#include "scenario.h"
#include "session.h"
#include "target.h"

// TODO this is require for some none virtual calls to functions higher in the stack.
#include "iomap.h"

//This appears to be missing the entries for 309+ which TS has for sight 11
const int MapClass::RadiusOffset[] = {
    0, -129, -128, -127, -1, 1, 127, 128,
    129, -257, -256, -255, -130, -126, -2, 2,
    126, 130, 255, 256, 257, -385, -384, -383,
    -258, -254, -131, -125, -3, 3, 125, 131,
    254, 258, 383, 384, 385, -513, -512, -511,
    -387, -386, -382, -381, -259, -253, -132, -124,
    -4, 4, 124, 132, 253, 259, 381, 382,
    386, 387, 511, 512, 513, -641, -640, -639,
    -515, -514, -510, -509, -388, -380, -260, -252,
    -133, -123, -5, 5, 123, 133, 252, 260,
    380, 388, 509, 510, 514, 515, 639, 640,
    641, -769, -768, -767, -643, -642, -638, -637,
    -516, -508, -389, -379, -261, -251, -134, -122,
    -6, 6, 122, 134, 251, 261, 379, 389,
    508, 516, 637, 638, 642, 643, 767, 768,
    769, -897, -896, -895, -771, -770, -766, -765,
    -645, -644, -636, -635, -517, -507, -390, -378,
    -262, -250, -135, -121, -7, 7, 121, 135,
    250, 262, 378, 390, 507, 517, 635, 636,
    644, 645, 765, 766, 770, 771, 895, 896,
    897, -1025, -1024, -1023, -899, -898, -894, -893,
    -773, -772, -764, -763, -646, -634, -518, -506,
    -391, -377, -263, -249, -136, -120, -8, 8,
    120, 136, 249, 263, 377, 391, 506, 518,
    634, 646, 763, 764, 772, 773, 893, 894,
    898, 899, 1023, 1024, 1025, -1153, -1152, -1151,
    -1027, -1026, -1022, -1021, -901, -900, -892, -891,
    -774, -762, -647, -633, -519, -505, -392, -376,
    -264, -248, -137, -119, -9, 9, 119, 137,
    248, 264, 376, 392, 505, 519, 633, 647,
    762, 774, 891, 892, 900, 901, 1021, 1022,
    1026, 1027, 1151, 1152, 1153, -1281, -1280, -1279,
    -1155, -1154, -1150, -1149, -1029, -1028, -1020, -1019,
    -903, -902, -890, -889, -775, -761, -648, -632,
    -520, -504, -393, -375, -265, -247, -138, -118,
    -10, 10, 118, 138, 247, 265, 375, 393,
    504, 520, 632, 648, 761, 775, 889, 890,
    902, 903, 1019, 1020, 1028, 1029, 1149, 1150,
    1154, 1155, 1279, 1280, 1281
};

// This relates to sight, sight > 11 == overflow == bad.
// give this a read, it shows how it works with visual help http://modenc.renegadeprojects.com/CellSpread
const int MapClass::RadiusCount[] = {
    1,
    9,
    21,
    37,
    61,
    89,
    121,
    161,
    205,
    253,
    309
};

MapClass::MapClass() :
    MapCellX(0),
    MapCellY(0),
    MapCellWidth(0),
    MapCellHeight(0),
    TotalValue(0),
    Array(),
    XSize(0),
    YSize(0),
    TotalSize(0)
{
}

void MapClass::One_Time()
{
    GameScreenClass::One_Time();
    XSize = MAP_MAX_WIDTH;
    YSize = MAP_MAX_HEIGHT;
    TotalSize = (YSize * XSize);
    Alloc_Cells();

    for (int i = 0; i < MAP_MAX_CRATES; ++i) {
        new(&Crates[i]) CrateClass;
    }
}

void MapClass::Init_Clear()
{
    GameScreenClass::Init_Clear();
    Init_Cells();
    OreLogicPos = 0;
    OreGrowthCount = 0;
    OreGrowthExcess = 0;
    OreSpreadCount = 0;
    OreSpreadExcess = 0;
}

void MapClass::Alloc_Cells()
{
    // Clear all cells in the vector array, reconstruct the vector and then resize to total cell count.
    Array.Clear();
    new(&Array) VectorClass<CellClass>;
    Array.Resize(TotalSize);
}

void MapClass::Free_Cells()
{
    // Clear all cells in the vector.
    Array.Clear();
}

void MapClass::Init_Cells()
{
    // Reset the total cell count
    TotalValue = 0;

    // Loop through the whole vector and call the the constructor for each cell with placement new.
    for (int cellnum = 0; cellnum < MAP_MAX_AREA; ++cellnum) {
        new(&Array[cellnum]) CellClass;
    }
}

void MapClass::Logic_AI()
{
    if (Session.Game_To_Play() != GAME_CAMPAIGN && Session.MPlayer_Goodies_Allowed()) {
        for (int i = 0; i < ARRAY_SIZE(Crates); ++i) {
            if (Crates[i].Get_Cell() != -1) {
                if (Crates[i].Timer_Expired()) {
                    Crates[i].Remove_It();
                    Place_Random_Crate();
                }
            }
        }
    }

    if (Rule.Ore_Grows() || Rule.Ore_Spreads()) {
        // Limit how much of the map we try to mark at any one time to limit how often the growth logic runs.
        int growth_limit = Max(MAP_MAX_AREA / (Rule.Ore_Growth_Rate() * GAME_TICKS_PER_MINUTE), 1);
        int index;

        // Build lists of cells that need growth and spreading applied to them.
        for (index = OreLogicPos; index < MAP_MAX_AREA; ++index) {
            if (In_Radar(index)) {
                if (Array[index].Can_Ore_Grow()) {
                    if (Scen.Get_Random_Value(0, OreGrowthExcess) <= OreGrowthCount) {
                        if (OreGrowthCount >= ARRAY_SIZE(OreGrowth)) {
                            OreGrowth[Scen.Get_Random_Value(0, OreGrowthCount - 1)] = index;
                        } else {
                            OreGrowth[OreGrowthCount++] = index;
                        }
                    }

                    ++OreGrowthExcess;
                }

                if (Array[index].Can_Ore_Spread()) {
                    if (Scen.Get_Random_Value(0, OreSpreadExcess) <= OreSpreadCount) {
                        if (OreSpreadCount >= ARRAY_SIZE(OreSpread)) {
                            OreSpread[Scen.Get_Random_Value(0, OreSpreadCount - 1)] = index;
                        } else {
                            OreSpread[OreSpreadCount++] = index;
                        }
                    }

                    ++OreSpreadExcess;
                }
            }

            if (--growth_limit == 0) {
                break;
            }
        }

        OreLogicPos = index;

        // If we've finished marking the whole map, reset our logic position and action what we marked.
        if (OreLogicPos >= MAP_MAX_AREA) {
            OreLogicPos = 0;

            for (int index = 0; index < OreGrowthCount; ++index) {
                Array[OreGrowth[index]].Grow_Ore();
            }

            OreGrowthCount = 0;
            OreGrowthExcess = 0;

            for (int index = 0; index < OreSpreadCount; ++index) {
                Array[OreSpread[index]].Spread_Ore(false);
            }

            OreSpreadCount = 0;
            OreSpreadExcess = 0;
        }
    }
}

void MapClass::Set_Map_Dimensions(int x, int y, int w, int h)
{
    MapCellX = x;
    MapCellY = y;
    MapCellWidth = w;
    MapCellHeight = h;
}

BOOL MapClass::Place_Random_Crate()
{
    for ( int i = 0; i < ARRAY_SIZE(Crates); ++i ) {
        if (Crates[i].Get_Cell() == -1) {
            // Try to place crate up to 1000 times, return result of final attempt if all others fail.
            for (int i = 0; i < 1000; ++i) {
                if (Crates[i].Create_Crate(Pick_Random_Location())) {
                    return true;
                }
            }

            return Crates[i].Create_Crate(Pick_Random_Location());
        }
    }

    return false;
}

int16_t MapClass::Pick_Random_Location() const
{
    return Cell_From_XY(MapCellX + Scen.Get_Random_Value(0, MapCellWidth - 1), MapCellY + Scen.Get_Random_Value(0, MapCellHeight - 1));
}

BOOL MapClass::In_Radar(int16_t cellnum) const
{
    if (cellnum <= MAP_MAX_AREA) {

        unsigned cell_x = Cell_Get_X(cellnum);
        unsigned cell_y = Cell_Get_Y(cellnum);

        // Treat as unsigned to check value is within both bounds for only one check by using unsigned underflow behaviour.
        if ((cell_x - (unsigned)MapCellX) < (unsigned)MapCellWidth && (cell_y - (unsigned)MapCellY) < (unsigned)MapCellHeight) {
            return true;
        }
    }

    return false;
}

void MapClass::Sight_From(int16_t cellnum, int radius, HouseClass *house, BOOL a4)
{
    // TODO requires RadarClass
#ifndef RAPP_STANDALONE
    void(*func)(const MapClass*, int16_t, int, HouseClass *, BOOL) = reinterpret_cast<void(*)(const MapClass*, int16_t, int, HouseClass *, BOOL)>(0x004FE438);
    func(this, cellnum, radius, house, a4);
#elif 0
    DEBUG_ASSERT(cellnum < MAP_MAX_AREA);
    DEBUG_ASSERT(radius < ARRAY_SIZE(RadiusCount));
    DEBUG_ASSERT(house != nullptr);

    // NOTE: In C&C/Sole, this doesn't take a house and instead always passes PlayerPtr to Map_Cell().
    // Sole adjusts radius to be 10 if its greater than 10. Possible fix to avoid function treating radius > 10 as 0?
    // "!!!going past ten is a hard code Vegas crash!!!"

    if (In_Radar(cellnum)) {
        if (radius >= 0 && radius < ARRAY_SIZE(RadiusCount)) {
            const int *offset_ptr = RadiusOffset;
            int radius_count = RadiusCount[radius];

            // In Sole/C&C the check is for radius > 1 and the adjustment is - 1. Not sure what significance that has, but a4 appears the be a bool that flags if this check and adjustment is made.
            if (a4 && radius > 2) {
                radius_count -= RadiusCount[radius];
                offset_ptr = &RadiusOffset[RadiusCount[radius]];
            }

            CellClass *cell = &Array[cellnum];
            DEBUG_ASSERT(cell != nullptr);

            while (--radius_count != -1) {
                int16_t offset_cellnum = *offset_ptr + cellnum;
                ++offset_ptr;

                // If the cell we are considering is within the map and the distance in the X axis is less than radius, calc actual distance.
                if (offset_cellnum < MAP_MAX_AREA) {
                    if (Abs(Cell_Get_X(offset_cellnum) - Cell_Get_X(cellnum)) <= radius) {
                        // In SS/C&C, distance uses raw cell numbers into the int16_t int version of distance and checks <= radius, not radius * 256.
                        if (Distance(Cell_To_Coord(offset_cellnum), Cell_To_Coord(cellnum)) <= (radius * 256)) {
                            if (!cell->Get_Bit4()) {
                                // TODO Calls to RadarClass function higherup class stack, bad juju, should be virtual.
                                Map.Map_Cell(offset_cellnum, house);
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

int MapClass::Zone_Reset(int zones)
{
    // Loop through and reset every cells movement zones to 0.
    for (int index = 0; index < Array.Length(); ++index) {
        CellClass &cell = Array[index];

        if (zones & (1 << MZONE_NORMAL)) {
            cell.Set_Zone(MZONE_NORMAL, 0);
        }

        if (zones & (1 << MZONE_CRUSHER)) {
            cell.Set_Zone(MZONE_CRUSHER, 0);
        }

        if (zones & (1 << MZONE_DESTROYER)) {
            cell.Set_Zone(MZONE_DESTROYER, 0);
        }

        if (zones & (1 << MZONE_AMPHIBIOUS_DESTROYER)) {
            cell.Set_Zone(MZONE_AMPHIBIOUS_DESTROYER, 0);
        }
    }

    int tmp;

    // Calculate zones on the map that are all suitable for a given movement zone. Zones are contiguous areas on the map that can be reached using a given movement zone.
    if (zones & (1 << MZONE_NORMAL)) {
        tmp = 1;

        for (int cell = 0; cell < MAP_MAX_AREA; ++cell) {
            if (Zone_Span(cell, tmp, MZONE_NORMAL) != 0) {
                ++tmp;
            }
        }
    }

    if (zones & (1 << MZONE_CRUSHER)) {
        tmp = 1;

        for (int cell = 0; cell < MAP_MAX_AREA; ++cell) {

            if (Zone_Span(cell, tmp, MZONE_CRUSHER) != 0) {
                ++tmp;
            }
        }
    }

    if (zones & (1 << MZONE_DESTROYER)) {
        tmp = 1;

        for (int cell = 0; cell < MAP_MAX_AREA; ++cell) {
            if (Zone_Span(cell, tmp, MZONE_DESTROYER) != 0) {
                ++tmp;
            }
        }
    }

    if (zones & (1 << MZONE_AMPHIBIOUS_DESTROYER)) {
        tmp = 1;

        for (int cell = 0; cell < MAP_MAX_AREA; ++cell) {
            if (Zone_Span(cell, tmp, MZONE_AMPHIBIOUS_DESTROYER) != 0) {
                ++tmp;
            }
        }
    }

    return 0;
}

int MapClass::Zone_Span(int16_t cell, int zone, MZoneType mzone)
{
    int16_t coord_x = Cell_Get_X(cell);
    int16_t coord_y = Cell_Get_Y(cell);

    // Amphibious Destroyer is Vessels and other floaty things?
    SpeedType speed = (mzone == MZONE_AMPHIBIOUS_DESTROYER ? SPEED_FLOAT : SPEED_TRACK);

    // Check the coords are within the visible part of the map
    if (coord_y >= MapCellY && coord_y < (MapCellHeight + MapCellY) &&
        coord_x >= MapCellX && coord_x < (MapCellWidth + MapCellX)) {

        int retval = 0;
        int left_pos;

        // Scan backwards from coord to edge of map until we find a cell that is already marked for this zone or can't be moved to and record the pos.
        for (left_pos = coord_x; left_pos >= MapCellX; --left_pos) {
            CellClass &cell = Array[Cell_From_XY(left_pos, coord_y)];

            // Check if clear to move, ignoring units, buildings and infantry.
            if (cell.Get_Zone(mzone) != 0 ||
                !cell.Is_Clear_To_Move(speed, true, true, -1, mzone)) {
                if (left_pos == coord_x) {
                    return 0;
                }

                ++left_pos;
                break;
            }

        }

        left_pos = Max(left_pos, MapCellX);

        int right_pos;

        // Scan forward to other edge of the map doing the same.
        for (right_pos = coord_x; right_pos < MapCellWidth + MapCellX; ++right_pos) {
            CellClass &cell = Array[Cell_From_XY(right_pos, coord_y)];

            // Check if clear to move, ignoring units, buildings and infantry.
            if (cell.Get_Zone(mzone) != 0 ||
                !cell.Is_Clear_To_Move(speed, true, true, -1, mzone)) {
                --right_pos;
                break;
            }
        }

        right_pos = Min(right_pos, MapCellWidth + MapCellX - 1);

        // Set all the cells between the two positions to our zone value for this movement zone.
        for (int i = left_pos; i <= right_pos; ++i) {
            Array[Cell_From_XY(i, coord_y)].Set_Zone(mzone, zone);
            ++retval;
        }

        // If we have at least one cell, repeat the process above and below, sweet sweet recursion...
        if ((left_pos - 1) <= right_pos) {
            for (int i = left_pos - 1; i <= right_pos; ++i) {
                retval += Zone_Span(Cell_From_XY(i, coord_y - 1), zone, mzone);
                retval += Zone_Span(Cell_From_XY(i, coord_y + 1), zone, mzone);
            }
        }

        // Return the number of cells that are in this zone.
        return retval;
    }

    return 0;
}

int16_t MapClass::Nearby_Location(int16_t cellnum, SpeedType speed, int zone, MZoneType mzone) const
{
    DEBUG_ASSERT(cellnum < MAP_MAX_AREA);

    int16_t near_cells[10];
    int16_t near_cellnum;

    int near_cell_index = 0;
    int cell_x = Cell_Get_X(cellnum);
    int cell_y = Cell_Get_Y(cellnum);

    // Calculate the bounds of the area we want to check
    int left = cell_x - MapCellX;
    int right = MapCellWidth - left - 1;
    int top = cell_y - MapCellY;
    int bottom = MapCellHeight - top - 1;

    for (int i = 0; i < 64; ++i) {
        for (int j = -i; j <= i; ++j) {
            if (j >= -left && i <= top) {
                near_cellnum = Cell_From_XY(j + cell_x, cell_y - i);

                if (Map.In_Radar(near_cellnum)) {
                    if (Array[near_cellnum].Is_Clear_To_Move(speed, false, false, zone, mzone)) {
                        near_cells[near_cell_index++] = near_cellnum;
                    }
                }
            }

            if (near_cell_index == ARRAY_SIZE(near_cells)) {
                break;
            }

            if (j <= right && i <= bottom) {
                near_cellnum = Cell_From_XY(cell_x + j, i + cell_y);

                if (Map.In_Radar(near_cellnum)) {
                    if (Array[near_cellnum].Is_Clear_To_Move(speed, false, false, zone, mzone)) {
                        near_cells[near_cell_index++] = near_cellnum;
                    }
                }
            }

            if (near_cell_index == ARRAY_SIZE(near_cells)) {
                break;
            }
        }

        if (near_cell_index == ARRAY_SIZE(near_cells)) {
            break;
        }

        for (int k = -(i - 1); k <= i - 1; ++k) {
            if (k >= -top && i <= left) {
                near_cellnum = Cell_From_XY(cell_x - i, k + cell_y);

                if (Map.In_Radar(near_cellnum)) {
                    if (Array[near_cellnum].Is_Clear_To_Move(speed, false, false, zone, mzone)) {
                        near_cells[near_cell_index++] = near_cellnum;
                    }
                }
            }

            if (near_cell_index == ARRAY_SIZE(near_cells)) {
                break;
            }

            if (k <= bottom && i <= right) {
                near_cellnum = Cell_From_XY(cell_x + i, cell_y + k);

                if (Map.In_Radar(near_cellnum)) {
                    if (Array[near_cellnum].Is_Clear_To_Move(speed, false, false, zone, mzone)) {
                        near_cells[near_cell_index++] = near_cellnum;
                    }
                }
            }

            if (near_cell_index == ARRAY_SIZE(near_cells)) {
                break;
            }
        }

        if (near_cell_index > 0) {
            break;
        }
    }

    if (near_cell_index > 0) {
        return near_cells[g_frame % near_cell_index];
    }

    return 0;
}

BOOL MapClass::Base_Region(int16_t cellnum, HousesType &house, ZoneType &zone) const
{
    // TODO requires HouseClass
#ifndef RAPP_STANDALONE
    BOOL(*func)(const MapClass*, int16_t, HousesType&, ZoneType&) = reinterpret_cast<BOOL(*)(const MapClass*, int16_t, HousesType&, ZoneType&)>(0x004FFEAC);
    return func(this, cellnum, house, zone);
#elif 0
    if (cellnum < MAP_MAX_AREA && In_Radar(cellnum)) {
        for (house = HOUSES_FIRST; house < HOUSES_COUNT; ++house) {
            HouseClass *hptr = HouseClass::As_Pointer(house);
            DEBUG_ASSERT(hptr != nullptr);

            if (hptr != nullptr) {
                if (hptr->Is_Active() && !hptr->Defeated && Valid_Coord(hptr->field_2AE)) {
                    zone = hptr->Which_Zone(cellnum);

                    if (zone != ZONE_NONE) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
#else
    return false;
#endif
}

int MapClass::Destroy_Bridge_At(int16_t cellnum)
{
    // TODO Needs TemplateClass, AnimClass
#ifndef RAPP_STANDALONE
    int(*func)(const MapClass*, int16_t) = reinterpret_cast<int(*)(const MapClass*, int16_t)>(0x004FFF1C);
    return func(this, cellnum);
#else
    return 0;
#endif
}

void MapClass::Detach(int32_t target, int a2)
{
    // TODO Requires TriggerClass
#ifndef RAPP_STANDALONE
    void(*func)(const MapClass*, int32_t, int) = reinterpret_cast<void(*)(const MapClass*, int32_t, int)>(0x0050078C);
    func(this, target, a2);
#elif 0
    if (Target_Get_RTTI(target) == RTTI_TRIGGER) {
        for (int index = 0; index < MapTriggers.Count(); ++index) {
            TriggerClass *trig = MapTriggers[index];
            DEBUG_ASSERT(trig != nullptr);

            if (As_Trigger(target) == trig) {
                MapTriggers.Delete(trig);
                break;
            }
        }

        for (int cellnum = 0; cellnum < MAP_MAX_AREA; ++cellnum) {
            CellClass &cell = Array[cellnum];

            if (cell.CellTag == As_Trigger(target)) {
                cell.CellTag = nullptr;
            }
        }

    }
#endif
}
