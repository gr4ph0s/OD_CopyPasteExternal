#include <fstream>
#include <string>
#include <vector>
#include "c4d.h"
#include "C4D_PasteFromExternal.h"

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

//Split a string according a given delimiter
std::vector<std::string> PasteFromExternal::split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
        {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
    return tokens;
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

    CPolygon* list_polyObj_write = polyObj->GetPolygonW();
    for (int i = 0; i < objData->polyCount; i++) {
        Int32 count_pts_in_poly = objData->polygonData[i]->pts_id.GetCount();
        for (int y = 0; y < count_pts_in_poly; y++){
            list_polyObj_write[i][y] = objData->polygonData[i]->pts_id[y];
        }
    }

    doc->AddUndo(UNDOTYPE_NEW, polyObj);
    doc->InsertObject(polyObj, nullptr, nullptr);

    doc->EndUndo();
    EventAdd();
    return true;
}


//Ugly function too will be moved to parser class and cut
void PasteFromExternal::ParseFileToIobject(iobject* objData)
{
    Bool debug = true;
    objData->IsReadFinnished = false;

    //GetTempDir
    char const *tempdirchar = getenv("TMPDIR");
    if (tempdirchar == 0)
        tempdirchar = getenv("TMP");

    if (tempdirchar == 0)
        tempdirchar = getenv("TEMP");

    if (tempdirchar == 0)
        tempdirchar = getenv("TEMPDIR");

    if (tempdirchar == 0)
        tempdirchar = getenv("USERPROFILE");

    if (tempdirchar == 0)
        tempdirchar = "/tmp";

    //Build full path for temp data
    std::string tempdir = tempdirchar;
    #ifdef _WIN32
    std::string folder = tempdir + '\\' + "ODVertexData.txt";
    #else
    std::string folder = tempdir + '/' + "ODVertexData.txt";
    #endif

    //Read data
    std::ifstream file(folder);
    if (!file){
        if (debug)
            GePrint("Can't read File");
        return;
    }

    std::string line;
    ReadState toRead = READ_NONE;
    Int32 linesToRead = 0;
    Int32 linesReaded = 0;
    while (std::getline(file, line))
    {
    switch (toRead)
        {
        case READ_NONE:
            {
            if (line.find("VERTICES:") == 0)
                {
                //get the number of vertices
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2){
                    if (debug)
                        GePrint("VERTICE HEADER - Bad formating");
                    return;
                }

                //parse it in Int32 and fill data
                String maxonString = strData[1].c_str();
                linesToRead = maxonString.ToInt32();
                objData->vertexCount = linesToRead;

                //Set read mode to Vertices
                toRead = READ_VERTICES;
                linesReaded = 0;
                }

            else if (line.find("POLYGONS:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2){
                    if (debug)
                        GePrint("POLY HEADER - Bad formating");
                    return;
                }

                String maxonString = strData[1].c_str();
                linesToRead = maxonString.ToInt32();
                objData->polyCount = linesToRead;

                toRead = READ_POLYGONS;
                linesReaded = 0;
                }

            else if (line.find("WEIGHT:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2){
                    if (debug)
                        GePrint("WEIGHT HEADER - Bad formating");
                    return;
                }
                String maxonString = strData[0].c_str();
                if (objData->weightName.Append(maxonString) == nullptr){
                    if (debug)
                        GePrint("WEIGHT HEADER - FAIL INSERT");
                    return;
                }
                toRead = READ_WEIGHT;
                linesReaded = 0;
                }
            else if (line.find("UV:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 3){
                    if (debug)
                        GePrint("UV HEADER - BAD HEADER");
                    return;
                }
                String maxonStringName = strData[1].c_str();
                String maxonStringUvCount = strData[2].c_str();

                struct_uvInfo buffer_struct;
                buffer_struct.uvName = maxonStringName;
                buffer_struct.uvCount = maxonStringUvCount.ToInt32();

                if (objData->uvInfo.Append(buffer_struct) == nullptr){
                    if (debug)
                        GePrint("UV HEADER - FAIL INSERT");
                    return;
                }
                toRead = READ_UV;
                linesReaded = 0;
                }
            else if (line.find("MORPH:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2){
                    if (debug)
                        GePrint("MORPH HEADER - BAD HEADER");
                    return;
                }
                String maxonString = strData[1].c_str();
                if (objData->morphName.Append(maxonString) == nullptr){
                    if (debug)
                        GePrint("UV HEADER - FAIL INSERT");
                    return;
                }
                toRead = READ_WEIGHT;
                linesReaded = 0;
                }
    
            break;
            }

        // read vertices data
        case READ_VERTICES:
            {
            //Get Data in std type
            std::vector<std::string> strData = this->split(line, " ");
            if (strData.size() != 3){
                if (debug)
                    GePrint("VERTICE - BAD Formating");
                return;
            }

            //translate into c4d
            String maxonStringx = strData[0].c_str();
            String maxonStringy = strData[1].c_str();
            String maxonStringz = strData[2].c_str();

            //fill our struct
            struct_vertexData vertexData;
            vertexData.x = maxonStringx.ToFloat();
            vertexData.y = maxonStringy.ToFloat();
            vertexData.z = maxonStringz.ToFloat();

            //append data to our list
            if (objData->vertexData.Append(vertexData) == nullptr){
                if (debug)
                    GePrint("VERTICE - FAIL INSERT - 01");
                return;
            }
            
            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead)
                toRead = READ_NONE;

            break;
            }

        case READ_POLYGONS:
            {
            //Split the line and check if it's correctly formatted
            std::vector<std::string> strData = this->split(line, ";;");
            if (strData.size() != 3){
                if (debug)
                    GePrint("FACE - BAD FORMATTING");
                return;
            }

            objData->polygonData.Append();
            Int32 last_id = objData->polygonData.GetCount() - 1;
            if (last_id < 0){
                if (debug)
                    GePrint("FACE - BAD Last_ID");
                return;
            }
            objData->polygonData[last_id] = NewObjClear(struct_polygonData);
            

            //Get polygon Data
            std::vector<std::string> PolyIdstrData = this->split(line, ",");

            //Get Material Name
            String maxonStringMaterialName = strData[1].c_str();

            //Get Face Type
            face_type faceData;
            if (strData[2] == "SubD")
                faceData = SUBD;
            else if (strData[2] == "CCSS")
                faceData = CCSS;
            else
                faceData = FACE;

            //translate them c4d
            String maxonStringx = strData[0].c_str();
            String maxonStringy = strData[1].c_str();
            String maxonStringz = strData[2].c_str();

            //Convert them from std::string to Int32
            for (std::string i : PolyIdstrData)
                {
                String buffer_String_pt_id = i.c_str();
                Int32 buffer_int32 = buffer_String_pt_id.ToInt32();
                if (objData->polygonData[last_id]->pts_id.Append(buffer_int32) == nullptr){
                    if (debug)
                        GePrint("FACE - FAIL INSERT - 02");
                    return;
                }
                }

            //fill our struct
            objData->polygonData[last_id]->material_name = strData[1].c_str();
            objData->polygonData[last_id]->type = faceData;

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead)
                toRead = READ_NONE;

            break;
            }

        // read Weight
        case READ_WEIGHT:
            {
            //no split needed so we can directly cast it into c4d type
            String maxonStringData = line.c_str();

            //append data to our list
            if (objData->weightData.Append(maxonStringData.ToFloat()) == nullptr){
                if (debug)
                    GePrint("WEIGHT - FAIL INSERT");
                return;
            }

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead)
                toRead = READ_NONE;

            break;
            }

        case READ_UV:
            {
            //Split the line and check if it's correctly formatted
            std::vector<std::string> strData = this->split(line, ":");
            if (strData.size() != 3 && strData.size() != 5){
                if (debug)
                    GePrint("UV - BAD FORMATTING - 01");
                return;
            }

            struct_uvData uvData;

            //Get uv coordinate
            std::vector<std::string> uvStrData = this->split(strData[0], " ");
            if (uvStrData.size() != 2){
                if (debug)
                    GePrint("UV - BAD FORMATTING - 02");
                return;
            }
                
            //translate into c4d
            String maxonStringU = uvStrData[0].c_str();
            String maxonStringV = uvStrData[1].c_str();

            Bool isContinuous = false;
            String maxonStringPoly_id, maxonStringPt_id;

            //if discontinuous UV
            if (uvStrData.size() == 5){
                isContinuous = false;
                maxonStringPoly_id = strData[2].c_str();
                maxonStringPt_id = strData[4].c_str();
            }
            else{
                isContinuous = true;
                maxonStringPoly_id = "0";
                maxonStringPt_id = strData[2].c_str();
            }

            //Fill our struct
            uvData.isContinuous = isContinuous;
            uvData.u = maxonStringU.ToFloat();
            uvData.v = maxonStringU.ToFloat();
            uvData.pt_id = maxonStringPt_id.ToInt32();
            uvData.poly_id = maxonStringPoly_id.ToInt32();
                           
            //append data to our list
            if (objData->uvData.Append(uvData) == nullptr){
                if (debug)
                    GePrint("UV - FAIL INSERT");
                return;
            }

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead)
                toRead = READ_NONE;
            }
            break;

        //read morph
        case READ_MORPH:
            {
            struct_morphData morphData;

            //Check if It's None and assign Null value
            if (line.find("None") == 0)
            {
                Float32 NoneValue = 0.0;
                morphData.delta_x = NoneValue;
                morphData.delta_y = NoneValue;
                morphData.delta_z = NoneValue;
            }

            //Split the line and check if it's correctly formatted
            std::vector<std::string> strData = this->split(line, " ");
            if (strData.size() != 3){
                if (debug)
                    GePrint("MORPH - BAD FORMATTING");
                return;
            }

            //translate into c4d
            String maxonStringx = strData[0].c_str();
            String maxonStringy = strData[1].c_str();
            String maxonStringz = strData[2].c_str();

            //fill our struct
            morphData.delta_x = maxonStringx.ToFloat();
            morphData.delta_y = maxonStringy.ToFloat();
            morphData.delta_z = maxonStringz.ToFloat();

            //append data to our list
            if (objData->morphData.Append(morphData) == nullptr){
                if (debug)
                    GePrint("MORPH - FAIL INSERT");
                return;
            }

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead)
                toRead = READ_NONE;

            break;
            }
        }
    }
    //set data to our returned data
    objData->IsReadFinnished = true;
    return;
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