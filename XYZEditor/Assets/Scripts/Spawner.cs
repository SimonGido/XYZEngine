using XYZ;

using System.IO;
using System.Threading.Tasks;
using System;

namespace Example
{
    public class Spawner : Entity
    {
        //Texture2D texture;
        //SubTexture subTexture;
        //ShaderAsset shader;
        // MaterialAsset material;
        AnimatedMesh mesh;
        AnimationController controller;
        AnimationAsset animation;
        SkeletonAsset skeleton;

        public float Speed = 2.0f;
        public Vector3 Velocity;
        public float Test = 4.0f;
        public void OnCreate()
        {
            Log.Info("OnCreate called");
            //texture = new Texture2D("Assets/Textures/scarychar.png");
            //subTexture = new SubTexture(texture);
            //shader = AssetManager.GetAsset<ShaderAsset>("Resources/Shaders/DefaultLitShader.shader");
            // material = AssetManager.GetAsset<MaterialAsset>("Resources/Materials/DefaultLit.mat");

            string characterRunning = "Resources/Meshes/Character Running.fbx";
            mesh = new AnimatedMesh(new MeshSource(characterRunning));
            uint[] bones = mesh.CreateBones(this);
           
            controller = new AnimationController();
            skeleton = new SkeletonAsset(characterRunning);
            animation = new AnimationAsset(characterRunning, "Armature|ArmatureAction", skeleton);
            controller.SetSkeletonAsset(skeleton);
            controller.AddState("Run", animation);

            AnimationController test = new AnimationController();

            AnimationComponent animationComponent = CreateComponent<AnimationComponent>();
            animationComponent.BoneEntities = bones;
            animationComponent.Controller = controller;
            animationComponent.Playing = true;
            //SpriteRenderer renderer = CreateComponent<SpriteRenderer>();
            //
            //renderer.MaterialAsset = material;
            //renderer.Sprite = subTexture;
            //renderer.Color = new Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        }

        public void OnDestroy()
        {
            Log.Info("OnDestroy called");
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