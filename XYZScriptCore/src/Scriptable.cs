using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XYZ
{
    interface Scriptable
    {
    void OnCreate();
    void OnUpdate(float ts);
    void OnDestroy();
    }
}
