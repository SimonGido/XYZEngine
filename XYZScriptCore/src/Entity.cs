using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Entity
    {
        public uint ID { get; private set; }

        internal Entity(uint id)
        {
            ID = id;
        }
        protected Entity() { ID = 0; }
        ~Entity()
        {
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(ID, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(ID, typeof(T));
        }

        public void RemoveComponent<T>() where T : Component, new ()
        {
            RemoveComponent_Native(ID, typeof(T));
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            return null;
        }

        public static Entity Create()
        {
            return new Entity(Create_Native());
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(uint entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool RemoveComponent_Native(uint entityID, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern uint Create_Native();
    }
}
