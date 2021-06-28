#include "stdafx.h"
#include "CallbackManager.h"

namespace XYZ {
	CallbackManager::CallbackManager()
		:
		m_StoragesCreated(0)
	{
	}
	CallbackManager::CallbackManager(const CallbackManager& other)
		:
		m_StoragesCreated(other.m_StoragesCreated)
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Copy();
			}
			counter++;
		}
	}
	CallbackManager::CallbackManager(CallbackManager&& other) noexcept
		:
		m_StoragesCreated(other.m_StoragesCreated)
	{
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Move();
			}
			counter++;
		}
		other.m_Storages.clear();
	}
	CallbackManager::~CallbackManager()
	{
		destroyStorages();
	}
	CallbackManager& CallbackManager::operator=(CallbackManager&& other) noexcept
	{
		destroyStorages();
		size_t counter = 0;
		m_Storages.resize(other.m_Storages.size());
		for (auto storage : other.m_Storages)
		{
			if (storage)
			{
				m_Storages[counter] = storage->Move();
			}
			counter++;
		}
		other.m_Storages.clear();
		return *this;
	}
	void CallbackManager::Clear()
	{
		destroyStorages();
	}
	void CallbackManager::OnEntityDestroyed(uint32_t entity, const Signature& signature)
	{
		for (auto storage : m_Storages)
		{
			if (storage)
				storage->Execute(entity, CallbackType::EntityDestroy);
		}
	}
	void CallbackManager::destroyStorages()
	{
		for (auto storage : m_Storages)
		{
			if (storage)
				delete storage;
		}
	}
}