#ifndef C4D_PASTE_FROM_EXTERNAL_H__
#define C4D_PASTE_FROM_EXTERNAL_H__

#include "c4d.h"
#include "../../DataStruct.h"

#define ID_C4D_PASTE_FROM_EXTERNAL 1039180


class PasteFromExternal : public CommandData
{
public:
    //PasteFromExternal();
    //virtual ~PasteFromExternal();

    virtual Bool Execute(BaseDocument *doc);

    std::vector<std::string> split(const std::string& str, const std::string& delim);
    void ParseFileToIobject(iobject* objData);
    Bool PasteFromExternal::CreatePolyObj(const iobject* objData, BaseDocument *doc);


    //virtual Bool Execute(BaseDocument* doc);

};
#endif