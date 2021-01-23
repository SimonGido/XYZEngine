using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Input
    {
        public static bool IsKeyPressed(KeyCode keycode)
        {
            return IsKeyPressed_Native(keycode);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed_Native(KeyCode keycode);
    }
}
