#include "stdafx.h"
#include "LuaModule.h"


#include "XYZ/ECS/Entity.h"
#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/Serializer.h"
#include "XYZ/Core/Application.h"
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



// TODO: delete
#include <locale>
#include <codecvt>

namespace XYZ {

	static int SetLuaPath(lua_State* L, const char* path)
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
		std::string cur_path = lua_tostring(L, -1); // grab path string from top of stack
		cur_path.append(";"); // do your path magic here
		cur_path.append(path);
		cur_path.append("\\?.lua");
		lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
		lua_pushstring(L, cur_path.c_str()); // push the new one
		lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
		lua_pop(L, 1); // get rid of package table from top of stack
		return 0; // all done!
	}


	LuaModule::LuaModule(const std::string& directory, const std::string& filename)
		:
		m_Directory(directory),
		m_FileName(filename)
	{
		std::wstring unicodeDir(m_Directory.begin(), m_Directory.end());
		m_FileWatcher = FileWatcher::Create(unicodeDir);
		m_FileWatcher->AddListener(this);
		m_FileWatcher->Start();

		m_L.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os);
		SetLuaPath(m_L, Application::Get().GetApplicationDir().c_str());

		// Input
		{
			sol::usertype<Input> type = m_L.new_usertype<Input>("Input");
			type["IsKeyPressed"] = &Input::IsKeyPressed;
			type["IsMouseButtonPressed"] = &Input::IsMouseButtonPressed;
			type["GetMouseX"] = &Input::GetMouseX;
			type["GetMouseY"] = &Input::GetMouseY;
		}
		// Vec2
		{
			sol::usertype<glm::vec2> type = m_L.new_usertype<glm::vec2>("Vec2",
				sol::call_constructor, sol::factories(
					[]() { return glm::vec2(); },
					[](float val) { return glm::vec2(val); },
					[](float x, float y) { return glm::vec2(x, y); }
			));
			type["x"] = &glm::vec2::x;
			type["y"] = &glm::vec2::y;
		}

		// Vec3
		{
			sol::usertype<glm::vec3> type = m_L.new_usertype<glm::vec3>("Vec3",
				sol::call_constructor, sol::factories(
					[]() { return glm::vec3(); },
					[](float val) { return glm::vec3(val); },
					[](float x, float y, float z) { return glm::vec3(x, y, z); }
			));
			type["x"] = &glm::vec3::x;
			type["y"] = &glm::vec3::y;
			type["z"] = &glm::vec3::z;
		}

		// Vec4
		{
			sol::usertype<glm::vec4> type = m_L.new_usertype<glm::vec4>("Vec4",
				sol::call_constructor, sol::factories(
					[]() { return glm::vec4(); },
					[](float val) { return glm::vec4(val); },
					[](float x, float y, float z, float w) { return glm::vec4(x, y, z, w); }
			));
			type["x"] = &glm::vec4::x;
			type["y"] = &glm::vec4::y;
			type["z"] = &glm::vec4::z;
			type["w"] = &glm::vec4::w;
		}

		// Texture
		{
			sol::usertype<Texture> type = m_L.new_usertype<Texture>("Texture",
				"GetWidth", &Texture::GetWidth,
				"GetHeight", &Texture::GetHeight,
				"GetChannels", &Texture::GetChannels
				);
		}

		// Texture2D
		{
			sol::usertype<Texture2D> type = m_L.new_usertype<Texture2D>("Texture2D",
				sol::base_classes, sol::bases<Texture>()
				);
		}
		
		// TextureSpecs
		{
			sol::usertype<TextureSpecs> type = m_L.new_usertype<TextureSpecs>("TextureSpecs");
			type["Wrap"] = &TextureSpecs::Wrap;
			type["MinParam"] = &TextureSpecs::MinParam;
			type["MagParam"] = &TextureSpecs::MagParam;
		}

		// RefTexture
		{
			sol::usertype<Ref<Texture>> type = m_L.new_usertype<Ref<Texture>>("RefTexture",
				sol::call_constructor, sol::factories(
					[](const TextureSpecs& specs, const std::string& path) { return Texture2D::Create(specs, path); }
			));
			type["Get"] = &Ref<Texture>::Get;
		}

		// SubTexture
		{
			sol::automagic_enrollments enrolments;
			enrolments.default_constructor = false;
			sol::usertype<SubTexture> type = m_L.new_usertype<SubTexture>("SubTexture", enrolments);
			type["SetTexture"] = &SubTexture::SetTexture;
			type["SetCoords"] = &SubTexture::SetCoords;
			type["GetTexCoords"] = &SubTexture::GetTexCoords;
			type["GetTexture"] = &SubTexture::GetTexture;
		}
		// RefSubTexture
		{
			sol::usertype<Ref<SubTexture>> type = m_L.new_usertype<Ref<SubTexture>>("RefSubTexture",
				sol::call_constructor, sol::factories(
					[](const Ref<Texture>& texture, const glm::vec4& texCoords) { return Ref<SubTexture>::Create(texture, texCoords); }
			));
			type["Get"] = &Ref<SubTexture>::Get;
		}

		// AnimationController
		{
			sol::usertype<AnimationController> type = m_L.new_usertype<AnimationController>("AnimationController");
			type["TransitionTo"] = &AnimationController::TransitionTo;
		}


		// Transform
		{
			sol::usertype<TransformComponent> type = m_L.new_usertype<TransformComponent>("TransformComponent");
			type["Translation"] = &TransformComponent::Translation;
			type["Rotation"] = &TransformComponent::Rotation;
			type["Scale"] = &TransformComponent::Scale;
		}

		// SpriteRenderer
		{
			sol::usertype<SpriteRenderer> type = m_L.new_usertype<SpriteRenderer>("SpriteRenderer");
			type["SubTexture"] = &SpriteRenderer::SubTexture;
			type["Color"] = &SpriteRenderer::Color;
		}

		// AnimatorComponent
		{
			sol::usertype <AnimatorComponent> type = m_L.new_usertype<AnimatorComponent>("AnimatorComponent");
			type["Controller"] = &AnimatorComponent::Controller;
		}

		// LuaEntity
		{
			sol::usertype<LuaEntity> type = m_L.new_usertype<LuaEntity>("Entity");
			type["GetTransform"] = &LuaEntity::GetComponent<TransformComponent>;
			type["GetSpriteRenderer"] = &LuaEntity::GetComponent<SpriteRenderer>;
			type["GetAnimator"] = &LuaEntity::GetComponent<AnimatorComponent>;
			type["FindEntity"] = &LuaEntity::FindEntity;
			type["CreateEntity"] = &LuaEntity::CreateEntity;
		}

		std::string fullPath = m_Directory + "/" + filename;
		m_L.script_file(fullPath);
	}

	LuaModule::~LuaModule()
	{
		m_FileWatcher->Stop();

		lua_close(m_L);
	}

	void LuaModule::OnUpdate(Timestep ts)
	{
		{
			std::scoped_lock lock(m_Mutex);
			if (m_Reload)
				m_Reload = !tryReload();
		}
		auto res = m_L["OnUpdate"](ts.GetSeconds());
		if (!res.valid())
		{
			sol::error err = res;
			XYZ_LOG_ERR("Exception OnUpdate: ", err.what());
		}
	}
	void LuaModule::OnFileChange(const std::wstring& filepath)
	{
		std::string fullPath = m_Directory + "/" + m_FileName;	
		std::scoped_lock lock(m_Mutex);
		m_Reload = true;
	}
	void LuaModule::OnFileAdded(const std::wstring& filepath)
	{
	}
	void LuaModule::OnFileRemoved(const std::wstring& filepath)
	{
	}
	void LuaModule::OnFileRenamed(const std::wstring& filepath)
	{
	}

	bool LuaModule::tryReload()
	{
		std::string fullPath = m_Directory + "/" + m_FileName;
		auto it = fopen(fullPath.c_str(), "r");
		if (it)
		{
			fclose(it);
			m_L.script_file(fullPath);
			XYZ_LOG_INFO("Lua Script successfully reloaded ", m_FileName);
			return true;
		}
		return false;
	}
}