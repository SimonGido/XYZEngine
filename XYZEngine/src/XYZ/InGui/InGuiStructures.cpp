#include "stdafx.h"
#include "InGuiStructures.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	
	InGuiRenderConfiguration::InGuiRenderConfiguration()	
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Texture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		Material = Ref<XYZ::Material>::Create(shader);
		Material->Set("u_Texture", Texture, TextureID);
		Material->Set("u_Texture", Font->GetTexture(), FontTextureID);
		Material->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		Material->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));
		
	
		float divisor = 8.0f;
		SubTexture[BUTTON] = Ref<SubTexture2D>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_CHECKED] = Ref<SubTexture2D>::Create(Texture, glm::vec2(1, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_UNCHECKED] = Ref<SubTexture2D>::Create(Texture, glm::vec2(0, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER] = Ref<SubTexture2D>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER_HANDLE] = Ref<SubTexture2D>::Create(Texture, glm::vec2(1, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[WINDOW] = Ref<SubTexture2D>::Create(Texture, glm::vec2(0, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[MIN_BUTTON] = Ref<SubTexture2D>::Create(Texture, glm::vec2(1, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CLOSE_BUTTON] = Ref<SubTexture2D>::Create(Texture, glm::vec2(2, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOWN_ARROW] = Ref<SubTexture2D>::Create(Texture, glm::vec2(2, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[RIGHT_ARROW] = Ref<SubTexture2D>::Create(Texture, glm::vec2(2, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[LEFT_ARROW] = Ref<SubTexture2D>::Create(Texture, glm::vec2(3, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOCKSPACE] = Ref<SubTexture2D>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
	
	
		Color[DEFAULT_COLOR] = { 1.0f,1.0f,1.0f,1.0f };
		Color[HOOVER_COLOR] = { 1.0f, 2.5f, 2.8f, 1.0f };
		Color[SELECT_COLOR] = { 0.8f,0.0f,0.2f,0.6f };
		Color[LINE_COLOR] = { 0.4f,0.5f,0.8f,1.0f };
		Color[SELECTOR_COLOR] = { 1.0f,1.0f,1.0f,1.0f };
	}

	InGuiPerFrameData::InGuiPerFrameData()
	{
		ModifiedWindow = nullptr;
		
		TempVertices = new InGuiVertex[260 * 4];
		ViewProjectionMatrix = glm::mat4(1.0f);

		ModifiedWindowMouseOffset = { 0,0 };
		SelectedPoint = { 0,0 };

		PanelOffset = 0.0f;
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

	void InGuiPerFrameData::ResetFrameData()
	{
		TexturePairs.clear();
		KeyCode = ToUnderlying(KeyCode::XYZ_KEY_NONE);
		Mode = ToUnderlying(KeyMode::XYZ_MOD_NONE);
		Code = ToUnderlying(MouseCode::XYZ_MOUSE_NONE);
		PanelOffset = 0.0f;
	}

	void InGuiPerFrameData::ResetWindowData()
	{	
		WindowSpaceOffset = { 0,0 };
		CurrentWindow = nullptr;
		
		ActiveMesh = nullptr;
		ActiveLineMesh = nullptr;
		ActiveOverlayMesh = nullptr;
		ActiveOverlayLineMesh = nullptr;

		MenuBarOffset = { 0,0 };
		PopupOffset = { 0,0 };
		PopupSize = { 0,0 };

		MaxHeightInRow = 0.0f;
		MenuItemOffset = 0.0f;
	}

	void InGuiRenderQueue::PushOverlay(InGuiMesh* mesh, InGuiLineMesh* lineMesh, uint8_t queueType)
	{
		XYZ_ASSERT(queueType < Type::NUM, "");
		if (!m_Queues[queueType].NumOverLayers)
		{
			m_Queues[queueType].InGuiMeshes.push_back(mesh);
			m_Queues[queueType].InGuiLineMeshes.push_back(lineMesh);
			m_Queues[queueType].NumOverLayers++;
		}
		else
		{
			m_Queues[queueType].InGuiMeshes.insert(m_Queues[queueType].InGuiMeshes.end() - m_Queues[queueType].NumOverLayers, mesh);
			m_Queues[queueType].InGuiLineMeshes.insert(m_Queues[queueType].InGuiLineMeshes.end() - m_Queues[queueType].NumOverLayers, lineMesh);
		}
	}
	void InGuiRenderQueue::Push(InGuiMesh* mesh, InGuiLineMesh* lineMesh, uint8_t queueType)
	{
		XYZ_ASSERT(queueType < Type::NUM,"");
		m_Queues[queueType].InGuiMeshes.insert(m_Queues[queueType].InGuiMeshes.end() - m_Queues[queueType].NumOverLayers, mesh);
		m_Queues[queueType].InGuiLineMeshes.insert(m_Queues[queueType].InGuiLineMeshes.end() - m_Queues[queueType].NumOverLayers, lineMesh);
	}

	void InGuiRenderQueue::Submit(uint8_t queueType)
	{
		XYZ_ASSERT(queueType < Type::NUM, "");
		for (uint32_t j = 0; j < m_Queues[queueType].InGuiMeshes.size(); ++j)
		{
			InGuiRenderer::SubmitUI(*m_Queues[queueType].InGuiMeshes[j]);
			InGuiRenderer::SubmitLineMesh(*m_Queues[queueType].InGuiLineMeshes[j]);
		}	
		m_Queues[queueType].InGuiMeshes.clear();
		m_Queues[queueType].InGuiLineMeshes.clear();
		m_Queues[queueType].NumOverLayers = 0;
	}

	void InGuiRenderQueue::Reset()
	{
		for (size_t i = 0; i < Type::NUM; ++i)
		{
			m_Queues[i].InGuiMeshes.clear();
			m_Queues[i].InGuiLineMeshes.clear();
			m_Queues[i].NumOverLayers = 0;
		}
	}
}