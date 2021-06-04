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

        public void OnCreate()
        {
        }

        public void OnUpdate(float ts)
        {
            Matrix4 transform = GetTransform();
            Vector3 translation = transform.Translation;
         
            float speed = Speed * ts;
            if (XYZ.Input.IsKeyPressed(KeyCode.KEY_LEFT))
            {
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
