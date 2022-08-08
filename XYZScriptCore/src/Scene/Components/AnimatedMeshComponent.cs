using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class AnimatedMeshComponent : Component
    {

        public AnimatedMesh Mesh
        {
            get 
            {
                return new AnimatedMesh(GetAnimatedMesh_Native(Entity.ID));
            }
            set 
            {
                SetAnimatedMesh_Native(Entity.ID, value.m_UnmanagedInstance);
            }
        }
        public MaterialAsset MaterialAsset
        {
            get
            {
                return new MaterialAsset(GetMaterialAsset_Native(Entity.ID));
            }
            set
            {
                SetMaterialAsset_Native(Entity.ID, value.m_UnmanagedInstance);
            }
        }

        public uint[] BoneEntities
        {
            get
            {        
                return GetBoneEntities_Native(Entity.ID);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetAnimatedMesh_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr SetAnimatedMesh_Native(uint entityID, IntPtr meshInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint[] GetBoneEntities_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetMaterialAsset_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr SetMaterialAsset_Native(uint entityID, IntPtr matInstance);

    }
}
