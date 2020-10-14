#include "stdafx.h"
#include "InGuiStructures.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	
	InGuiRenderConfiguration::InGuiRenderConfiguration()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Ref<Texture2D> texture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/Gui/TexturePack_Dark.png");
		InTexture = texture;
		Ref<Texture2D> fontTexture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Linear, TextureParam::Linear, "Assets/Font/Arial.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create("Assets/Font/Arial.fnt");

		InMaterial = Material::Create(shader);
		InMaterial->Set("u_Texture", texture, TextureID);
		InMaterial->Set("u_Texture", fontTexture, FontTextureID);
		InMaterial->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		InMaterial->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));
		
		SubTexture[BUTTON] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[CHECKBOX_CHECKED] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[CHECKBOX_UNCHECKED] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[SLIDER] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[SLIDER_HANDLE] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[WINDOW] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[MIN_BUTTON] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[DOWN_ARROW] = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[RIGHT_ARROW] = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
		SubTexture[DOCKSPACE] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 8, texture->GetHeight() / 4));
	
	
		Color[DEFAULT_COLOR] = { 1.0f,1.0f,1.0f,1.0f };
		Color[HOOVER_COLOR] = { 1.0f, 2.5f, 2.8f, 1.0f };
		Color[SELECT_COLOR] = { 0.8f,0.0f,0.2f,0.6f };
		Color[LINE_COLOR] = { 0.4f,0.5f,0.8f,1.0f };
		Color[SELECTOR_COLOR] = { 1.0f,1.0f,1.0f,1.0f };
	}

	InGuiPerFrameData::InGuiPerFrameData()
	{
		EventReceivingWindow = nullptr;
		ModifiedWindow = nullptr;
		CurrentWindow = nullptr;
		CurrentNodeWindow = nullptr;
		CurrentNode = nullptr;

		ModifiedWindowMouseOffset = { 0,0 };
		SelectedPoint = { 0,0 };
		LeftNodePinOffset = 0.0f;
		RightNodePinOffset = 0.0f;

		ItemOffset = 0.0f;
		KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		CapslockEnabled = false;
		
		ResetWindowData();
	}

	void InGuiPerFrameData::ResetWindowData()
	{	
		WindowSpaceOffset = { 0,0 };
		CurrentWindow = nullptr;
		
		MenuBarOffset = { 0,0 };
		PopupOffset = { 0,0 };
		PopupSize = { 0,0 };

		MaxHeightInRow = 0.0f;
		MenuItemOffset = 0.0f;
	}

	InGuiNodeWindow::InGuiNodeWindow()
	{
	}
	InGuiNodeWindow::~InGuiNodeWindow()
	{
		for (auto node : Nodes)
			delete node.second;
	}
	void InGuiRenderQueue::PushOverlay(InGuiMesh* mesh, InGuiLineMesh* lineMesh)
	{
		if (!m_NumOverLayers)
		{
			m_InGuiMeshes.push_back(mesh);
			m_InGuiLineMeshes.push_back(lineMesh);
			m_NumOverLayers++;
		}
		else
		{
			m_InGuiMeshes.insert(m_InGuiMeshes.end() - m_NumOverLayers, mesh);
			m_InGuiLineMeshes.insert(m_InGuiLineMeshes.end() - m_NumOverLayers, lineMesh);
		}
	}
	void InGuiRenderQueue::Push(InGuiMesh* mesh, InGuiLineMesh* lineMesh)
	{
		m_InGuiMeshes.insert(m_InGuiMeshes.end() - m_NumOverLayers, mesh);
		m_InGuiLineMeshes.insert(m_InGuiLineMeshes.end() - m_NumOverLayers, lineMesh);
	}
	void InGuiRenderQueue::Reset()
	{
		size_t numMeshes = m_InGuiMeshes.size();
		m_InGuiMeshes.clear();
		m_InGuiLineMeshes.clear();
		m_InGuiMeshes.reserve(numMeshes);
		m_InGuiLineMeshes.reserve(numMeshes);
		m_NumOverLayers = 0;
	}
}