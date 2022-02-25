using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class AnimationAsset : UnmanagedResource
    {
        public AnimationAsset()
        {

        }
        public AnimationAsset(string filename, string animationName, SkeletonAsset skeletonAsset)
        {
            m_UnmanagedInstance = Constructor_Native(filename, animationName, skeletonAsset.m_UnmanagedInstance);
        }

        internal AnimationAsset(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }
  

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(string filename, string animationName, IntPtr skeletonAssetInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }
}
