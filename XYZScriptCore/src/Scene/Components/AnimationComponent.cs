using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


using System.Runtime.CompilerServices;


namespace XYZ
{
    public class AnimationComponent : Component
    {
        public AnimationController Controller
        {
            get
            {
                return new AnimationController(GetController_Native(Entity.ID));
            }
            set
            {
                SetController_Native(Entity.ID, value.m_UnmanagedInstance);
            }
        }

        public uint[] BoneEntities
        {
            get
            {
                return GetBoneEntities_Native(Entity.ID);
            }
            set
            {
                SetBoneEntities_Native(Entity.ID, value);
            }
        }

        public float AnimationTime
        {
            get { return GetAnimationTime_Native(Entity.ID); }
            set { SetAnimationTime_Native(Entity.ID, value); }
        }
        public bool Playing
        {
            get { return GetPlaying_Native(Entity.ID); }
            set { SetPlaying_Native(Entity.ID, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr GetController_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint[] GetBoneEntities_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float GetAnimationTime_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool GetPlaying_Native(uint entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetController_Native(uint entityID, IntPtr controllerInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr SetBoneEntities_Native(uint entityID, uint[] entities);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr SetAnimationTime_Native(uint entityID, float time);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr SetPlaying_Native(uint entityID, bool play);
    }
}
