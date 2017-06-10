#include <fstream>
#include <string>
#include <vector>
#include <c4d_basetag.h>
#include <c4d_baseobject.h>
#include <operatingsystem.h>

#include "c4d.h"
#include "C4D_PasteFromExternal.h"
#include "../Parser/ParserRead.h"
#include "../../DataStruct.h"


// Made from my pythonic version https://gist.github.com/gr4ph0s/ebecaabcf90bbdd73a54124544e0b532
void PasteFromExternal::GetLinkedDiscontinuous(PolygonObject* pObj, Int32 pt_id, Int32 poly_id, maxon::BaseArray<struct_uv>* list_id){
    const CPolygon* polys_data = pObj->GetPolygonR();
    const CPolygon poly = polys_data[poly_id];

    Int32 found_id = poly.Find(pt_id);

    if (found_id != NOTOK){
        struct_uv buffer_struct;
        buffer_struct.poly_id = poly_id;
        buffer_struct.pt_num = found_id;
        list_id->Append(buffer_struct);
        }
}

// Made from my pythonic version https://gist.github.com/gr4ph0s/ebecaabcf90bbdd73a54124544e0b532
void PasteFromExternal::GetLinkedContinuous(PolygonObject* pObj, Int32 pt_id, maxon::BaseArray<struct_uv>* list_id){
    Neighbor nbr = Neighbor();
    nbr.Init(pObj->GetPointCount(), pObj->GetPolygonR(), pObj->GetPolygonCount(), nullptr);
    Int32 *linkedPoly = nullptr, linkedPolyCount = 0;
    nbr.GetPointPolys(pt_id, &linkedPoly, &linkedPolyCount);
    const CPolygon* polys_data = pObj->GetPolygonR();

    for (Int32 poly_id = 0; poly_id<linkedPolyCount; poly_id++){
        const CPolygon poly = polys_data[poly_id];
        Int32 found_id = poly.Find(pt_id);

        if (found_id != NOTOK){
            struct_uv buffer_struct;
            buffer_struct.poly_id = poly_id;
            buffer_struct.pt_num = found_id;
            list_id->Append(buffer_struct);
        }
    }
}

void PasteFromExternal::SetUv(PolygonObject* pObj, UVWTag* uvTag, struct_uvData* uvData, BaseDocument* doc){
    Vector vec = Vector(uvData->u, uvData->v, 0);

    maxon::BaseArray<struct_uv>* linked_data = NewObj(maxon::BaseArray<struct_uv>);
    if (uvData->isContinuous)
        GetLinkedContinuous(pObj, uvData->pt_id, linked_data);
    else
        GetLinkedDiscontinuous(pObj, uvData->pt_id, uvData->pt_id, linked_data);

    doc->AddUndo(UNDOTYPE_CHANGE, uvTag);


    for (struct_uv& data : *linked_data){
        UVWStruct buffer_uv = uvTag->GetSlow(data.poly_id);
        buffer_uv[data.pt_num] = vec;
        uvTag->SetSlow(data.poly_id, buffer_uv);
        }

}

Bool PasteFromExternal::CreateUV(const iobject* objData, BaseDocument* doc, PolygonObject* obj)
{
/*
    CREATE UV there is no proper ngon handle.
*/

    //for each UV Info create an UV tag
    for (int i = 0; i < objData->uvInfo.GetCount(); i++) {
        UVWTag* uvwTag = UVWTag::Alloc(objData->polyCount);
        uvwTag->SetName(objData->uvInfo[i].uvName);

        Int32 globalCount = 0;

        //then we set each UV
        for (int i = 0; i < objData->uvData.GetCount(); i++){
            struct_uvData buffer_data = objData->uvData[i];
            this->SetUv(obj, uvwTag, &buffer_data, doc);
            }

        //Add tag to the obj
        obj->InsertTag(uvwTag);

        globalCount += objData->uvInfo[i].uvCount;
    }
    return true;
}

