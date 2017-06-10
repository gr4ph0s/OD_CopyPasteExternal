#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <ios>

#include "../main.h"
#include "../../DataStruct.h"


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

Int32 GetLineCount(std::ifstream* file){
    // new lines will be skipped unless we stop it from happening:    
    file->unsetf(std::ios_base::skipws);

    // count the newlines with an algorithm specialized for counting:
    unsigned line_count = std::count(
        std::istream_iterator<char>(*file),
        std::istream_iterator<char>(),'\n');

    file->clear();
    file->seekg(0);

    return line_count;
  }

std::string GetPathTempFile(){
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
    return folder;
}

void ReadVerticesHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    //get the number of vertices
    std::vector<std::string> strData = split(line, ":");
    if (strData.size() != 2){
        *Error = true;
        if (debug)
            GePrint("VERTICE HEADER - Bad formating");
        return;
        }

    //parse it in Int32 and fill data
    String maxonString = strData[1].c_str();
    objData->vertexCount = maxonString.ToInt32();


    //Set read mode to Vertices
    *linesToRead = maxonString.ToInt32();
    *toRead = READ_VERTICES;
    *linesReaded = 0;
    return;
}

void ReadPolygonsHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    std::vector<std::string> strData = split(line, ":");
    if (strData.size() != 2){
        *Error = true;
        if (debug)
            GePrint("POLY HEADER - Bad formating");
        return;
        }

    String maxonString = strData[1].c_str();    
    objData->polyCount = maxonString.ToInt32();

    *linesToRead = maxonString.ToInt32();
    *toRead = READ_POLYGONS;
    *linesReaded = 0;
}

void ReadWeightHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    std::vector<std::string> strData = split(line, ":");

    //Check the size
    if (strData.size() != 2){
        *Error = true;
        if (debug)
            GePrint("WEIGHT HEADER - Bad formating");
        return;
        }

    String maxonString = strData[0].c_str();
    if (objData->weightName.Append(maxonString) == nullptr){
        *Error = true;
        if (debug)
            GePrint("WEIGHT HEADER - FAIL INSERT");
        return;
        }

    *linesToRead = objData->vertexCount;
    *toRead = READ_WEIGHT;
    *linesReaded = 0;
}

void ReadUVHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    std::vector<std::string> strData = split(line, ":");
    if (strData.size() != 3){
        *Error = true;
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
        *Error = true;
        if (debug)
            GePrint("UV HEADER - FAIL INSERT");
        return;
        }

    *linesToRead = maxonStringUvCount.ToInt32();
    *toRead = READ_UV;
    *linesReaded = 0;
}

void ReadMorphHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    std::vector<std::string> strData = split(line, ":");
    if (strData.size() != 2){
        *Error = true;
        if (debug)
            GePrint("MORPH HEADER - BAD HEADER");
        return;
        }
    String maxonString = strData[1].c_str();
    if (objData->morphName.Append(maxonString) == nullptr){
        *Error = true;
        if (debug)
            GePrint("MORPH HEADER - FAIL INSERT");
        return;
        }

    *linesToRead = objData->vertexCount;
    *toRead = READ_WEIGHT;
    *linesReaded = 0;
}

void ReadVertices(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    //Get Data in std type
    std::vector<std::string> strData = split(line, " ");
    if (strData.size() != 3){
        *Error = true;
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
        *Error = true;
        if (debug)
            GePrint("VERTICE - FAIL INSERT - 01");
        return;
        }

    //check if we still have something to read
    *linesReaded = *linesReaded+1;
    if (*linesReaded == *linesToRead)
        *toRead = READ_NONE;
}

void ReadPolygons(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    //Split the line and check if it's correctly formatted
    std::vector<std::string> strData = split(line, ";;");
    if (strData.size() != 3){
        *Error = true;
        if (debug)
            GePrint("FACE - BAD FORMATTING");
        return;
        }

    objData->polygonData.Append();
    Int32 last_id = objData->polygonData.GetCount() - 1;
    if (last_id < 0){
        *Error = true;
        if (debug)
            GePrint("FACE - BAD Last_ID");
        return;
        }
    objData->polygonData[last_id] = NewObjClear(struct_polygonData);


    //Get polygon Data
    std::vector<std::string> PolyIdstrData = split(line, ",");

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
            *Error = true;
            if (debug)
                GePrint("FACE - FAIL INSERT - 02");
            return;
            }
        }

    //fill our struct
    objData->polygonData[last_id]->material_name = strData[1].c_str();
    objData->polygonData[last_id]->type = faceData;

    //check if we still have something to read
    *linesReaded = *linesReaded + 1;
    if (*linesReaded == *linesToRead)
        *toRead = READ_NONE;
}

void ReadWeights(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    //no split needed so we can directly cast it into c4d type
    String maxonStringData = line.c_str();

    //append data to our list
    if (objData->weightData.Append(maxonStringData.ToFloat()) == nullptr){
        *Error = true;
        if (debug)
            GePrint("WEIGHT - FAIL INSERT");
        return;
        }

    //check if we still have something to read
    *linesReaded = *linesReaded + 1;
    if (*linesReaded == *linesToRead)
        *toRead = READ_NONE;
}

void ReadUVs(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
    //Split the line and check if it's correctly formatted
    std::vector<std::string> strData = split(line, ":");
    if (strData.size() != 3 && strData.size() != 5){
        *Error = true;
        if (debug)
            GePrint("UV - BAD FORMATTING - 01");
        return;
        }

    struct_uvData uvData;

    //Get uv coordinate
    std::vector<std::string> uvStrData = split(strData[0], " ");
    if (uvStrData.size() != 2){
        *Error = true;
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
    if (strData.size() == 5){
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
    uvData.v = maxonStringV.ToFloat();
    uvData.pt_id = maxonStringPt_id.ToInt32();
    uvData.poly_id = maxonStringPoly_id.ToInt32();

    //append data to our list
    if (objData->uvData.Append(uvData) == nullptr){
        *Error = true;
        if (debug)
            GePrint("UV - FAIL INSERT");
        return;
        }

    //check if we still have something to read
    *linesReaded = *linesReaded + 1;
    if (*linesReaded == *linesToRead)
        *toRead = READ_NONE;
}

void ReadMorphs(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error){
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
    std::vector<std::string> strData = split(line, " ");
    if (strData.size() != 3){
        *Error = true;
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
        *Error = true;
        if (debug)
            GePrint("MORPH - FAIL INSERT");
        return;
        }

    //check if we still have something to read
    *linesReaded = *linesReaded + 1;
    if (*linesReaded == *linesToRead)
        *toRead = READ_NONE;
}