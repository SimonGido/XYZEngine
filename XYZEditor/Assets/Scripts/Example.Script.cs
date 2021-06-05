using System;

using XYZ;

namespace Example
{
    public class Script : Entity
    {
        public string Test = "Pes";
        public float VerticalSpeed = 5.0f;
        public float ImpulseStrength = 0.5f;
        public float Rotation = 0.0f;
        public Vector3 Velocity;
        RigidBody2DComponent RigidBody;

        public void OnCreate()
        {
            RigidBody = GetComponent<RigidBody2DComponent>();
        }

        public void OnDestroy()
        {

        }

        public void OnUpdate(float ts)
        {
            Matrix4 transform = GetTransform();
            Vector3 translation = transform.Translation;
            Vector2 point = new Vector2(translation.X, translation.Y);

            float impulse = ImpulseStrength * ts;
            if (XYZ.Input.IsKeyPressed(KeyCode.KEY_LEFT))
            {            
                RigidBody.ApplyForce(new Vector2(-impulse, 0.0f), point);
            }
            else if (XYZ.Input.IsKeyPressed(KeyCode.KEY_RIGHT))
            {
                RigidBody.ApplyForce(new Vector2(impulse, 0.0f), point);
            }

            if (XYZ.Input.IsKeyPressed(KeyCode.KEY_UP))
            {
                RigidBody.ApplyForce(new Vector2(0.0f, impulse * 100.0f), point);
            }
            else if (XYZ.Input.IsKeyPressed(KeyCode.KEY_DOWN))
            {
                RigidBody.ApplyForce(new Vector2(0.0f, -impulse), point);
            }       
        }
    }
}
