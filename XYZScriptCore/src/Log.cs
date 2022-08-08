using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Log
    {
        public static void Info(string message)
        {
            Info_Native(message);
        }
        public static void Warn(string message)
        {
            Warn_Native(message);
        }
        public static void Error(string message)
        {
            Error_Native(message);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Info_Native(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Warn_Native(string message);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Error_Native(string message);
    }
}
