#include "stdafx.h"
#include "ECSSerializer.h"


namespace XYZ {
    void ECSSerializer::SerializeRaw(const ECSManager& ecs, ByteStream& out)
    {
        out << ecs.m_EntityManager.m_Bitset.GetNumberOfSignatures();
        size_t offset = 0;
        for (bool created : ecs.m_ComponentManager.m_StorageCreated)
        {
            if (created)
            {
                const IComponentStorage* storage = ecs.m_ComponentManager.GetIStorage(offset);
                out << storage->ID();
                out << storage->Size();
                for (size_t i = 0; i < storage->Size(); ++i)
                {
                    Entity entity = storage->GetEntityAtIndex(i);
                    out << entity;
                    storage->CopyComponentData(entity, out);
                }
            }
            offset += sizeof(ComponentStorage<IComponent>);
        }
    }
    void ECSSerializer::DeserializeRaw(ECSManager& ecs, const ByteStream& in)
    {
        size_t maxEntity = 0;
        in >> maxEntity;
        ecs.Clear();
        while (!in.End())
        {
            uint16_t componentID = 0;
            size_t size = 0;
            in >> componentID;
            in >> size;
            for (size_t i = 0; i < size; ++i)
            {
                IComponentStorage& storage = ecs.GetIStorage((size_t)componentID);
                Entity entity;
                in >> entity;
                storage.AddRawComponent(entity, in);
            }
        }
    }
}