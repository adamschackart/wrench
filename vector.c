/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- TODO: Move to 'math' directory once we get namespace resolution sorted out.
----------------------------------------------------------------------------- */

#define WRENCH_IMPLEMENTATION
#include <vector.h>

/*
================================================================================
 * ~~ [ integer vector ] ~~ *
--------------------------------------------------------------------------------
*/

static void vector_IntVector_ctor(WrenVM* vm)
{
    const int dimensions = wrenGetSlotInt(vm, 1);

    // TODO: VM small block allocator if size <= small block size.
    int* elements = (int*)wrench_calloc(dimensions, sizeof(int));

    if (elements != NULL)
    {
        vector_IntVector* self = (vector_IntVector*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vector_IntVector));
        WRENCH_SET_MAGIC_TAG(self, vector, IntVector);

        self->dimensions = dimensions;
        self->elements = elements;
    }
    else
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Out of memory! Failed to allocate IntVector of size %i.", dimensions);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void vector_IntVector_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, vector, IntVector);
    wrench_free(((vector_IntVector*)data)->elements);
}

static void vector_IntVector_dimensions_get(WrenVM* vm)
{
    vector_IntVector* self = (vector_IntVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, IntVector);

    wrenSetSlotInt(vm, 0, self->dimensions);
}

static void vector_IntVector_index1_get(WrenVM* vm)
{
    vector_IntVector* self = (vector_IntVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, IntVector);

    const int index = wrenGetSlotInt(vm, 1);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "IntVector%i[%i]", (int)self->dimensions, index);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    wrenSetSlotInt(vm, 0, self->elements[index]);
}

static void vector_IntVector_index1_set(WrenVM* vm)
{
    vector_IntVector* self = (vector_IntVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, IntVector);

    const int index = wrenGetSlotInt(vm, 1);
    const int value = wrenGetSlotInt(vm, 2);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "IntVector%i[%i]=%i", (int)self->dimensions, index, value);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    self->elements[index] = value;
}

/*
================================================================================
 * ~~ [ float vector ] ~~ *
--------------------------------------------------------------------------------
*/

static void vector_FltVector_ctor(WrenVM* vm)
{
    const int dimensions = wrenGetSlotInt(vm, 1);

    // TODO: Use VM small block allocation if size <= small block size.
    float* elements = (float*)wrench_calloc(dimensions, sizeof(float));

    if (elements != NULL)
    {
        vector_FltVector* self = (vector_FltVector*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vector_FltVector));
        WRENCH_SET_MAGIC_TAG(self, vector, FltVector);

        self->dimensions = dimensions;
        self->elements = elements;
    }
    else
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Out of memory! Failed to allocate FltVector of size %i.", dimensions);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void vector_FltVector_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, vector, FltVector);
    wrench_free(((vector_FltVector*)data)->elements);
}

static void vector_FltVector_dimensions_get(WrenVM* vm)
{
    vector_FltVector* self = (vector_FltVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, FltVector);

    wrenSetSlotInt(vm, 0, self->dimensions);
}

static void vector_FltVector_index1_get(WrenVM* vm)
{
    vector_FltVector* self = (vector_FltVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, FltVector);

    const int index = wrenGetSlotInt(vm, 1);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "FltVector%i[%i]", (int)self->dimensions, index);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    wrenSetSlotFloat(vm, 0, self->elements[index]);
}

static void vector_FltVector_index1_set(WrenVM* vm)
{
    vector_FltVector* self = (vector_FltVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, FltVector);

    const int index = wrenGetSlotInt(vm, 1);
    const float value = wrenGetSlotFloat(vm, 2);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "FltVector%i[%i]=%f", (int)self->dimensions, index, value);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    self->elements[index] = value;
}

/*
================================================================================
 * ~~ [ double vector ] ~~ *
--------------------------------------------------------------------------------
*/

static void vector_DblVector_ctor(WrenVM* vm)
{
    const int dimensions = wrenGetSlotInt(vm, 1);

    // TODO: Use the VM small block allocator if size <= small block size.
    double* elements = (double*)wrench_calloc(dimensions, sizeof(double));

    if (elements != NULL)
    {
        vector_DblVector* self = (vector_DblVector*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(vector_DblVector));
        WRENCH_SET_MAGIC_TAG(self, vector, DblVector);

        self->dimensions = dimensions;
        self->elements = elements;
    }
    else
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "Out of memory! Failed to allocate DblVector of size %i.", dimensions);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }
}

static void vector_DblVector_dtor(void* data)
{
    WRENCH_CHECK_MAGIC_TAG(data, vector, DblVector);
    wrench_free(((vector_DblVector*)data)->elements);
}

static void vector_DblVector_dimensions_get(WrenVM* vm)
{
    vector_DblVector* self = (vector_DblVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, DblVector);

    wrenSetSlotInt(vm, 0, self->dimensions);
}

