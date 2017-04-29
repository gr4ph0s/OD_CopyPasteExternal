#ifndef C4D_EXPORT_TO_EXTERNAL_H__
#define C4D_EXPORT_TO_EXTERNAL_H__

#include "c4d.h"

#define ID_C4D_EXPORT_TO_EXTERNAL 1039181

class ExportToExternal : public CommandData
{
public :
    ExportToExternal();
    virtual ~ExportToExternal();

};
#endif