using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class MaterialAsset : UnmanagedResource
    {
        public MaterialAsset() { }
        public MaterialAsset(ShaderAsset shaderAsset)
        {
            m_UnmanagedInstance = Constructor_Native(shaderAsset.m_UnmanagedInstance);
        }
   
        internal MaterialAsset(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }
        ~MaterialAsset()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        public void SetTexture(string name, Texture2D texture)
        {
            SetTexture_Native(name, texture.m_UnmanagedInstance);
        }

        public void SetTexture(string name, Texture2D texture, uint index)
        {
            SetTextureArr_Native(name, texture.m_UnmanagedInstance, index);
        }

        public Material Material
        {
            get 
            {
                return new Material(GetMaterial_Native(m_UnmanagedInstance));
            }
        }
        public MaterialInstance MaterialInstance
        {
            get
            {
                return new MaterialInstance(GetMaterialInstance_Native(m_UnmanagedInstance));
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(IntPtr shaderAssetInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTexture_Native(string name, IntPtr textureInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTextureArr_Native(string name, IntPtr textureInstance, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetMaterial_Native(IntPtr unmanagedInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetMaterialInstance_Native(IntPtr unmanagedInstance);
    }
}
