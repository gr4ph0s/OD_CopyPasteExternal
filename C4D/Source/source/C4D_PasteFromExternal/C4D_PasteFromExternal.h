#ifndef C4D_PASTE_FROM_EXTERNAL_H__
#define C4D_PASTE_FROM_EXTERNAL_H__

#include "c4d.h"

#define ID_C4D_PASTE_FROM_EXTERNAL 1039180

enum face_type{FACE,SUBD,CCSS};

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



class PasteFromExternal : public CommandData
{
public:
    //PasteFromExternal();
    //virtual ~PasteFromExternal();
    enum ReadState{ READ_NONE, READ_VERTICES, READ_POLYGONS, READ_WEIGHT, READ_MORPH, READ_UV};

    virtual Bool Execute(BaseDocument *doc);

    std::vector<std::string> split(const std::string& str, const std::string& delim);
    void ParseFileToIobject(iobject* objData);
    Bool PasteFromExternal::CreatePolyObj(const iobject* objData, BaseDocument *doc);


    //virtual Bool Execute(BaseDocument* doc);

};
#endif