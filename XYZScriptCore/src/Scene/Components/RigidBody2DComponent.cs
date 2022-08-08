using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class RigidBody2DComponent : Component
    {
        public void ApplyForce(Vector2 impulse, Vector2 point)
        {
            ApplyForce_Native(Entity.ID, ref impulse, ref point);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void ApplyForce_Native(uint entityID, ref Vector2 impulse, ref Vector2 point);
    }

}
