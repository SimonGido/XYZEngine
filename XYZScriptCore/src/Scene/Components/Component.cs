using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace XYZ
{
    public abstract class Component
    {
        public Entity Entity { get; set; }

    }

   


    public class CameraComponent : Component
    {
        // TODO
    }

    public class ScriptComponent : Component
    {
        // TODO
    }

   

    public class AnimatorComponent : Component
    {
        // TODO
    }
    public class ParticleComponent : Component
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