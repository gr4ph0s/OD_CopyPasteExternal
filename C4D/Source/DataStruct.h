#ifndef DATASTRUCT_H__
#define DATASTRUCT_H__

#include "c4d.h"

enum ReadState{ READ_NONE, READ_VERTICES, READ_POLYGONS, READ_WEIGHT, READ_MORPH, READ_UV };

enum face_type{ FACE, SUBD, CCSS };

struct struct_vertexData{
    Float32 x;
    Float32 y;
    Float32 z;
    };

struct struct_polygonData{
    maxon::BaseArray<Int32> pts_id;
    String material_name;
    face_type type;
    };

struct struct_uvInfo{
    String uvName;
    Int32 uvCount;
    };

struct struct_uvData{
    Bool isContinuous;
    Float32 u;
    Float32 v;
    Int32 poly_id;
    Int32 pt_id;
    };

struct struct_morphData{
    Float32 delta_x;
    Float32 delta_y;
    Float32 delta_z;
    };

struct iobject{
    Int32 vertexCount;
    maxon::BaseArray<struct_vertexData> vertexData;

    Int32 polyCount;
    maxon::BaseArray<struct_polygonData*> polygonData;

    maxon::BaseArray<struct_uvInfo> uvInfo;
    maxon::BaseArray<struct_uvData> uvData;

    maxon::BaseArray<String> weightName;
    maxon::BaseArray<Float32> weightData;

    maxon::BaseArray<String> morphName;
    maxon::BaseArray<struct_morphData> morphData;

    Bool IsReadFinnished;
    };

#endif