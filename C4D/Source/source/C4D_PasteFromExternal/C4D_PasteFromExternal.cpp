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
std::vector<std::string> split(const std::string& str, const std::string& delim)
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


iobject* PasteFromExternal::ParseFileToIobject()
{
    //GetTempDir
    char const *tempdirchar = getenv("TMPDIR");
    if (tempdirchar == 0)
        tempdirchar = "/tmp";

    //Build full path for temp data
    std::string tempdir = tempdirchar;
    #ifdef _WIN32
    std::string folder = tempdir + '\\' + "ODVertexData.txt";
    #else
    std::string folder = tempdir + '/' + "ODVertexData.txt";
    #endif

    //inite our object data
    AutoNew<iobject> objectData;

    //Read data
    std::ifstream file(folder);
    if (!file)
        return nullptr;

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
                if (strData.size() != 2)
                    return nullptr;

                //parse it in Int32 and fill data
                String maxonString = strData[1].c_str();
                linesToRead = maxonString.ToInt32();
                objectData->vertexCount = linesToRead;

                //Set read mode to Vertices
                toRead = READ_VERTICES;
                linesReaded = 0;
                }

            else if (line.find("POLYGONS:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2)
                    return nullptr;

                String maxonString = strData[1].c_str();
                linesToRead = maxonString.ToInt32();
                objectData->polyCount = linesToRead;

                toRead = READ_POLYGONS;
                linesReaded = 0;
                }

            else if (line.find("WEIGHT:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2)
                    return nullptr;
                String maxonString = strData[0].c_str();
                if (objectData->weightName.Append(maxonString) == nullptr)
                    return nullptr;
                toRead = READ_WEIGHT;
                linesReaded = 0;
                }
            else if (line.find("UV:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 3)
                    return nullptr;
                String maxonStringName = strData[1].c_str();
                String maxonStringUvCount = strData[2].c_str();

                struct_uvInfo buffer_struct;
                buffer_struct.uvName = maxonStringName;
                buffer_struct.uvCount = maxonStringUvCount.ToInt32();

                if(objectData->uvInfo.Append(buffer_struct) == nullptr)
                    return nullptr;
                toRead = READ_UV;
                linesReaded = 0;
                }
            else if (line.find("MORPH:") == 0)
                {
                std::vector<std::string> strData = this->split(line, ":");
                if (strData.size() != 2)
                    return nullptr;
                String maxonString = strData[1].c_str();
                if(objectData->morphName.Append(maxonString) == nullptr)
                    return nullptr;
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
            if (strData.size() != 3)
                return nullptr;

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
            if(objectData->vertexData.Append(vertexData) == nullptr)
                return nullptr;
            
            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead -1)
                toRead = READ_NONE;

            break;
            }

        case READ_POLYGONS:
            {
            //Split the line and check if it's correctly formatted
            std::vector<std::string> strData = this->split(line, ";;");
            if (strData.size() != 3)
                return nullptr;

            struct_polygonData polygonData;
            //Get polygon Data
            std::vector<std::string> PolyIdstrData = this->split(line, ",");
            maxon::BaseArray<Int32> int32_pt_id;

            //Convert them from std::string to Int32
            for (std::string i : PolyIdstrData)
            {
                String buffer_String_pt_id = i.c_str();
                Int32 buffer_int32 = buffer_String_pt_id.ToInt32();
                if(int32_pt_id.Append(buffer_int32) == nullptr)
                    return nullptr;
             }

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

            //fill our struct
            if(polygonData.pts_id.CopyFrom(int32_pt_id) == maxon::FAILED)
                return nullptr;
            polygonData.material_name = strData[1].c_str();
            polygonData.type = faceData;

            //append data to our list
            if(objectData->polygonData.Append(polygonData) == nullptr)
                return nullptr;

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead - 1)
                toRead = READ_NONE;

            break;
            }

        // read Weight
        case READ_WEIGHT:
            {
            //no split needed so we can directly cast it into c4d type
            String maxonStringData = line.c_str();

            //append data to our list
            if(objectData->weightData.Append(maxonStringData.ToFloat()) == nullptr)
                return nullptr;

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead - 1)
                toRead = READ_NONE;

            break;
            }

        case READ_UV:
            {
            //Split the line and check if it's correctly formatted
            std::vector<std::string> strData = this->split(line, ":");
            if (strData.size() != 3 || strData.size() != 5)
                return nullptr;

            struct_uvData uvData;

            //Get uv coordinate
            std::vector<std::string> uvStrData = this->split(strData[0], " ");
            if (uvStrData.size() != 2)
                return nullptr;
                
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
            if(objectData->uvData.Append(uvData) == nullptr)
                return nullptr;

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead - 1)
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
            if (strData.size() != 3)
                return nullptr;

            //translate into c4d
            String maxonStringx = strData[0].c_str();
            String maxonStringy = strData[1].c_str();
            String maxonStringz = strData[2].c_str();

            //fill our struct
            morphData.delta_x = maxonStringx.ToFloat();
            morphData.delta_y = maxonStringy.ToFloat();
            morphData.delta_z = maxonStringz.ToFloat();

            //append data to our list
            if(objectData->morphData.Append(morphData) == nullptr)
                return nullptr;

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead - 1)
                toRead = READ_NONE;

            break;
            }
        }
    }
   return nullptr;
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