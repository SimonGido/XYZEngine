using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Texture2D
    {
        public Texture2D(uint width, uint height)
        {
            m_UnmanagedInstance = Constructor_Native(width, height);
        }
        public Texture2D(string path)
        {
            m_UnmanagedInstance = ConstructorPath_Native(ref path);
        }
        ~Texture2D()
        {
            Destructor_Native(m_UnmanagedInstance);
        }

        public void SetData(Vector4[] data)
        {
            SetData_Native(m_UnmanagedInstance, data, data.Length);
        }

        internal Texture2D(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        internal IntPtr m_UnmanagedInstance;

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(uint width, uint height);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr ConstructorPath_Native(ref string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetData_Native(IntPtr unmanagedInstance, Vector4[] data, int size);
    }
}
