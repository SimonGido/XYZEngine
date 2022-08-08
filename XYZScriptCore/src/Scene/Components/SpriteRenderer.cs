using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class SpriteRenderer : Component
    {
        public SubTexture Sprite
        {
            get
            {
                return new SubTexture(GetSubTexture_Native(Entity.ID));
            }
            set
            {
                SetSubTexture_Native(Entity.ID, value.m_UnmanagedInstance);
            }
        }

        public Material Material
        {
            get
            {
                return new Material(GetMaterial_Native(Entity.ID));
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

        public Vector4 Color
        {
            get
            {
                Vector4 result;
                GetColor_Native(Entity.ID, out result);
                return result;
            }
            set
            {
                SetColor_Native(Entity.ID, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetSubTexture_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetSubTexture_Native(uint entityID, IntPtr subTexture);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetMaterial_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetMaterialAsset_Native(uint entityID, IntPtr material);
       
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetMaterialAsset_Native(uint entityID);
       
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetColor_Native(uint entityID, out Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetColor_Native(uint entityID, ref Vector4 color);

    }
}
