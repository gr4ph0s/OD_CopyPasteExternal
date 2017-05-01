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
    iobject* objectData = new iobject();

    //Read data
    std::ifstream file(folder);
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
                std::string strData = line.substr(line.find(":") + 1);

                //parse it in Int32 and fill data
                String maxonString = strData.c_str();
                linesToRead = maxonString.ToInt32();
                objectData->vertexCount = linesToRead;

                //Set read mode to Vertices
                toRead = READ_VERTICES;
                linesReaded = 0;
                }

            else if (line.find("POLYGONS:") == 0)
                {
                std::string strData = line.substr(line.find(":") + 1);
                String maxonString = strData.c_str();
                linesToRead = maxonString.ToInt32();
                objectData->polyCount = linesToRead;

                toRead = READ_POLYGONS;
                linesReaded = 0;
                }

            else if (line.find("WEIGHT:") == 0)
                {
                std::string strData = line.substr(line.find(":") + 1);
                String maxonString = strData.c_str();
                objectData->weightName.push_back(maxonString);
                toRead = READ_WEIGHT;
                linesReaded = 0;
                }
            else if (line.find("UV:") == 0)
                {
                size_t last = line.find_last_of(":");
                size_t first = line.find_first_of(":");
                std::string strDataName = line.substr(first + 1, last - first);
                std::string strDataUvCount = line.substr(last + 1);
                String maxonStringName = strDataName.c_str();
                String maxonStringUvCount = strDataUvCount.c_str();

                struct_uvInfo buffer_struct;
                buffer_struct.uvName = maxonStringName;
                buffer_struct.uvCount = maxonStringUvCount.ToInt32();

                objectData->uvInfo.push_back(buffer_struct);
                toRead = READ_UV;
                linesReaded = 0;
                }
            else if (line.find("MORPH:") == 0)
                {
                std::string strData = line.substr(line.find(":") + 1);
                String maxonString = strData.c_str();
                objectData->morphName.push_back(maxonString);
                toRead = READ_WEIGHT;
                linesReaded = 0;
                }
    
            break;
            }

        // read vertices data
        case READ_VERTICES:
            {
            //Get Data in std type
            size_t last = line.find_last_of(" ");
            size_t first = line.find_first_of(" ");
            std::string strx = line.substr(0, first);
            std::string stry = line.substr(first + 1, last - first);
            std::string strz = line.substr(last + 1);

            //translate into c4d
            String maxonStringx = strx.c_str();
            String maxonStringy = stry.c_str();
            String maxonStringz = strz.c_str();

            //fill our struct
            struct_vertexData vertexData;
            vertexData.x = maxonStringx.ToFloat();
            vertexData.y = maxonStringy.ToFloat();
            vertexData.z = maxonStringz.ToFloat();

            //append data to our list
            objectData->vertexData.Insert(objectData->vertexData.end, vertexData);
            
            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead -1)
                toRead = READ_NONE;

            break;
            }

        case READ_POLYGONS:
            {

            }
            break;

        // read Weight
        case READ_WEIGHT:
            {
            //no split needed so we can directly cast it into c4d type
            String maxonStringData = line.c_str();

            //append data to our list
            objectData->weightData.push_back(maxonStringData.ToFloat());

            //check if we still have something to read
            linesReaded++;
            if (linesReaded == linesToRead - 1)
                toRead = READ_NONE;

            break;
            }

        case READ_UV:
            {

            }
            break;

        //read morph
        case READ_MORPH:
            {
            //Get Data in std type
            size_t last = line.find_last_of(" ");
            size_t first = line.find_first_of(" ");
            std::string strx = line.substr(0, first);
            std::string stry = line.substr(first + 1, last - first);
            std::string strz = line.substr(last + 1);

            //translate into c4d
            String maxonStringx = strx.c_str();
            String maxonStringy = stry.c_str();
            String maxonStringz = strz.c_str();

            //fill our struct
            struct_morphData morphData;
            morphData.delta_x = maxonStringx.ToFloat();
            morphData.delta_y = maxonStringy.ToFloat();
            morphData.delta_z = maxonStringz.ToFloat();

            //append data to our list
            objectData->morphData.push_back(morphData);

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