static void vector_DblVector_index1_get(WrenVM* vm)
{
    vector_DblVector* self = (vector_DblVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, DblVector);

    const int index = wrenGetSlotInt(vm, 1);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "DblVector%i[%i]", (int)self->dimensions, index);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    wrenSetSlotDouble(vm, 0, self->elements[index]);
}

static void vector_DblVector_index1_set(WrenVM* vm)
{
    vector_DblVector* self = (vector_DblVector*)wrenGetSlotForeign(vm, 0);
    WRENCH_CHECK_MAGIC_TAG(self, vector, DblVector);

    const int index = wrenGetSlotInt(vm, 1);
    const double value = wrenGetSlotDouble(vm, 2);

    if (index < 0 || index >= self->dimensions)
    {
        char error[1024 * 4];
        wrench_snprintf(error, sizeof(error), "DblVector%i[%i]=%f", (int)self->dimensions, index, value);

        wrenSetSlotString(vm, 0, (const char*)error);
        wrenAbortFiber(vm, 0);
    }

    self->elements[index] = value;
}

/*
================================================================================
 * ~~ [ (un)hook ] ~~ *
--------------------------------------------------------------------------------
*/

#if WRENCH_FILE_EXTENDED
    /*
     * Enable user extension of stdlib modules.
     */
    #include <vector_ex.inl>
#else
    static bool vectorWrenInitEx(WrenVM* vm)
    {
        return true;
    }

    static void vectorWrenQuitEx(void)
    {
        //
    }
#endif /* WRENCH_FILE_EXTENDED */

