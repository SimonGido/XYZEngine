using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace XYZ
{
    public abstract class Component
    {
        public Entity Entity { get; set; }

    }

    public class SceneTagComponent : Component
    {
        public string Tag
        {
            get
            {
                return GetTag_Native(Entity.ID);
            }
            set
            {
                SetTag_Native(value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTag_Native(string tag);

    }

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
        public static extern void GetTransform_Native(uint entityID, out Matrix4 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTransform_Native(uint entityID, ref Matrix4 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetTranslation_Native(uint entityID, out Vector3 result);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTranslation_Native(uint entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetRotation_Native(uint entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetRotation_Native(uint entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetScale_Native(uint entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetScale_Native(uint entityID, ref Vector3 result);
    }

    public class CameraComponent : Component
    {
        // TODO
    }

    public class ScriptComponent : Component
    {
        // TODO
    }

    public class SpriteRenderer : Component
    {
        // TODO
    }

    public class AnimatorComponent : Component
    {
        // TODO
    }
    public class ParticleComponentGPU : Component
    {
        // TODO
    }
    public class Relationship : Component
    {
        // TODO
    }
    public class PointLight2D : Component
    {
        // TODO
    }
    public class RigidBody2DComponent : Component
    {
        public void ApplyForce(Vector2 impulse, Vector2 point)
        {
            ApplyForce_Native(Entity.ID, ref impulse, ref point);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ApplyForce_Native(uint entityID, ref Vector2 impulse, ref Vector2 point);
    }

    public class BoxCollider2DComponent : Component
    {
    }
    public class CircleCollider2DComponent : Component
    {
    }
    public class PolygonCollider2DComponent : Component
    {
    }
}