#ifndef C4D_PASTE_FROM_EXTERNAL_H__
#define C4D_PASTE_FROM_EXTERNAL_H__

#include "c4d.h"

#define ID_C4D_PASTE_FROM_EXTERNAL 1039180

struct struct_vertexData{
    Float32 x;
    Float32 y;
    Float32 z;
    };

struct struct_polygonData{
    maxon::BaseArray<Int32> pts_id;
    String material_name;
    String type;
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
    maxon::BaseArray<struct_polygonData> polygonData;

    std::vector<struct_uvInfo> uvInfo;
    std::vector<struct_uvData> uvData;

    std::vector<String> weightName;
    std::vector<Float32> weightData;

    std::vector<String> morphName;
    std::vector<struct_morphData> morphData;
    };



class PasteFromExternal : public CommandData
{
public:
    //PasteFromExternal();
    //virtual ~PasteFromExternal();
    enum ReadState{ READ_NONE, READ_VERTICES, READ_POLYGONS, READ_WEIGHT, READ_MORPH, READ_UV};
    virtual iobject* PasteFromExternal::ParseFileToIobject();

    //virtual Bool Execute(BaseDocument* doc);

};
#endif