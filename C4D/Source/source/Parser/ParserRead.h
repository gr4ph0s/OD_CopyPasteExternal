#ifndef PARSER_READ_H__
#define PARSER_READ_H__

#include <string>

std::string GetPathTempFile();
Int32 GetLineCount(std::ifstream* file);

void ReadVerticesHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadPolygonsHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadWeightHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadUVHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadMorphHeader(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);

void ReadVertices(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadPolygons(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadWeights(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadUVs(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);
void ReadMorphs(iobject* objData, std::string line, Int32* linesToRead, ReadState* toRead, Int32* linesReaded, Bool* debug, Bool* Error);

#endif // PARSER_READ_H__
