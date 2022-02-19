#include "stdafx.h"
#include "Mesh.h"



namespace XYZ {
   
    Mesh::Mesh(const Ref<MeshSource>& meshSource)
        :
        m_MeshSource(meshSource)
    {
    }

    Mesh::~Mesh()
    {
    }

    AnimatedMesh::AnimatedMesh(const Ref<MeshSource>& meshSource)
        :
        m_MeshSource(meshSource)
    {
    }
}