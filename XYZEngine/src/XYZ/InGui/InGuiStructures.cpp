#include "stdafx.h"
#include "InGuiStructures.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	
	InGuiRenderConfiguration::InGuiRenderConfiguration()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Ref<Texture2D> texture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Texture2D> fontTexture = Texture2D::Create(TextureWrap::Clamp, "Assets/Font/Arial.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create("Assets/Font/Arial.fnt");

		InMaterial = Material::Create(shader);
		InMaterial->Set("u_Texture", texture, TextureID);
		InMaterial->Set("u_Texture", fontTexture, FontTextureID);
		InMaterial->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		InMaterial->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));
		
		
		
		ButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		CheckboxSubTextureChecked = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		CheckboxSubTextureUnChecked = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		SliderSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		SliderHandleSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		WindowSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		MinimizeButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		DownArrowButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(3, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		RightArrowButtonSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
		DockSpaceSubTexture = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4));
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