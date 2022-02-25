using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace XYZ
{
    public class AssetManager
    {
        public static T GetAsset<T>(string path) where T : UnmanagedResource, new()
        {
            T asset = new T();
            asset.m_UnmanagedInstance = GetAsset_Native(path, typeof(T));
            return asset;
        }


        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetAsset_Native(string path, Type type);
    }
}
