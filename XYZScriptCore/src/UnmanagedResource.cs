﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XYZ
{
    public abstract class UnmanagedResource
    {
        internal IntPtr m_UnmanagedInstance;

        public abstract void Destroy();
    }
}
