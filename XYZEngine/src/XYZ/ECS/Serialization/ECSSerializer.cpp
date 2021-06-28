#include "stdafx.h"
#include "ECSSerializer.h"


namespace XYZ {
    void ECSSerializer::Serialize(const ECSManager& ecs, ByteStream& out, bool tight )
    {
        serializeECSHeaderAndData(ecs, out, tight);
        for (auto storage : ecs.m_ComponentManager.m_Storages)
        {
            if (storage)
            {
                out << storage->ID();
                out << storage->Size();
                for (size_t i = 0; i < storage->Size(); ++i)
                {
                    Entity entity = storage->GetEntityAtIndex(i);
                    out << entity;
                    storage->CopyComponentData(entity, out);
                }
            }
        }
    }
    void ECSSerializer::Deserialize(ECSManager& ecs, const ByteStream& in)
    {
        deserializeECSHeaderAndData(ecs, in);
        while (!in.End())
        {
            uint16_t componentID = 0;
            size_t size = 0;
            in >> componentID;
            in >> size;
            IComponentStorage* storage = ecs.GetIStorage(componentID);
            if (storage)
            {
                for (size_t i = 0; i < size; ++i)
                {
                    Entity entity;
                    in >> entity;
                    storage->AddRawComponent(entity, in);
                }
            }
        }
    }

    void ECSSerializer::SerializeComponent(const ECSManager& ecs, Entity entity, uint16_t componentID, ByteStream& out, bool writeInfo)
    {
        if (writeInfo)
        {
            out << componentID;
            out << (size_t)1;
        }
        const IComponentStorage* storage = ecs.GetIStorage(componentID);
        out << entity;
        storage->CopyComponentData(entity, out);
    }

    void ECSSerializer::serializeECSHeaderAndData(const ECSManager& ecs, ByteStream& out, bool tight)
    {
        uint32_t maxEntity = (uint32_t)ecs.m_EntityManager.m_Bitset.GetNumberOfSignatures();
        ECSHeader header{ maxEntity - 1, true, tight };
        out << header;

        if (header.EntitiesIncluded)
        {
            if (header.Tight)
            {
                uint32_t numBitsets = (uint32_t)std::ceil((double)header.NumEntities / (double)ECSHeader::TightPackLength);
                for (uint32_t i = 0; i < numBitsets; ++i)
                {
                    uint8_t bitset = 0;
                    for (uint32_t bitIndex = 0; bitIndex < ECSHeader::TightPackLength; ++bitIndex)
                    {
                        Entity entity((i * ECSHeader::TightPackLength) + bitIndex + 1);
                        if ((uint32_t)entity == header.NumEntities)
                        {
                            out << bitset;
                            return;
                        }
                        bitset |= (ecs.IsValid(entity) << bitIndex);
                    }
                    out << bitset;
                }
            }
            else
            {
                for (uint32_t i = 0; i < header.NumEntities; ++i)
                    out << ecs.IsValid(i + 1);
            }
        }
    }
    void ECSSerializer::deserializeECSHeaderAndData(ECSManager& ecs, const ByteStream& in)
    {
        ECSHeader header;
        in >> header;
        if (header.EntitiesIncluded)
        {
            ecs.Clear();
            for (uint32_t i = 0; i < header.NumEntities; ++i)
            {
                ecs.CreateEntity();
            }
            if (header.Tight)
            {
                uint32_t numBitsets = (uint32_t)std::ceil((double)header.NumEntities / (double)ECSHeader::TightPackLength);
                for (uint32_t i = 0; i < numBitsets; ++i)
                {
                    uint8_t bitset;
                    in >> bitset;
                    for (uint32_t bitIndex = 0; bitIndex < ECSHeader::TightPackLength; ++bitIndex)
                    {
                        Entity entity((i * ECSHeader::TightPackLength) + bitIndex + 1);
                        if ((uint32_t)entity == header.NumEntities)
                            return;

                        bool valid = bitset | bitIndex;
                        if (!valid)  ecs.DestroyEntity(entity);
                    }
                }
            }
            else
            {
                for (uint32_t i = 0; i < header.NumEntities; ++i)
                {
                    bool valid = false;
                    in >> valid;
                    if (!valid) ecs.DestroyEntity(i);
                }
            }
        }
        else
        {
            // TODO: something went wrong
        }
    }
}