#include "stdafx.h"
#include "LuaApp.h"


#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Serializer.h"
#include "XYZ/Core/Input.h"
#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/MouseCodes.h"
#include "LuaEntity.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/RefCountedPtr.h>
#include <LuaBridge/RefCountedObject.h>


// TODO: delete
#include <locale>
#include <codecvt>

namespace luabridge {

	template <typename T>
	struct EnumWrapper 
	{
		static inline
			typename std::enable_if<std::is_enum<T>::value, void>::type
			push(lua_State* L, T value) 
		{
			lua_pushnumber(L, static_cast<std::size_t>(value));
		}

		static inline
			typename std::enable_if<std::is_enum<T>::value, T>::type
			get(lua_State* L, int index) 
		{
			return static_cast<T>(lua_tointeger(L, index));
		}
	};

	template <>
	struct luabridge::Stack<XYZ::KeyCode> : EnumWrapper<XYZ::KeyCode> 
	{};
	template <>
	struct luabridge::Stack<XYZ::MouseCode> : EnumWrapper<XYZ::MouseCode>
	{};

	template <>
	struct luabridge::Stack<XYZ::TextureParam> : EnumWrapper<XYZ::TextureParam>
	{};
	template <>	
	struct luabridge::Stack<XYZ::TextureWrap> : EnumWrapper<XYZ::TextureWrap>
	{};


	template <class T>
	struct ContainerTraits <XYZ::Ref <T> >
	{
		typedef typename T Type;

		static T* get(XYZ::Ref<T> c)
		{
			return c.Raw();
		}
	};
}

namespace XYZ {

	static bool CheckLua(lua_State* L, int err)
	{
		if (err != 0)
		{
			XYZ_LOG_ERR(lua_tostring(L, -1));
			return false;
		}
		return true;
	}

	static Ref<Texture2D> CreateTexture2D(TextureWrap wrap, TextureParam minParam, TextureParam magParam, const std::string& path)
	{
		return Texture2D::Create({ wrap, minParam, magParam }, path);
	}

	static Ref<SubTexture> CreateSubTexture(const Ref<Texture>& texture, const glm::vec4& texCoords)
	{
		return Ref<SubTexture>::Create(texture, texCoords);
	}
	