//Create PolyObj
Bool PasteFromExternal::CreatePolyObj(const iobject* objData,BaseDocument *doc)
{
    StopAllThreads();
    doc->StartUndo();

    PolygonObject* polyObj = PolygonObject::Alloc(objData->vertexCount, objData->polyCount);
    PointObject* ptObj = ToPoint(polyObj);

    //Set Vertices
    //Get the write array of the obj
    Vector* list_ptObj_write = ptObj->GetPointW();
    for (int i = 0; i < objData->vertexCount; i++) {
        list_ptObj_write[i] = Vector(objData->vertexData[i].x, objData->vertexData[i].y, objData->vertexData[i].z);
    }
    
    //Set Polygons
    CPolygon* list_polyObj_write = polyObj->GetPolygonW();
    for (int i = 0; i < objData->polyCount; i++) {
        Int32 count_pts_in_poly = objData->polygonData[i]->pts_id.GetCount();

        //Triangle a,b,c and d=c
        if (count_pts_in_poly == 3){
            for (int y = 0; y < count_pts_in_poly; y++){
                list_polyObj_write[i][y] = objData->polygonData[i]->pts_id[y];
                }
            list_polyObj_write[i].d = objData->polygonData[i]->pts_id[2];
         }

         //Quad
        else if (count_pts_in_poly == 4){
            for (int y = 0; y < count_pts_in_poly; y++){
                list_polyObj_write[i][y] = objData->polygonData[i]->pts_id[y];
                }
            }
        //Ngon don't know how handle this shit so For the moment I stop the creation if any ngons found :/
        else
            {
                PolygonObject::Free(polyObj);
                return false;
            }        
    }
    this->CreateUV(objData, doc, polyObj);


    doc->AddUndo(UNDOTYPE_NEW, polyObj);
    doc->InsertObject(polyObj, nullptr, nullptr);

    doc->EndUndo();
    EventAdd();
    return true;
}

//Read temp file and store it into an iobject struct
void PasteFromExternal::ParseFileToIobject(iobject* objData)
{
    Bool debug = true;
    objData->IsReadFinnished = false;
    StatusSetSpin();
    std::string filePath = GetPathTempFile();

    //Read data
    std::ifstream* file = NewObj(std::ifstream, filePath);
    if (!file){
        if (debug)
            GePrint("Can't read File");
        return;
    }

    Int32 linesCount = GetLineCount(file);

    //init data for read
    std::string line;
    ReadState toRead = READ_NONE;
    Int32 linesToRead = 0;
    Int32 linesReaded = 0;
    Int32 currentLine = 0;
    Int32 percent = 0;
    Bool Error = false;
    while (std::getline(*file, line) && !Error)
    {
    switch (toRead)
        {
        case READ_NONE:
            if (line.find("VERTICES:") == 0)
                ReadVerticesHeader(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);

            else if (line.find("POLYGONS:") == 0)
                ReadPolygonsHeader(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);

            else if (line.find("WEIGHT:") == 0)
                ReadWeightHeader(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);

            else if (line.find("UV:") == 0)
                ReadUVHeader(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);

            else if (line.find("MORPH:") == 0)
                ReadMorphHeader(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
    
            break;

        case READ_VERTICES:
            ReadVertices(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
            break;

        case READ_POLYGONS:
            ReadPolygons(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
            break;

        // read Weight
        case READ_WEIGHT:
            ReadWeights(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
            break;


        case READ_UV:
            ReadUVs(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
            break;

        //read morph
        case READ_MORPH:
            ReadMorphs(objData, line, &linesToRead, &toRead, &linesReaded, &debug, &Error);
            break;

        }
    currentLine++;
    percent = (currentLine * 100) / linesCount;
    StatusSetBar(percent);
    }
    //set data to our returned data only if we have read everything
    if (currentLine == linesCount + 1)
        objData->IsReadFinnished = true;

    StatusClear();
    return;
}

Bool PasteFromExternal::Execute(BaseDocument *doc)
    {
    AutoNew<iobject> dataObj;
    ParseFileToIobject(dataObj);
    if (dataObj == nullptr || !dataObj->IsReadFinnished){
        GePrint("error");
        return true;
    }

    CreatePolyObj(dataObj, doc);
    return true;
    }

Bool RegisterPasteFromExternal()
{
    const String name = "C4D_PasteFromExternal";
    return RegisterCommandPlugin(
        ID_C4D_PASTE_FROM_EXTERNAL,
        name,
        0,
        nullptr,
        String(),
        NewObjClear(PasteFromExternal));
}