using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class AnimationController : UnmanagedResource
    {
        public AnimationController()
        {
            m_UnmanagedInstance = Constructor_Native();
        }

        ~AnimationController()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        public void SetSkeletonAsset(SkeletonAsset skeleton)
        {
            SetSkeletonAsset_Native(m_UnmanagedInstance, skeleton.m_UnmanagedInstance);
        }
        public void SetState(string name)
        {
            SetState_Native(m_UnmanagedInstance, name);
        }

        public void AddState(string name, AnimationAsset animation)
        {
            AddState_Native(m_UnmanagedInstance, name, animation.m_UnmanagedInstance);
        }

        internal AnimationController(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetSkeletonAsset_Native(IntPtr unmanangedInstance, IntPtr skeletonInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetState_Native(IntPtr unmanangedInstance, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AddState_Native(IntPtr unmanangedInstance, string name, IntPtr animationInstance);
    }
}
