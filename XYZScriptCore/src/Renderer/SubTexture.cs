using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class SubTexture : UnmanagedResource
    {
        public SubTexture(Texture2D texture)
        {
            m_UnmanagedInstance = Constructor_Native(texture.m_UnmanagedInstance);
        }
        public SubTexture(Texture2D texture, ref Vector4 texCoords)
        {
            m_UnmanagedInstance = ConstructorTexCoords_Native(texture.m_UnmanagedInstance, ref texCoords);
        }
        ~SubTexture()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        public Texture2D GetTexture()
        {
            return new Texture2D(GetTexture_Native(m_UnmanagedInstance));
        }
        public Vector4 GetTexCoords()
        {
            Vector4 result;
            GetTexCoords_Native(m_UnmanagedInstance, out result);
            return result;
        }

        internal SubTexture(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(IntPtr textureInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Destructor_Native(IntPtr unmanagedInstance);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr ConstructorTexCoords_Native(IntPtr textureInstance, ref Vector4 texCoords);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetTexture_Native(IntPtr unmanagedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTexCoords_Native(IntPtr unmanagedInstance, out Vector4 texCoords);

    }
}
