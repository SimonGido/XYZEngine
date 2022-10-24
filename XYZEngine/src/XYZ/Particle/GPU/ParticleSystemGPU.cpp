#include "stdafx.h"
#include "ParticleSystemGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	
	std::string ParticleVariable::GLSLType() const
	{
		switch (Type)
		{
		case ParticleVariableType::None: break;
		case ParticleVariableType::Float: return "float";			
		case ParticleVariableType::Vec2:  return "vec2";
		case ParticleVariableType::Vec3:  return "vec3";
		case ParticleVariableType::Vec4:  return "vec4";

		case ParticleVariableType::Int:	  return "int";
		case ParticleVariableType::IVec2: return "ivec2";
		case ParticleVariableType::IVec3: return "ivec3";
		case ParticleVariableType::IVec4: return "ivec4";

		case ParticleVariableType::Bool:  return "bool";
		};

		XYZ_ASSERT(false, "Invalid type");
		return std::string();
	}

	ParticleSystemGPU::ParticleSystemGPU(ParticleSystemLayout inputLayout, ParticleSystemLayout outputLayout, uint32_t maxParticles)
		:
		m_InputLayout(std::move(inputLayout)),
		m_OutputLayout(std::move(outputLayout)),
		m_EmittedParticles(0)
	{
		m_ParticleBuffer.SetMaxParticles(maxParticles, m_InputLayout.GetStride());
	}
	uint32_t ParticleSystemGPU::Update(Timestep ts)
	{
		const uint32_t bufferOffset = m_EmittedParticles * GetInputStride();
		const uint32_t bufferSize   = m_ParticleBuffer.GetBufferSize() - bufferOffset;
		std::byte* particleBuffer   = m_ParticleBuffer.GetData(m_EmittedParticles);

		uint32_t emitted = 0;
		for (auto& emitter : ParticleEmitters)
		{
			emitted += emitter.Emit(ts, particleBuffer, bufferSize);
		}
		m_EmittedParticles += emitted;
		return emitted;
	}
	
	ParticleBuffer::ParticleBuffer(uint32_t maxParticles, uint32_t stride)
		:
		m_Stride(stride)
	{
		m_Data.resize(maxParticles * stride);
		memset(m_Data.data(), 0, m_Data.size());
	}

	void ParticleBuffer::SetMaxParticles(uint32_t maxParticles, uint32_t stride)
	{
		m_Stride = stride;
		m_Data.resize(maxParticles * stride);
		memset(m_Data.data(), 0, m_Data.size());
	}

	std::byte* ParticleBuffer::GetData(uint32_t particleOffset)
	{
		uint32_t offset = particleOffset * m_Stride;
		return &m_Data.data()[offset];
	}
	const std::byte* ParticleBuffer::GetData(uint32_t particleOffset) const
	{
		uint32_t offset = particleOffset * m_Stride;
		return &m_Data.data()[offset];
	}


	ParticleSystemGPUShaderGenerator::ParticleSystemGPUShaderGenerator(const Ref<ParticleSystemGPU>& particleSystem)
	{
		auto& outputLayout = particleSystem->GetOutputLayout();
		auto& inputLayout = particleSystem->GetInputLayout();

		auto& outputVariables = outputLayout.GetVariables();
		auto& inputVariables = inputLayout.GetVariables();

		m_SourceCode += "//#type compute\n";
		m_SourceCode += "#version 460\n";
		m_SourceCode += "#include \"Resources/Shaders/Includes/Math.glsl\"\n";

		addStruct("DrawCommand", {
			Variable{"uint", "Count", sizeof(uint32_t) },
			Variable{"uint", "InstanceCount", sizeof(uint32_t) },
			Variable{"uint", "FirstIndex", sizeof(uint32_t) },
			Variable{"uint", "BaseVertex", sizeof(uint32_t) },
			Variable{"uint", "BaseInstance", sizeof(uint32_t) }
		});


		addStruct(outputLayout.GetName(), particleVariablesToVariables(outputVariables));
		addStruct(inputLayout.GetName(), particleVariablesToVariables(inputVariables));

		addUniforms("Uniform", "u_Uniforms", {
			Variable{"uint",  "CommandCount"},
			Variable{"float", "Timestep"},
			Variable{"float", "Speed"},
			Variable{"uint",  "EmittedParticles"},
			Variable{"bool",  "Loop"},
		});

		addSSBO(5, "DrawCommand", { {"DrawCommand", "Command", 0, true} });
		addSSBO(6, outputLayout.GetName(), { {outputLayout.GetName(), "Outputs", 0, true} });
		addSSBO(7, inputLayout.GetName(), { {inputLayout.GetName(), "Inputs", 0, true} });

		addEntryPoint(32, 32, 1);
	}

	void ParticleSystemGPUShaderGenerator::addStruct(const std::string_view name, const std::vector<Variable>& variables)
	{
		uint32_t offset = 0;
		if (!variables.empty())
		{
			m_SourceCode += fmt::format("struct {}", name) + "\n{\n";
			for (auto& var : variables)
			{
				if (var.IsArray)
				{
					m_SourceCode += fmt::format("	{} {}[];\n", var.Type, var.Name);
				}
				else
				{
					m_SourceCode += fmt::format("	{} {};\n", var.Type, var.Name);
				}
				offset += var.Size;
			}

			uint32_t paddingSize = Math::RoundUp(offset, 16) - offset;
			if (paddingSize != 0)
			{
				m_SourceCode += fmt::format("	uint Padding[{}];\n", paddingSize / sizeof(uint32_t));
			}
			m_SourceCode += "};\n";
		}
	}

	void ParticleSystemGPUShaderGenerator::addSSBO(uint32_t binding, const std::string_view name, const std::vector<Variable>& variables)
	{
		m_SourceCode += fmt::format("layout(std430, binding = {}) buffer buffer_{}\n", binding, name);
		m_SourceCode += "{\n";
		for (const auto& var : variables)
		{
			if (var.IsArray)
			{
				m_SourceCode += fmt::format("	{} {}[];\n", var.Type, var.Name);
			}
			else
			{
				m_SourceCode += fmt::format("	{} {};\n", var.Type, var.Name);
			}
		}
		m_SourceCode += "\n};\n";
	}

	void ParticleSystemGPUShaderGenerator::addUniforms(const std::string_view name, const std::string_view declName, const std::vector<Variable>& variables)
	{
		m_SourceCode += fmt::format("layout(push_constant) uniform {}\n", name);
		m_SourceCode += "{\n";
		for (const auto& var : variables)
		{
			if (var.IsArray)
			{
				m_SourceCode += fmt::format("	{} {}[];\n", var.Type, var.Name);
			}
			else
			{
				m_SourceCode += fmt::format("	{} {};\n", var.Type, var.Name);
			}
		}
		m_SourceCode += "}";
		m_SourceCode += fmt::format("{};\n", declName);
	}

	void ParticleSystemGPUShaderGenerator::addEntryPoint(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		m_SourceCode += fmt::format("layout(local_size_x = {}, local_size_y = {}, local_size_z = {}) in;\n", groupX, groupY, groupZ);
		m_SourceCode +=
			"void main(void)\n"
			"{\n"
			"	uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;\n"																													   \
			"	if (id >= u_Uniforms.EmittedParticles)\n"
			"		return;\n"
			"	Update(id);\n"
			"}";
	}
	void ParticleSystemGPUShaderGenerator::addUpdate()
	{

	}

	std::vector<ParticleSystemGPUShaderGenerator::Variable> ParticleSystemGPUShaderGenerator::particleVariablesToVariables(const std::vector<ParticleVariable>& variables)
	{
		std::vector<Variable> tempVariables;
		tempVariables.reserve(variables.size());
		for (auto& var : variables)
			tempVariables.push_back(Variable{ var.GLSLType(), var.Name, var.Size, var.IsArray });

		return tempVariables;
	}
}