#include "stdafx.h"
#include "Manifold.h"

#include "Collision.h"
#include "XYZ/Utils/Math/Math.h"


namespace XYZ {


    void Manifold::Solve()
    {
        Collision::AABBvsAABB(*this);
    }
    void Manifold::Initialize(const glm::vec2& gravity, float dt)
    {
        Collision::AABBvsAABB(*this);
        Restitution = std::min(A->m_Restitution, B->m_Restitution);
        StaticFriction = std::sqrt(A->m_StaticFriction * A->m_StaticFriction);
        DynamicFriction = std::sqrt(A->m_DynamicFriction * A->m_DynamicFriction);

        for (uint32_t i = 0; i < ContactCount; ++i)
        {
            // Calculate radii from COM to contact
            glm::vec2 ra = Contacts[i] - A->m_Position;
            glm::vec2 rb = Contacts[i] - B->m_Position;

            
            glm::vec2 rv = B->m_Velocity + Math::Cross(B->m_AngularVelocity, rb) -
                           A->m_Velocity - Math::Cross(A->m_AngularVelocity, ra);


            // Determine if we should perform a resting collision or not
            // The idea is if the only thing moving this object is gravity,
            // then the collision should be performed without any restitution
            glm::vec2 grav = gravity * dt;
            float lenSqr = rv.x * rv.x + rv.y * rv.y;
            float gravityLenSqr = grav.x * grav.x + grav.y * grav.y;
            if (lenSqr < gravityLenSqr + FLT_EPSILON)
                Restitution = 0.0f;
        }
    }
    void Manifold::ApplyImpulse()
    {
        for (uint32_t i = 0; i < ContactCount; ++i)
        {
            glm::vec2 ra = Contacts[i] - A->m_Position;
            glm::vec2 rb = Contacts[i] - B->m_Position;

            glm::vec2 rv = B->m_Velocity + Math::Cross(B->m_AngularVelocity, rb) -
                A->m_Velocity - Math::Cross(A->m_AngularVelocity, ra);

            float contactVel = Math::Dot(rv, Normal);

            // Do not resolve if velocities are separating
            if (contactVel > 0)
                return;

            float raCrossN = Math::Cross(ra, Normal);
            float rbCrossN = Math::Cross(rb, Normal);
            float sqrRacrossN = raCrossN * raCrossN;
            float sqrRbcrossN = rbCrossN * rbCrossN;
            float invMassSum = A->m_InverseMass + B->m_InverseMass + sqrRacrossN * A->m_InverseInertia + sqrRbcrossN * B->m_InverseInertia;

            // Calculate impulse scalar
            float j = -(1.0f + Restitution) * contactVel;
            j /= invMassSum;
            j /= (float)ContactCount;

            // Apply impulse
            glm::vec2 impulse = Normal * j;
            A->ApplyImpulse(-impulse, ra);
            B->ApplyImpulse(impulse, rb);

            // Friction impulse
            rv = B->m_Velocity + Math::Cross(B->m_AngularVelocity, rb) -
                A->m_Velocity - Math::Cross(A->m_AngularVelocity, ra);

            glm::vec2 t = rv - (Normal * Math::Dot(rv, Normal));
            Math::Normalize(t);

            // j tangent magnitude
            float jt = -Math::Dot(rv, t);
            jt /= invMassSum;
            jt /= (float)ContactCount;

            // Don't apply tiny friction impulses
            //if (Equal(jt, 0.0f))
            //    return;

            // Coulumb's law
            glm::vec2 tangentImpulse;
            if (std::abs(jt) < j * StaticFriction)
                tangentImpulse = t * jt;
            else
                tangentImpulse = t * -j * DynamicFriction;

            // Apply friction impulse
            A->ApplyImpulse(-tangentImpulse, ra);
            B->ApplyImpulse(tangentImpulse, rb);
        }
    }
    void Manifold::PositionalCorrection()
    {
        float slop = 0.05f; // Penetration allowance
        float percent = 0.4f; // Penetration percentage to correct
        glm::vec2 correction = (std::max(PenetrationDepth - slop, 0.0f) / (A->m_InverseMass + B->m_InverseMass)) * Normal * percent;
        A->m_Position -= correction * A->m_InverseMass;
        B->m_Position += correction * B->m_InverseMass;

        ContactCount = 0;
    }
}