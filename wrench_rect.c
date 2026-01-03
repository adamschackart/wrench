/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- TODO: Move to 'math' directory once we get namespace resolution sorted out.
----------------------------------------------------------------------------- */

#ifndef WRENCH_IMPLEMENTATION
#define WRENCH_IMPLEMENTATION 1
#endif
#include <wrench_rect.h>

/*
================================================================================
 * ~~ [ integer rectangle ] ~~ *
--------------------------------------------------------------------------------
*/

static void rect_IntRect_ctor(WrenVM* vm)
{
    rect_IntRect* self = (rect_IntRect*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(rect_IntRect));
    WRENCH_SET_MAGIC_TAG(self, rect, IntRect);
}

static void rect_IntRect_dtor(void* data)
{
    //
}

static void rect_IntRect_index1_get(WrenVM* vm)
{
    rect_IntRect* self = (rect_IntRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, IntRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    wrenSetSlotInt(vm, 0, self->xywh[index]);
}

static void rect_IntRect_index1_set(WrenVM* vm)
{
    rect_IntRect* self = (rect_IntRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, IntRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    self->xywh[index] = wrenGetSlotInt(vm, 2);
}

/*
================================================================================
 * ~~ [ float rectangle ] ~~ *
--------------------------------------------------------------------------------
*/

static void rect_FltRect_ctor(WrenVM* vm)
{
    rect_FltRect* self = (rect_FltRect*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(rect_FltRect));
    WRENCH_SET_MAGIC_TAG(self, rect, FltRect);
}

static void rect_FltRect_dtor(void* data)
{
    //
}

static void rect_FltRect_index1_get(WrenVM* vm)
{
    rect_FltRect* self = (rect_FltRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, FltRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    wrenSetSlotFloat(vm, 0, self->xywh[index]);
}

static void rect_FltRect_index1_set(WrenVM* vm)
{
    rect_FltRect* self = (rect_FltRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, FltRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    self->xywh[index] = wrenGetSlotFloat(vm, 2);
}

/*
================================================================================
 * ~~ [ double rectangle ] ~~ *
--------------------------------------------------------------------------------
*/

static void rect_DblRect_ctor(WrenVM* vm)
{
    rect_DblRect* self = (rect_DblRect*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(rect_DblRect));
    WRENCH_SET_MAGIC_TAG(self, rect, DblRect);
}

static void rect_DblRect_dtor(void* data)
{
    //
}

static void rect_DblRect_index1_get(WrenVM* vm)
{
    rect_DblRect* self = (rect_DblRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, DblRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    wrenSetSlotDouble(vm, 0, self->xywh[index]);
}

static void rect_DblRect_index1_set(WrenVM* vm)
{
    rect_DblRect* self = (rect_DblRect*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, rect, DblRect);

    const int index = wrenGetSlotInt(vm, 1);
    wrench_assert(index >= 0 && index < 4, "%i", index);

    self->xywh[index] = wrenGetSlotDouble(vm, 2);
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_RECT_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #ifndef __RECT_EX_INL__
    #include <rect_ex.inl>
    #endif
#else
    static bool rectWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void rectWrenQuitEx(void)
    {
        //
    }

    static bool rectIntRectWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool rectFltRectWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static bool rectDblRectWrenInitEx(WrenVM* vm)
    {
        return true;
    }
#endif /* WRENCH_RECT_EXTENDED */

WRENCH_EXPORT bool rectWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "rect")) { return false; } else
    {
        WREN_BEGIN_CLASS(rect, IntRect);
        {
            if (!wrenCode(vm,

            "construct new() {}\n"

            "static create(x, y, w, h) {\n"
                "var v = new()\n"

                "v[0] = x\n"
                "v[1] = y\n"
                "v[2] = w\n"
                "v[3] = h\n"

                "return v\n"
            "}\n"

            "static create(xy, wh) {\n"
                "var v = new()\n"

                "v[0] = xy[0]\n"
                "v[1] = xy[1]\n"
                "v[2] = wh[0]\n"
                "v[3] = wh[1]\n"

                "return v\n"
            "}\n"

            )) { return false; }

            WREN_INDEX_PROPERTY(rect, IntRect, false, 1);

            if (!wrenCode(vm,

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "w { this[2] }\n"
            "w=(value) { this[2] = value }\n"
            "h { this[3] }\n"
            "h=(value) { this[3] = value }\n"
            "width { this[2] }\n"
            "width=(value) { this[2] = value }\n"
            "height { this[3] }\n"
            "height=(value) { this[3] = value }\n"

            )) { return false; }

            // TODO: xy
            // TODO: xy=(value)
            // TODO: wh
            // TODO: wh=(value)
            // TODO: toList
            // TODO: fromList
            // TODO: toString
            // TODO: fromString
            // TODO: iterate
            // TODO: iteratorValue
            // TODO: copy
            // TODO: ==
            // TODO: !=
            // TODO: zero
            // TODO: is_zero (! operator)
            // TODO: center
            // TODO: center=(value)
            // TODO: centerIn
            // TODO: area
            // TODO: aabbox
            // TODO: aabbox=(value)
            // TODO: intersects
            // TODO: intersectsRect
            // TODO: intersectsPoint
            // TODO: flt
            // TODO: flt=(value)
            // TODO: dbl
            // TODO: dbl=(value)

            if (!rectIntRectWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(rect, FltRect);
        {
            if (!wrenCode(vm,

            "construct new() {}\n"

            "static create(x, y, w, h) {\n"
                "var v = new()\n"

                "v[0] = x\n"
                "v[1] = y\n"
                "v[2] = w\n"
                "v[3] = h\n"

                "return v\n"
            "}\n"

            "static create(xy, wh) {\n"
                "var v = new()\n"

                "v[0] = xy[0]\n"
                "v[1] = xy[1]\n"
                "v[2] = wh[0]\n"
                "v[3] = wh[1]\n"

                "return v\n"
            "}\n"

            )) { return false; }

            WREN_INDEX_PROPERTY(rect, FltRect, false, 1);

            if (!wrenCode(vm,

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "w { this[2] }\n"
            "w=(value) { this[2] = value }\n"
            "h { this[3] }\n"
            "h=(value) { this[3] = value }\n"
            "width { this[2] }\n"
            "width=(value) { this[2] = value }\n"
            "height { this[3] }\n"
            "height=(value) { this[3] = value }\n"

            )) { return false; }

            // TODO: xy
            // TODO: xy=(value)
            // TODO: wh
            // TODO: wh=(value)
            // TODO: toList
            // TODO: fromList
            // TODO: toString
            // TODO: fromString
            // TODO: iterate
            // TODO: iteratorValue
            // TODO: copy
            // TODO: ==
            // TODO: !=
            // TODO: zero
            // TODO: is_zero (! operator)
            // TODO: center
            // TODO: center=(value)
            // TODO: centerIn
            // TODO: area
            // TODO: aabbox
            // TODO: aabbox=(value)
            // TODO: intersects
            // TODO: intersectsRect
            // TODO: intersectsPoint
            // TODO: int
            // TODO: int=(value)
            // TODO: dbl
            // TODO: dbl=(value)

            if (!rectFltRectWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(rect, DblRect);
        {
            if (!wrenCode(vm,

            "construct new() {}\n"

            "static create(x, y, w, h) {\n"
                "var v = new()\n"

                "v[0] = x\n"
                "v[1] = y\n"
                "v[2] = w\n"
                "v[3] = h\n"

                "return v\n"
            "}\n"

            "static create(xy, wh) {\n"
                "var v = new()\n"

                "v[0] = xy[0]\n"
                "v[1] = xy[1]\n"
                "v[2] = wh[0]\n"
                "v[3] = wh[1]\n"

                "return v\n"
            "}\n"

            )) { return false; }

            WREN_INDEX_PROPERTY(rect, DblRect, false, 1);

            if (!wrenCode(vm,

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "w { this[2] }\n"
            "w=(value) { this[2] = value }\n"
            "h { this[3] }\n"
            "h=(value) { this[3] = value }\n"
            "width { this[2] }\n"
            "width=(value) { this[2] = value }\n"
            "height { this[3] }\n"
            "height=(value) { this[3] = value }\n"

            )) { return false; }

            // TODO: xy
            // TODO: xy=(value)
            // TODO: wh
            // TODO: wh=(value)
            // TODO: toList
            // TODO: fromList
            // TODO: toString
            // TODO: fromString
            // TODO: iterate
            // TODO: iteratorValue
            // TODO: copy
            // TODO: ==
            // TODO: !=
            // TODO: zero
            // TODO: is_zero (! operator)
            // TODO: center
            // TODO: center=(value)
            // TODO: centerIn
            // TODO: area
            // TODO: aabbox
            // TODO: aabbox=(value)
            // TODO: intersects
            // TODO: intersectsRect
            // TODO: intersectsPoint
            // TODO: int
            // TODO: int=(value)
            // TODO: flt
            // TODO: flt=(value)

            if (!rectDblRectWrenInitEx(vm))
            {
                return false;
            }
        }
        WREN_END_CLASS();
    }

    if (!rectWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void rectWrenQuit(void)
{
    rectWrenQuitEx();
}
