#include "stdafx.h"
#include "ParticleMaterial.h"

#include "XYZ/Utils/StringUtils.h"

namespace XYZ {
	static std::string GetStatement(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, ";");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		size_t length = end - str + 1;
		return std::string(str, length);
	}

	static std::string GetEndStruct(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, "};");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		size_t length = end - str + 1;
		return std::string(str, length);
	}

	static ShaderStruct ParseStruct(const std::string& structSource)
	{
		ShaderStruct shaderStruct;
		std::vector<std::string> tokens = std::move(Utils::SplitString(structSource, "\t\n"));
		std::vector<std::string> structName = std::move(Utils::SplitString(tokens[0], " \r"));
		shaderStruct.Name = structName[1];
		for (size_t i = 1; i < tokens.size(); ++i)
		{
			std::vector<std::string> variables = std::move(Utils::SplitString(tokens[i], " \r"));
			if (variables.size() > 1)
			{
				ShaderUniformDataType type = StringToShaderDataType(variables[0]);
				variables[1].pop_back(); // pop ;
				std::string name = variables[1];
				shaderStruct.Variables.push_back({name, type});
			}
		}
		return shaderStruct;
	}

	static std::vector<ShaderStruct> ParseStructs(const std::string& source)
	{
		std::vector<ShaderStruct> structs;
		const char* token = nullptr;
		const char* src = source.c_str();
		while (token = Utils::FindToken(src, "struct"))
		{
			structs.push_back(ParseStruct(GetEndStruct(token, &src)));
		}			
		return structs;
	}

	struct ParShaderBuffer
	{
		std::string					Name;
		uint32_t					Size;
		uint32_t					Binding;
		bool						Indirect;
		bool						RenderBuffer;
		bool						ParticleBuffer;
		std::vector<ShaderVariable> Variables; // This is used only if RenderBuffer is true
	};

	static bool ParseShaderBuffer(ParShaderBuffer& buffer, const std::string& bufferSource, const std::vector<ShaderStruct>& structs)
	{
		std::vector<std::string> split = Utils::SplitString(bufferSource, " ,=}{\n\r\t");
		auto bindingIt = std::find(split.begin(), split.end(), "binding");
		if (bindingIt != split.end())
			buffer.Binding = atoi((++bindingIt)->c_str());
		
		auto bufferIt = std::find(bindingIt, split.end(), "buffer");
		if (bufferIt == split.end())
			return false;

		auto nameIt = (++bufferIt);
		buffer.Name = *nameIt;

		auto externalIt = std::find(nameIt, split.end(), "external");
		if (externalIt != split.end())
			return false;

		auto indirectIt = std::find(nameIt, split.end(), "indirect");
		buffer.Indirect = (indirectIt != split.end());

		auto renderIt = std::find(nameIt, split.end(), "render");
		buffer.RenderBuffer = (renderIt != split.end());

		auto particleIt = std::find(nameIt, split.end(), "particle");
		buffer.ParticleBuffer = (particleIt != split.end());

	

		for (auto& str : structs)
		{
			auto structIt = std::find(nameIt, split.end(), str.Name);
			if (structIt != split.end())
			{
				buffer.Size = (uint32_t)str.Size();
				if (buffer.RenderBuffer)
					buffer.Variables = str.Variables;
				break;
			}
		}
		return true;
	}

	static std::vector<ParShaderBuffer> ParseShaderBuffers(const std::string& source, const std::vector<ShaderStruct>& structs)
	{
		std::vector<ParShaderBuffer> buffers;
		size_t bufferStart = 0;
		size_t bufferEnd = 0;
		bufferStart = source.find("layout", bufferStart);
		bufferEnd = source.find("};", bufferStart);

		while (bufferStart != std::string::npos)
		{
			std::string bufferStr = source.substr(bufferStart, bufferEnd - bufferStart);
			ParShaderBuffer buffer;
			if (ParseShaderBuffer(buffer, bufferStr, structs))
				buffers.push_back(buffer);
			bufferStart = source.find("layout", bufferStart + 1);
			bufferEnd = source.find("};", bufferStart);
		}
		return buffers;
	}

	struct ShaderCounter
	{
		std::string Name;
		uint32_t    Binding;
		uint32_t    Offset;
	};

	static bool ParseShaderCounter(ShaderCounter& counter, const std::string& counterSource)
	{
		std::vector<std::string> split = Utils::SplitString(counterSource, " ,=()\n\r\t");
		auto bindingIt = std::find(split.begin(), split.end(), "binding");
		if (bindingIt != split.end())
			counter.Binding = atoi((++bindingIt)->c_str());

		auto offsetIt = std::find(bindingIt, split.end(), "offset");
		if (offsetIt != split.end())
			counter.Offset = atoi(offsetIt->c_str());

		auto atomicIt = std::find(offsetIt, split.end(), "atomic_uint");
		if (atomicIt == split.end())
			return false;

		auto nameIt = (++atomicIt);
		counter.Name = *nameIt;
		
		return true;
	}

	static std::vector<ShaderCounter> ParseShaderCounters(const std::string& source)
	{
		std::vector<ShaderCounter> counters;
		size_t bufferStart = 0;
		size_t bufferEnd = 0;
		bufferStart = source.find("layout", bufferStart);
		bufferEnd = source.find("};", bufferStart);

		while (bufferStart != std::string::npos)
		{
			std::string bufferStr = source.substr(bufferStart, bufferEnd - bufferStart);
			ShaderCounter counter;
			if (ParseShaderCounter(counter, bufferStr))
				counters.push_back(counter);
			bufferStart = source.find("layout", bufferStart + 1);
			bufferEnd = source.find(";", bufferStart);
		}
		return counters;
	}
	
	static BufferLayout BufferLayoutFromVariables(const std::vector<ShaderVariable>& variables, uint32_t offset)
	{
		const uint32_t divisior = 1;
		uint32_t counter = offset;
		std::vector<BufferElement> elements;
		for (auto& variable : variables)
		{
			switch (variable.Type)
			{
			case XYZ::ShaderUniformDataType::None:
				break;
			case XYZ::ShaderUniformDataType::Int:
				elements.emplace_back(counter++, ShaderDataType::Int, variable.Name, divisior);
				break;
			case XYZ::ShaderUniformDataType::UInt:
				elements.emplace_back(counter++, ShaderDataType::Int, variable.Name, divisior);
				break;
			case XYZ::ShaderUniformDataType::Float:
				elements.emplace_back(counter++, ShaderDataType::Float, variable.Name, divisior);
				break;
			case XYZ::ShaderUniformDataType::Vec2:
				elements.emplace_back(counter++, ShaderDataType::Float2, variable.Name, divisior);
				break;
			case XYZ::ShaderUniformDataType::Vec3:
				elements.emplace_back(counter++, ShaderDataType::Float3, variable.Name, divisior);
				break;
			case XYZ::ShaderUniformDataType::Vec4:
				elements.emplace_back(counter++, ShaderDataType::Float4, variable.Name, divisior);
				break;
			}
		}
		return BufferLayout(elements);
	}

	ParticleMaterial::ParticleMaterial(uint32_t maxParticles, const Ref<Shader>& computeShader)
		:
		m_ComputeShader(computeShader),
		m_VertexArray(VertexArray::Create()),
		m_MaxParticles(maxParticles)
	{
		rebuild();
	}
	
	void ParticleMaterial::Compute() const
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.ElementCount)
				buffer.Storage->BindRange(0, buffer.ElementCount * buffer.ElementSize);
		}
		m_ComputeShader->SetVSUniforms(m_UniformBuffer);
		m_ComputeShader->Compute(32, 32, 1);
	}

	
	void ParticleMaterial::ResetCounters()
	{
		for (auto& counter : m_Counters)
		{
			counter.Atomic->Reset();
		}
	}

	void ParticleMaterial::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		rebuild();
	}

	void ParticleMaterial::SetComputeShader(const Ref<Shader>& computeShader)
	{
		m_ComputeShader = computeShader;
		rebuild();
	}

	void ParticleMaterial::SetParticleBuffersElementCount(uint32_t count)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.ParticleBuffer)
				buffer.ElementCount = count;
		}
	}

	void ParticleMaterial::SetBufferElementCount(const std::string& name, uint32_t count)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
				buffer.ElementCount = count;
		}
	}

	void ParticleMaterial::SetBufferSize(const std::string& name, uint32_t size)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
			{
				buffer.Storage->Resize(nullptr, size);
				return;
			}
		}
		XYZ_ASSERT(false, "No buffer with the name ", name);
	}
	void ParticleMaterial::SetBufferData(const std::string& name, void* data, uint32_t count, uint32_t elementSize, uint32_t offset)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
			{
				XYZ_ASSERT(elementSize == buffer.ElementSize, "Wrong element size");
				buffer.Storage->Update(data, count * elementSize, offset);
				return;
			}
		}
		XYZ_ASSERT(false, "No buffer with the name ", name);
	}

	void ParticleMaterial::GetBufferData(const std::string& name, void** data, uint32_t count, uint32_t offset)
	{
		for (auto& buffer : m_Buffers)
		{
			if (buffer.Name == name)
			{
				buffer.Storage->GetSubData(data, count * buffer.ElementSize, offset);
				return;
			}
		}
		XYZ_ASSERT(false, "No buffer with the name ", name);
	}

	void ParticleMaterial::parse()
	{
		std::ifstream in(m_ComputeShader->GetPath(), std::ios::in | std::ios::binary);
		if (in)
		{
			std::string source;
			in.seekg(0, std::ios::end);
			source.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&source[0], source.size());
			in.close();
			std::vector<ShaderStruct>  structs = std::move(ParseStructs(source));
			std::vector<ParShaderBuffer>  buffers = std::move(ParseShaderBuffers(source, structs));
			std::vector<ShaderCounter> counters = std::move(ParseShaderCounters(source));

			for (auto& buffer : buffers)
			{
				if (buffer.Indirect)
				{
					XYZ_ASSERT(!m_DrawCommand.Raw(), "Only one indirect buffer is supported");
					m_DrawCommand = IndirectBuffer::Create(nullptr, buffer.Size, buffer.Binding);
				}
				else
				{
					Buffer buf;
					buf.Name = buffer.Name;
					if (buffer.ParticleBuffer)
						buf.Storage = ShaderStorageBuffer::Create(buffer.Size * m_MaxParticles, buffer.Binding);
					else
						buf.Storage = ShaderStorageBuffer::Create(buffer.Size, buffer.Binding);

					buf.ElementSize = buffer.Size;
					buf.ElementCount = 0;
					buf.RenderBuffer = buffer.RenderBuffer;
					buf.ParticleBuffer = buffer.ParticleBuffer;

					m_Buffers.push_back(buf);
					if (buffer.RenderBuffer)
					{
						buf.Storage->SetLayout(BufferLayoutFromVariables(buffer.Variables, 1));
						m_VertexArray->AddShaderStorageBuffer(buf.Storage);
					}
				}
			}
			for (auto& counter : counters)
			{
				Counter cou;
				cou.Name = counter.Name;
				cou.Atomic = AtomicCounter::Create(1, counter.Binding);
				m_Counters.push_back(cou);
			}

			XYZ_ASSERT(m_DrawCommand.Raw(), "Compute shader does not define draw command");
		}
	}

	void ParticleMaterial::rebuild()
	{
		m_Buffers.clear();
		m_Counters.clear();
		m_DrawCommand.Reset();

		glm::vec3 quad[4] = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3( 0.5f, -0.5f, 0.0f),
			glm::vec3( 0.5f,  0.5f, 0.0f),
			glm::vec3(-0.5f,  0.5f, 0.0f)
		};
		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(quad, 4 * sizeof(glm::vec3));
		squareVBpar->SetLayout({
			{ 0, XYZ::ShaderDataType::Float3, "a_Position" }
			});
		m_VertexArray->AddVertexBuffer(squareVBpar);

		parse();
		m_UniformBuffer.Allocate(m_ComputeShader->GetVSUniformList().Size);
		m_UniformBuffer.ZeroInitialize();

		uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
		Ref<XYZ::IndexBuffer> squareIBpar;
		squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(squareIBpar);
	}

	void ParticleMaterial::resize()
	{
		for (auto& buffer : m_Buffers)
		{
			//if (buffer.ParticleBuffer)
			//	buffer.Storage->Resize()
		}
	}

	const ShaderUniform* ParticleMaterial::findUniform(const std::string& name) const
	{
		for (auto& uni : m_ComputeShader->GetVSUniformList().Uniforms)
		{
			if (uni.GetName() == name)
				return &uni;
		}
		return nullptr;
	}
}