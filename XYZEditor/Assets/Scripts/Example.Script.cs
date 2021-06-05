using System;

using XYZ;

namespace Example
{
    public class Script : Entity
    {
        public string Test = "Pes";
        public float VerticalSpeed = 5.0f;
        public float Speed = 2.0f;
        public float Rotation = 0.0f;
        public Vector3 Velocity;
        RigidBody2DComponent RigidBody;

        public void OnCreate()
        {
            //RigidBody = GetComponent<RigidBody2DComponent>();
        }

        public void OnDestroy()
        {

        }

        public void OnUpdate(float ts)
        {
            Matrix4 transform = GetTransform();
            Vector3 translation = transform.Translation;
         
            float speed = Speed * ts;
            if (XYZ.Input.IsKeyPressed(KeyCode.KEY_LEFT))
            {
                //Vector2 impulse = new Vector2(0.0f, 1.0f);
                //Vector2 point = new Vector2(translation.X, translation.Y);
                //RigidBody.ApplyForce(impulse, point);
                if (HasComponent<CameraComponent>())
                {
                    translation.X = -1000.0f;
                }
                translation.X -= speed;
            }
            else if (XYZ.Input.IsKeyPressed(KeyCode.KEY_RIGHT))
            {
                translation.X += speed;
            }

            if (XYZ.Input.IsKeyPressed(KeyCode.KEY_UP))
            {
                translation.Y += speed;
            }
            else if (XYZ.Input.IsKeyPressed(KeyCode.KEY_DOWN))
            {
                translation.Y -= speed;
            }
            transform.Translation = translation;
            SetTransform(transform);        
        }
    }
}
