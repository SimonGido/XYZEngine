using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Shader : UnmanagedResource
    {
        internal IntPtr m_UnmanagedInstance;

        public Shader(string path)
        {
            m_UnmanagedInstance = Constructor_Native(path);
        }
        public override void Destroy()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }
}
