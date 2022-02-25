using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class SkeletonAsset : UnmanagedResource
    {
        public SkeletonAsset()
        {
        }
        public SkeletonAsset(string path)
        {
            m_UnmanagedInstance = Constructor_Native(path);
        }

        ~SkeletonAsset()
        {
            Destructor_Native(m_UnmanagedInstance);
        }
   

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }
}
