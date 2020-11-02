#include "Inspectable.h"


namespace XYZ {
	void Inspectable::SetupMaterialValuesLengths(const std::vector<Uniform>& uniforms, std::vector<int32_t>& lengths, std::vector<int32_t>& selected)
	{
		for (auto& uniform : uniforms)
		{
			selected.push_back(-1);
			switch (uniform.Type)
			{
			case UniformDataType::FLOAT:
				lengths.push_back(5);
				break;
			case UniformDataType::FLOAT_VEC2:
				for (int32_t i = 0; i < 2; ++i)
					lengths.push_back(5);
				break;
			case UniformDataType::FLOAT_VEC3:
				for (int32_t i = 0; i < 3; ++i)
					lengths.push_back(5);
				break;
			case UniformDataType::FLOAT_VEC4:
				for (int32_t i = 0; i < 4; ++i)
					lengths.push_back(5);
				break;
			case UniformDataType::INT:
				lengths.push_back(5);
				break;
			case UniformDataType::FLOAT_MAT4:
				break;
			};
		}
	}
	void Inspectable::ShowUniforms(const uint8_t* buffer, const std::vector<Uniform>& uniforms, std::vector<int32_t>& lengths, std::vector<int32_t>& selected)
	{
		uint32_t countLengths = 0;
		uint32_t countSelected = 0;
		for (auto& uniform : uniforms)
		{
			switch (uniform.Type)
			{
			case UniformDataType::FLOAT:
				InGui::Float(1, uniform.Name.c_str(), (float*)&buffer[uniform.Offset], &lengths[countLengths], {}, { 50.0f, 25.0f }, selected[countSelected]);
				InGui::Separator();
				countLengths++;
				break;
			case UniformDataType::FLOAT_VEC2:
				InGui::Float(2, uniform.Name.c_str(), (float*)&buffer[uniform.Offset], &lengths[countLengths], {}, { 50.0f, 25.0f }, selected[countSelected]);
				InGui::Separator();
				countLengths += 2;
				break;
			case UniformDataType::FLOAT_VEC3:
				InGui::Float(3, uniform.Name.c_str(), (float*)&buffer[uniform.Offset], &lengths[countLengths], {}, { 50.0f, 25.0f }, selected[countSelected]);
				InGui::Separator();
				countLengths += 3;
				break;
			case UniformDataType::FLOAT_VEC4:
				InGui::Float(4, uniform.Name.c_str(), (float*)&buffer[uniform.Offset], &lengths[countLengths], {}, { 50.0f, 25.0f }, selected[countSelected]);
				InGui::Separator();
				countLengths += 4;
				break;
			case UniformDataType::INT:
				InGui::Int(1, uniform.Name.c_str(), (int*)&buffer[uniform.Offset], &lengths[countLengths], {}, { 50.0f, 25.0f }, selected[countSelected]);
				InGui::Separator();
				countLengths++;
				break;
			case UniformDataType::FLOAT_MAT4:
				break;
			};

			countSelected++;
		}
	}
}