#include "stdafx.h"
#include "InputField.h"

#include "Text.h"

namespace XYZ {
    InputField::InputField(const glm::vec4& selectColor, const glm::vec4& hooverColor, uint32_t entity, ECSManager* ecs)
        :
        SelectColor(selectColor), HooverColor(hooverColor), TextEntity(entity), ECS(ecs)
    {
        State<InputFieldState::NumStates>& selectState = Machine.CreateState();
        State<InputFieldState::NumStates>& releaseState = Machine.CreateState();
        State<InputFieldState::NumStates>& hooverState = Machine.CreateState();
        State<InputFieldState::NumStates>& unHooverState = Machine.CreateState();

        selectState.AllowTransition(releaseState.GetID());

        releaseState.AllowTransition(selectState.GetID());
        releaseState.AllowTransition(hooverState.GetID());
        releaseState.AllowTransition(unHooverState.GetID());
       
        hooverState.AllowTransition(selectState.GetID());
        hooverState.AllowTransition(unHooverState.GetID());
        hooverState.AllowTransition(hooverState.GetID());
       
        unHooverState.AllowTransition(hooverState.GetID());
       
        Machine.SetDefaultState(unHooverState.GetID());
    }


    template <>
    bool InputField::GetValueAs<std::string>(const InputField& field, std::string& value)
    {
        if (field.ECS && field.ECS->Contains<Text>(field.TextEntity))
        {
            value = field.ECS->GetStorageComponent<Text>(field.TextEntity).Source;
            return true;
        }
        return false;
    }

    template <>
    bool InputField::GetValueAs<float>(const InputField& field, float& value)
    {
        if (field.ECS && field.ECS->Contains<Text>(field.TextEntity))
        {
            value = (float)atof(field.ECS->GetStorageComponent<Text>(field.TextEntity).Source.c_str());
            return true;
        }
        return false;
    }

    template <>
    bool InputField::GetValueAs<int>(const InputField& field, int& value)
    {
        if (field.ECS && field.ECS->Contains<Text>(field.TextEntity))
        {
            value = atoi(field.ECS->GetStorageComponent<Text>(field.TextEntity).Source.c_str());
            return true;
        }
        return false;
    }
}