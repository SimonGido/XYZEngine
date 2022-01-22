using XYZ;

using System.IO;
using System.Threading.Tasks;

namespace Example
{
    public class Spawner : Entity
    {
        Texture2D texture;
        SubTexture subTexture;
        Shader shader;
        Material material;

        Entity entity;

        public float Speed = 2.0f;
        public Vector3 Velocity;

        public void OnCreate()
        {
            texture = new Texture2D("Assets/Textures/scarychar.png");
            subTexture = new SubTexture(texture);
            shader = new Shader("Resources/Shaders/DefaultShader.glsl");
            material = new Material(shader);

            entity = Create();
            SpriteRenderer renderer = entity.CreateComponent<SpriteRenderer>();
            renderer.Material = material;
            renderer.Sprite = subTexture;
            renderer.Color = new Vector4(1.0f, 0.0f, 1.0f, 1.0f);
        }

        public void OnDestroy()
        {
            texture.Destroy();
            subTexture.Destroy();
            shader.Destroy();
            material.Destroy();
        }

        public void OnUpdate(float ts)
        {
            TransformComponent transform = GetComponent<TransformComponent>();
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
        }
    }
}