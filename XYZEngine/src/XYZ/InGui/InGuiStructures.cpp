#include "stdafx.h"
#include "InGuiStructures.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	
	InGuiRenderConfiguration::InGuiRenderConfiguration()	
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Ref<Texture2D> texture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest, "Assets/Textures/Gui/TexturePack_Dark.png");
		InTexture = texture;
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create(12, "Assets/Fonts/arial.ttf");
		InMaterial = Material::Create(shader);
		InMaterial->Set("u_Texture", texture, TextureID);
		InMaterial->Set("u_Texture", Font->GetTexture(), FontTextureID);
		InMaterial->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		InMaterial->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));
		
	
		float divisor = 8.0f;
		SubTexture[BUTTON] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_CHECKED] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_UNCHECKED] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[SLIDER] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[SLIDER_HANDLE] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[WINDOW] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[MIN_BUTTON] = Ref<SubTexture2D>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[DOWN_ARROW] = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[RIGHT_ARROW] = Ref<SubTexture2D>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[LEFT_ARROW] = Ref<SubTexture2D>::Create(texture, glm::vec2(3, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		SubTexture[DOCKSPACE] = Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
	
	
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
		ActiveMesh = nullptr;
		ActiveLineMesh = nullptr;

		TempVertices = new InGuiVertex[260 * 4];
		ViewProjectionMatrix = glm::mat4(1.0f);

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

	InGuiPerFrameData::~InGuiPerFrameData()
	{
		delete[] TempVertices;
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