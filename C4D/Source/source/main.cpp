#include "c4d.h"
#include <string.h>
#include "main.h"


Bool PluginStart(void)
{
    if (!RegisterPasteFromExternal())
        return false;

    if (!RegisterExportToExternal())
        return false;
	return true;
}

void PluginEnd(void)
{
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!resource.Init())
				return false;

			return true;
			break;
	}

	return false;
}
