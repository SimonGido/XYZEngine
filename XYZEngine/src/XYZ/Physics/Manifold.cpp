#include "stdafx.h"
#include "Manifold.h"

#include "Collision.h"
#include "XYZ/Utils/Math/Math.h"

namespace XYZ {

    void Manifold::PreStep()
    {
        Collision::AABBvsAABB(*this);
        PositionalCorrection();
    }
    void Manifold::Resolve()
    {
        glm::vec2 rv = B->m_Velocity - A->m_Velocity;

        float velAlongNormal = glm::dot(rv, Normal);

        if (velAlongNormal > 0)
            return;

        float minRest = std::min(A->m_Restitution, B->m_Restitution);
        float impulseScalar = -(1.0f + minRest) * velAlongNormal;
        impulseScalar /= 1.0f / A->m_Mass + 1.0f / B->m_Mass;
        glm::vec2 impulse = impulseScalar * Normal;
        A->m_Velocity -= 1.0f / A->m_Mass * impulse;
        B->m_Velocity += 1.0f / B->m_Mass * impulse;


        // Friction
        rv = B->m_Velocity - A->m_Velocity;
        glm::vec2 tangent = rv - glm::dot(rv, Normal) * Normal;
        tangent = glm::normalize(tangent);

        float jt = -glm::dot(rv, tangent);
        if (std::isnan(jt))
            return;
        jt = jt / (1.0f / A->m_Mass + 1.0f / B->m_Mass);

        float mu = Math::PythagoreanSolve(A->m_StaticFriction, B->m_StaticFriction);

        glm::vec2 frictionImpulse;
        if (abs(jt) < jt * mu)
        {
            frictionImpulse = jt * tangent;
        }
        else
        {
            float dynamicFriction = Math::PythagoreanSolve(A->m_DynamicFriction, B->m_DynamicFriction);
            frictionImpulse = -jt * tangent * dynamicFriction;
        }
        A->m_Velocity -= (1.0f / A->m_Mass) * frictionImpulse;
        B->m_Velocity += (1.0f / B->m_Mass) * frictionImpulse;
    }
    void Manifold::Solve()
    {
    }
    void Manifold::Initialize(const glm::vec2& gravity, float dt)
    {
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

            float contactVel = glm::dot(rv, Normal);

            // Do not resolve if velocities are separating
            if (contactVel > 0)
                return;

            float raCrossN = Math::Cross(ra, Normal);
            float rbCrossN = Math::Cross(rb, Normal);
            float invMassSum = A->m_InverseMass + B->m_InverseMass + sqrt(raCrossN) * A->m_InverseInertia + sqrt(rbCrossN) * B->m_InverseInertia;

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

            glm::vec2 t = rv - (Normal * glm::dot(rv, Normal));
            t = glm::normalize(t);

            // j tangent magnitude
            float jt = -glm::dot(rv, t);
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
    }
}