	LuaApp::LuaApp(const std::string& directory, const std::string& filename)
		:
		m_Directory(directory),
		m_FileName(filename)
	{
		std::wstring unicodeDir(m_Directory.begin(), m_Directory.end());
		m_FileWatcher = FileWatcher::Create(unicodeDir);
		m_FileWatcher->AddListener(this);
		m_FileWatcher->Start();

		m_L = luaL_newstate();
		luaL_openlibs(m_L);

		// Input
		luabridge::getGlobalNamespace(m_L)
			.beginClass <Input>("Input")
			.addStaticFunction("IsMouseButtonPressed", Input::IsMouseButtonPressed)
			.addStaticFunction("IsKeyPressed", Input::IsKeyPressed)
			.addStaticFunction("GetMouseX", Input::GetMouseX)
			.addStaticFunction("GetMouseY", Input::GetMouseY)
			.endClass();


		// Math
		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec2>("Vec2")
			.addProperty("x", &glm::vec2::x)
			.addProperty("y", &glm::vec2::y)
			.endClass();
		
		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec3>("Vec3")
			.addProperty("x", &glm::vec3::x)
			.addProperty("y", &glm::vec3::y)
			.addProperty("z", &glm::vec3::z)
			.endClass();

		luabridge::getGlobalNamespace(m_L)
			.beginClass <glm::vec4>("Vec4")
			.addConstructor <void (*) (float, float, float , float)>()
			.addProperty("x", &glm::vec4::x)
			.addProperty("y", &glm::vec4::y)
			.addProperty("z", &glm::vec4::z)
			.addProperty("w", &glm::vec4::w)
			.endClass();


		luabridge::getGlobalNamespace(m_L)
			.beginClass<Texture>("Texture")
			.endClass()
			.deriveClass <Texture2D, Texture>("Texture2D")
			.addStaticFunction("Create", &CreateTexture2D)
			.endClass();

		luabridge::getGlobalNamespace(m_L)
			.beginClass<SubTexture>("SubTexture")
			.addConstructor <void (*) (const Ref<Texture>&, const glm::vec4&), Ref <SubTexture> >()
			.addFunction("SetTexture", &SubTexture::SetTexture)
			.addFunction("SetCoords", &SubTexture::SetCoords)
			.endClass()
			.beginClass<Ref<SubTexture>>("SubTextureRef")
			.addFunction("Get", &Ref<SubTexture>::Get)
			.addStaticFunction("Create", &CreateSubTexture)
			.endClass();

		// Components
		luabridge::getGlobalNamespace(m_L)
			.beginClass<TransformComponent>("TransformComponent")
			.addProperty("Translation", &TransformComponent::Translation)
			.addProperty("Rotation", &TransformComponent::Rotation)
			.addProperty("Scale", &TransformComponent::Scale)
			.endClass();


		luabridge::getGlobalNamespace(m_L)
			.beginClass<SpriteRenderer>("SpriteRenderer")
			.addProperty("SubTexture", &SpriteRenderer::SubTexture)
			.addProperty("Color", &SpriteRenderer::Color)
			.endClass();
		
		
		luabridge::getGlobalNamespace(m_L)
			.beginClass<AnimationController>("AnimationController")
			.addFunction("TransitionTo", &AnimationController::TransitionTo)
			.endClass();


		luabridge::getGlobalNamespace(m_L)
			.beginClass<AnimatorComponent>("AnimatorComponent")
			.addProperty("Controller", &AnimatorComponent::Controller)
			.endClass();


		// Entity
		luabridge::getGlobalNamespace(m_L)
			.beginClass<LuaEntity>("Entity")
			.addFunction("GetTransform", &LuaEntity::GetComponent<TransformComponent>)
			.addFunction("GetSpriteRenderer", &LuaEntity::GetComponent<SpriteRenderer>)
			.addFunction("GetAnimator", &LuaEntity::GetComponent<AnimatorComponent>)
			.addStaticFunction("FindEntity", &LuaEntity::FindEntity)
			.addStaticFunction("CreateEntity", &LuaEntity::CreateEntity)
			.endClass();


		std::string fullPath = m_Directory + "/" + filename;
	
		if (CheckLua(m_L, luaL_dofile(m_L, fullPath.c_str())))
		{
		}			
	}

	LuaApp::~LuaApp()
	{
		m_FileWatcher->Stop();

		lua_close(m_L);
	}

	void LuaApp::OnUpdate(Timestep ts)
	{
		try
		{
			{
				std::scoped_lock lock(m_Mutex);
				if (m_Reload)
					m_Reload = !tryReload();
			}

			luabridge::LuaRef handler = luabridge::getGlobal(m_L, "OnUpdate");
			handler((lua_Number)ts.GetSeconds());
		}
		catch (const std::exception& e)
		{
			XYZ_LOG_ERR("Exception OnUpdate: ", e.what());
		}	
	}
	void LuaApp::OnFileChange(const std::wstring& filepath)
	{
		std::string fullPath = m_Directory + "/" + m_FileName;	
		std::scoped_lock lock(m_Mutex);
		m_Reload = true;
	}
	void LuaApp::OnFileAdded(const std::wstring& filepath)
	{
	}
	void LuaApp::OnFileRemoved(const std::wstring& filepath)
	{
	}
	void LuaApp::OnFileRenamed(const std::wstring& filepath)
	{
	}

	bool LuaApp::tryReload()
	{
		std::string fullPath = m_Directory + "/" + m_FileName;
		auto it = fopen(fullPath.c_str(), "r");
		if (it)
		{
			fclose(it);
			if (CheckLua(m_L, luaL_dofile(m_L, fullPath.c_str())))
			{
			}
			XYZ_LOG_INFO("Lua Script successfully reloaded ", m_FileName);
			return true;
		}
		return false;
	}
}