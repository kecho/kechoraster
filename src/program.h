#ifndef __PROGRAM_INTERFACE__
#define __PROGRAM_INTERFACE__
#include "framebuffer.h"

class ProgramInterface
{
public:
    ProgramInterface(){}
    virtual ~ProgramInterface(){}
    virtual void OnStart() = 0;
    virtual void OnEnd() = 0;
    virtual void Render() = 0;
    virtual const char * GetName() = 0;
    virtual kechorender::core::FrameBuffer * GetFrame() = 0;
    static ProgramInterface * create();
};
#endif
