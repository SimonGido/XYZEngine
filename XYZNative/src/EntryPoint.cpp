
#include <XYZ.h>

#include <cr.h>

#define EXPORT extern "C" __declspec(dllexport)

using namespace XYZ;

EXPORT int on_load()
{
    return 0;
}

EXPORT int on_unload()
{
    return 0;
}

EXPORT int on_update(float dt)
{
    return 0;
}

EXPORT int on_close()
{
    return 0;
}

EXPORT int EntryPoint(struct cr_plugin* ctx, enum cr_op operation, float dt)
{
    assert(ctx);
    switch (operation) 
    {
    case CR_LOAD:   return on_load(); // loading back from a reload
    case CR_UNLOAD: return on_unload(); // preparing to a new reload
    case CR_CLOSE:  return on_close();
    }
    // CR_STEP
    return on_update(dt);
}