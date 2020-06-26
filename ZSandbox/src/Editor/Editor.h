#pragma once

#include "XYZ.h"

class Editor : public XYZ::Layer
{
	
public:
	Editor();
	~Editor();

	virtual void OnImGuiRender() override;
};