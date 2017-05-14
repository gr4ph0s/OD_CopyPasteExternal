#include <fstream>
#include <string>
#include <vector>
#include <c4d_basetag.h>

#include "c4d.h"
#include "C4D_PasteFromExternal.h"
#include "../Parser/ParserRead.h"
#include "../../DataStruct.h"

/*
enum ValidMesh{
    MESH_OK = 0,
    MESH_NO_POLY,
    MESH_NO_SPLINE,
    MESH_NOT_ALIVE
};

ValidMesh PasteFromExternal::bIsValidMesh(const BaseObject* obj)
{
ValidMesh returnValue = MESH_OK;

if (!obj)
return returnValue = MESH_NOT_ALIVE;

if (!obj->IsInstanceOf(Opolygon))
return returnValue = MESH_NO_POLY;

if (obj->IsInstanceOf(Ospline))
return returnValue = MESH_NO_SPLINE;

return returnValue;
}
*/




//Ugly function just for testing of the lolz !!! :)
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
        //Ngon don't know how handle this shit so For the moment I stop the creation if any ngons found:/
        else
            {
                PolygonObject::Free(polyObj);
                return false;
            }        
    }
    /*CREATE UV again there is no proper ngon handle.
    for (int i = 0; i < objData->polyCount; i++) {
        //for each UV Info create an UV tag
        UVWTag* uvwTag = UVWTag::Alloc(objData->polyCount);
        uvwTag->SetName(objData->uvInfo[i].uvName);

        Int32 globalCount = 0;

        //then we set each UV
        UVWStruct res;
        UVWHandle data = uvwTag->GetDataAddressW();
        for (Int32 y = globalCount; y<uvwTag->GetDataCount() + globalCount; y++){
            UVWTag::set(data, y, res);
        }
        globalCount += objData->uvInfo[i].uvCount;
    }
    */

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
    if (currentLine == linesCount)
        objData->IsReadFinnished = true;

    StatusClear();
    return;
}

Bool PasteFromExternal::Execute(BaseDocument *doc)
    {
    AutoNew<iobject> dataObj;
    ParseFileToIobject(dataObj);
    if (dataObj == nullptr || !dataObj->IsReadFinnished)
        GePrint("error");

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