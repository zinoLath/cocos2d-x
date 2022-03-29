#pragma once
#include "renderer/backend/RenderPipeline.h"
#include "renderer/backend/RenderPipelineDescriptor.h"
#include "gfx/backend/GFXDeviceManager.h"
#include <vector>

CC_BACKEND_BEGIN

class ProgramGFX;

class RenderPipelineGFX : public RenderPipeline
{
public:
	RenderPipelineGFX() = default;
	~RenderPipelineGFX();

	void update(
		const PipelineDescriptor& pipelineDescirptor,
		const RenderPassDescriptor& renderpassDescriptor) override;

	void doUpdate(cc::gfx::PipelineStateInfo* psinfo);

	ProgramGFX* getProgram() const { return _programGFX; }

private:
	ProgramGFX* _programGFX = nullptr;
	BlendDescriptor _blendDescriptor;
};

CC_BACKEND_END
