using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class AnimatedMesh : UnmanagedResource
    {
        public AnimatedMesh()
        {
        }
        public AnimatedMesh(MeshSource meshSource)
        {
            m_UnmanagedInstance = Constructor_Native(meshSource.m_UnmanagedInstance);
        }
        public override void Destroy()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        public void CreateBones(Entity parent)
        {
            CreateBones_Native(m_UnmanagedInstance, parent.ID);
        }

        internal AnimatedMesh(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(IntPtr meshSourceInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr CreateBones_Native(IntPtr meshSourceInstance, uint parent);
    }
}
