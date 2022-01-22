using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class TransformComponent : Component
    {
        public Matrix4 Transform
        {
            get
            {
                Matrix4 result;
                GetTransform_Native(Entity.ID, out result);
                return result;
            }
            set
            {
                SetTransform_Native(Entity.ID, ref value);
            }
        }
        public Vector3 Translation
        {
            get
            {
                Vector3 result;
                GetTranslation_Native(Entity.ID, out result);
                return result;
            }
            set
            {
                SetTranslation_Native(Entity.ID, ref value);
            }
        }
        public Vector3 Rotation
        {
            get
            {
                Vector3 result;
                GetRotation_Native(Entity.ID, out result);
                return result;
            }
            set
            {
                SetRotation_Native(Entity.ID, ref value);
            }
        }
        public Vector3 Scale
        {
            get
            {
                Vector3 result;
                GetScale_Native(Entity.ID, out result);
                return result;
            }
            set
            {
                SetScale_Native(Entity.ID, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTransform_Native(uint entityID, out Matrix4 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTransform_Native(uint entityID, ref Matrix4 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTranslation_Native(uint entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTranslation_Native(uint entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetRotation_Native(uint entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetRotation_Native(uint entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetScale_Native(uint entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetScale_Native(uint entityID, ref Vector3 result);
    }
}
