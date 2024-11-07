#pragma once
#include "CAudioNode.h"
#include <CNote.h>
class CInstrument :
    public CAudioNode
{
public:
    virtual void SetNote(CNote* note) = 0;
};

