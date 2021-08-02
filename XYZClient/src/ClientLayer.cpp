#include "ClientLayer.h"

namespace XYZ {
	static void SerializePlayer(Net::Message<MessageType>& message, const Player& player)
	{
		message << player.Position;
		message << player.Size;
		message << player.Color;
		message << player.ID;
	}

	static Player DeserializePlayer(Net::Message<MessageType>& message)
	{
		Player player;
		message >> player.ID;
		message >> player.Color;
		message >> player.Size;
		message >> player.Position;
		return player;
	}

	void ClientLayer::OnAttach()
	{
		m_Client.Connect("192.168.7.179", 60000);
		Application::Get().GetImGuiLayer()->BlockEvents(false);
		Application::Get().GetImGuiLayer()->EnableDockspace(false);
	}
	void ClientLayer::OnDetach()
	{
		m_Client.Disconnect();
	}
	void ClientLayer::OnUpdate(Timestep ts)
	{
		if (m_Client.IsConnected())
		{
			if (!m_Client.GetIncomingMessages().Empty())
			{
				auto msg = std::move(m_Client.GetIncomingMessages().PopFront().Message);
				switch (msg.Header.ID)
				{
				case MessageType::ServerAccept:
				{
					uint32_t id;
					msg >> id;
					m_Client.SetID(id);
					break;
				}
				case MessageType::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
					break;
				}
				case MessageType::PlayerUpdate:
				{
					m_Client.UpdatePlayers(msg);
					break;
				}
				}
			}
			m_Client.Update(ts);
		}
	}
	void ClientLayer::OnEvent(Event& event)
	{
		m_Client.OnEvent(event);
	}
	CustomClient::CustomClient()
		:
		m_CameraController(16.0f / 9.0f)
	{
	}
	void CustomClient::Update(Timestep ts)
	{
		Net::Message<MessageType> message;
		message.Header.ID = MessageType::PlayerUpdate;

		if (m_Players.size() > m_ID)
		{
			auto& player = m_Players[m_ID];
			if (Input::IsKeyPressed(KeyCode::KEY_LEFT))
			{
				player.Position.x -= sc_Speed * ts;
				SerializePlayer(message, player);
				Send(message);
			}
			else if (Input::IsKeyPressed(KeyCode::KEY_RIGHT))
			{
				player.Position.x += sc_Speed * ts;
				SerializePlayer(message, player);
				Send(message);
			}
			else if (Input::IsKeyPressed(KeyCode::KEY_UP))
			{
				player.Position.y += sc_Speed * ts;
				SerializePlayer(message, player);
				Send(message);
			}
			else if (Input::IsKeyPressed(KeyCode::KEY_DOWN))
			{
				player.Position.y -= sc_Speed * ts;
				SerializePlayer(message, player);
				Send(message);
			}
		}

		m_CameraController.OnUpdate(ts);

		Renderer::Clear();
		Renderer::SetClearColor({ 0.1f,0.1f,0.1f,1.0f });
		Renderer2D::BeginScene(m_CameraController.GetCamera().GetViewProjectionMatrix(), m_CameraController.GetCamera().GetPosition());

		for (auto& player : m_Players)
		{
			Renderer2D::SubmitQuad(glm::vec3(player.Position.x, player.Position.y, 0.0f), player.Size, player.Color);
		}

		Renderer2D::Flush();
		Renderer2D::FlushLines();
		Renderer2D::EndScene();
	}
	void CustomClient::UpdatePlayers(Net::Message<MessageType>& message)
	{
		size_t size;
		message >> size;
		for (size_t i = 0; i < size; ++i)
		{
			Player player = DeserializePlayer(message);	
			if (player.ID >= m_Players.size())
				m_Players.resize(player.ID + 1);
			m_Players[player.ID] = player;
		}
	}
	void CustomClient::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}
