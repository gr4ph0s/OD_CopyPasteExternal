#ifndef C4D_PASTE_FROM_EXTERNAL_H__
#define C4D_PASTE_FROM_EXTERNAL_H__

#include "c4d.h"
#include <c4d_basetag.h>

#include "../../DataStruct.h"

#define ID_C4D_PASTE_FROM_EXTERNAL 1039180


class PasteFromExternal : public CommandData
{
public:
    //PasteFromExternal();
    //virtual ~PasteFromExternal();
 
    virtual Bool Execute(BaseDocument *doc);

    void PasteFromExternal::GetLinkedDiscontinuous(PolygonObject* pObj, Int32 pt_id, Int32 poly_id, maxon::BaseArray<struct_uv>* list_id);
    void PasteFromExternal::GetLinkedContinuous(PolygonObject* pObj, Int32 pt_id, maxon::BaseArray<struct_uv>* list_id);
    void PasteFromExternal::SetUv(PolygonObject* pObj, UVWTag* uvTag, struct_uvData* uvData, BaseDocument* doc);

    Bool PasteFromExternal::CreateUV(const iobject* objData, BaseDocument* doc, PolygonObject* obj);

    std::vector<std::string> split(const std::string& str, const std::string& delim);
    void ParseFileToIobject(iobject* objData);
    Bool PasteFromExternal::CreatePolyObj(const iobject* objData, BaseDocument *doc);
    //virtual Bool Execute(BaseDocument* doc);

};
#endif