WRENCH_EXPORT bool vectorWrenInit(WrenVM* vm)
{
    if (!wrenBeginModule(vm, "vector")) { return false; } else
    {
        WREN_BEGIN_CLASS(vector, IntVector);
        {
            // TODO: toString
            // TODO: fromString

            WREN_GETTER(vector, IntVector, false, dimensions);
            WREN_INDEX_PROPERTY(vector, IntVector, false, 1);

            if (!wrenCode(vm,

            "construct new(dimensions) {}\n"

            "toList {\n"
                "var v = []\n"

                "for (i in 0...dimensions) {\n"
                    "v.add(this[i])\n"
                "}\n"

                "return v\n"
            "}\n"

            "static fromList(list) {\n"
                "var v = new(list.count)\n"

                "for (i in 0...list.count) {\n"
                    "v[i] = list[i]\n"
                "}\n"

                "return v\n"
            "}\n"

            "iterate(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iterate\")\n"
            "}\n"

            "iteratorValue(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iteratorValue\")\n"
            "}\n"

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "z { this[2] }\n"
            "z=(value) { this[2] = value }\n"
            "w { this[3] }\n"
            "w=(value) { this[3] = value }\n"

            "static xyzw(x, y, z, w) {\n"
                "var self = new(4)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"
                "self[3] = w\n"

                "return self\n"
            "}\n"

            "static xyz(x, y, z) {\n"
                "var self = new(3)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"

                "return self\n"
            "}\n"

            "static xy(x, y) {\n"
                "var self = new(2)\n"

                "self[0] = x\n"
                "self[1] = y\n"

                "return self\n"
            "}\n"

            "copy { type.fromList(toList) }\n"
            "- { fromList(toList.map { |item| -item }.toList) }\n"

            )) { return false; }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(vector, FltVector);
        {
            // TODO: toString
            // TODO: fromString

            WREN_GETTER(vector, FltVector, false, dimensions);
            WREN_INDEX_PROPERTY(vector, FltVector, false, 1);

            if (!wrenCode(vm,

            "construct new(dimensions) {}\n"

            "toList {\n"
                "var v = []\n"

                "for (i in 0...dimensions) {\n"
                    "v.add(this[i])\n"
                "}\n"

                "return v\n"
            "}\n"

            "static fromList(list) {\n"
                "var v = new(list.count)\n"

                "for (i in 0...list.count) {\n"
                    "v[i] = list[i]\n"
                "}\n"

                "return v\n"
            "}\n"

            "iterate(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iterate\")\n"
            "}\n"

            "iteratorValue(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iteratorValue\")\n"
            "}\n"

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "z { this[2] }\n"
            "z=(value) { this[2] = value }\n"
            "w { this[3] }\n"
            "w=(value) { this[3] = value }\n"

            "r { this[0] }\n"
            "r=(value) { this[0] = value }\n"
            "g { this[1] }\n"
            "g=(value) { this[1] = value }\n"
            "b { this[2] }\n"
            "b=(value) { this[2] = value }\n"
            "a { this[3] }\n"
            "a=(value) { this[3] = value }\n"

            "S { this[0] }\n"
            "S=(value) { this[0] = value }\n"
            "T { this[1] }\n"
            "T=(value) { this[1] = value }\n"
            "R { this[2] }\n"
            "R=(value) { this[2] = value }\n"
            "Q { this[3] }\n"
            "Q=(value) { this[3] = value }\n"

            "static xyzw(x, y, z, w) {\n"
                "var self = new(4)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"
                "self[3] = w\n"

                "return self\n"
            "}\n"

            "static xyz(x, y, z) {\n"
                "var self = new(3)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"

                "return self\n"
            "}\n"

            "static xy(x, y) {\n"
                "var self = new(2)\n"

                "self[0] = x\n"
                "self[1] = y\n"

                "return self\n"
            "}\n"

            "static rgba(r, g, b, a) {\n"
                "var self = new(4)\n"

                "self[0] = r\n"
                "self[1] = g\n"
                "self[2] = b\n"
                "self[3] = a\n"

                "return self\n"
            "}\n"

            "static rgb(r, g, b) {\n"
                "var self = new(3)\n"

                "self[0] = r\n"
                "self[1] = g\n"
                "self[2] = b\n"

                "return self\n"
            "}\n"

            "static STRQ(S, T, R, Q) {\n"
                "var self = new(4)\n"

                "self[0] = S\n"
                "self[1] = T\n"
                "self[2] = R\n"
                "self[3] = Q\n"

                "return self\n"
            "}\n"

            "static STR(S, T, R) {\n"
                "var self = new(3)\n"

                "self[0] = S\n"
                "self[1] = T\n"
                "self[2] = R\n"

                "return self\n"
            "}\n"

            "static ST(S, T) {\n"
                "var self = new(2)\n"

                "self[0] = S\n"
                "self[1] = T\n"

                "return self\n"
            "}\n"

            "copy { type.fromList(toList) }\n"
            "- { fromList(toList.map { |item| -item }.toList) }\n"

            "+(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] + other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] + other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "-(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] - other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] - other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "*(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] * other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] * other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "/(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] / other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] / other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            )) { return false; }
        }
        WREN_END_CLASS();

        WREN_BEGIN_CLASS(vector, DblVector);
        {
            // TODO: toString
            // TODO: fromString

            WREN_GETTER(vector, DblVector, false, dimensions);
            WREN_INDEX_PROPERTY(vector, DblVector, false, 1);

            if (!wrenCode(vm,

            "construct new(dimensions) {}\n"

            "toList {\n"
                "var v = []\n"

                "for (i in 0...dimensions) {\n"
                    "v.add(this[i])\n"
                "}\n"

                "return v\n"
            "}\n"

            "static fromList(list) {\n"
                "var v = new(list.count)\n"

                "for (i in 0...list.count) {\n"
                    "v[i] = list[i]\n"
                "}\n"

                "return v\n"
            "}\n"

            "iterate(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iterate\")\n"
            "}\n"

            "iteratorValue(iter) {\n"
                "Fiber.abort(\"TODO %(type.name).iteratorValue\")\n"
            "}\n"

            "x { this[0] }\n"
            "x=(value) { this[0] = value }\n"
            "y { this[1] }\n"
            "y=(value) { this[1] = value }\n"
            "z { this[2] }\n"
            "z=(value) { this[2] = value }\n"
            "w { this[3] }\n"
            "w=(value) { this[3] = value }\n"

            "static xyzw(x, y, z, w) {\n"
                "var self = new(4)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"
                "self[3] = w\n"

                "return self\n"
            "}\n"

            "static xyz(x, y, z) {\n"
                "var self = new(3)\n"

                "self[0] = x\n"
                "self[1] = y\n"
                "self[2] = z\n"

                "return self\n"
            "}\n"

            "static xy(x, y) {\n"
                "var self = new(2)\n"

                "self[0] = x\n"
                "self[1] = y\n"

                "return self\n"
            "}\n"

            "copy { type.fromList(toList) }\n"
            "- { fromList(toList.map { |item| -item }.toList) }\n"

            "+(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] + other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] + other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "-(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] - other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] - other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "*(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] * other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] * other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            "/(other) {\n"
                "var v = type.new(dimensions)\n"

                "if (other is type) {\n"
                    #if WRENCH_DEBUG
                    "if (dimensions != other.dimensions) {\n"
                        "Fiber.abort(\"add lhs length (%(dimensions)) != rhs length (%(other.dimensions))\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] / other[i]\n"
                    "}\n"
                "} else {\n"
                    #if WRENCH_DEBUG
                    "if (!(other is Num)) {\n"
                        "Fiber.abort(\"%(other)\")\n"
                    "}\n"
                    #endif

                    "for (i in 0...dimensions) {\n"
                        "v[i] = this[i] / other\n"
                    "}\n"
                "}\n"

                "return v\n"
            "}\n"

            )) { return false; }
        }
        WREN_END_CLASS();
    }

    if (!vectorWrenInitEx(vm))
    {
        return false;
    }

    return wrenEndModule(vm);
}

WRENCH_EXPORT void vectorWrenQuit(void)
{
    vectorWrenQuitEx();
}
