using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XYZ
{
    public class Core
    {
        public static void CollectGarbage()
        {
            GC.Collect();
        }
    }
}
