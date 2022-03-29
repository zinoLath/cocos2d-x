#include "RenderPipelineGFX.h"
#include "ShaderModuleGFX.h"
#include "DepthStencilStateGFX.h"
#include "ProgramGFX.h"
#include "UtilsGFX.h"

using namespace cc;

CC_BACKEND_BEGIN

RenderPipelineGFX::~RenderPipelineGFX()
{
	CC_SAFE_RELEASE(_programGFX);
}

void RenderPipelineGFX::update(
	const PipelineDescriptor& pipelineDescirptor,
	const RenderPassDescriptor& renderpassDescriptor)
{
	if (_programGFX != pipelineDescirptor.programState->getProgram())
	{
		CC_SAFE_RELEASE(_programGFX);
		_programGFX = static_cast<ProgramGFX*>(pipelineDescirptor.programState->getProgram());
		CC_SAFE_RETAIN(_programGFX);
	}
	// save
	_blendDescriptor = pipelineDescirptor.blendDescriptor;
}

void RenderPipelineGFX::doUpdate(gfx::PipelineStateInfo* psinfo)
{
	if (!psinfo)
		return;
	auto& target = psinfo->blendState.targets.at(0);
	auto& descriptor = _blendDescriptor;
	if (descriptor.blendEnabled)
	{
		target.blend = 1;
		target.blendSrc = UtilsGFX::toBlendFactor(descriptor.sourceRGBBlendFactor);
		target.blendDst = UtilsGFX::toBlendFactor(descriptor.destinationRGBBlendFactor);
		target.blendEq = UtilsGFX::toBlendOperation(descriptor.rgbBlendOperation);
		target.blendSrcAlpha = UtilsGFX::toBlendFactor(descriptor.sourceAlphaBlendFactor);
		target.blendDstAlpha = UtilsGFX::toBlendFactor(descriptor.destinationAlphaBlendFactor);
		target.blendAlphaEq = UtilsGFX::toBlendOperation(descriptor.alphaBlendOperation);
	}
	else
	{
		target.blend = 0;
	}

	const auto writeMaskRed = (uint32_t)descriptor.writeMask & (uint32_t)ColorWriteMask::RED;
	const auto writeMaskGreen = (uint32_t)descriptor.writeMask & (uint32_t)ColorWriteMask::GREEN;
	const auto writeMaskBlue = (uint32_t)descriptor.writeMask & (uint32_t)ColorWriteMask::BLUE;
	const auto writeMaskAlpha = (uint32_t)descriptor.writeMask & (uint32_t)ColorWriteMask::ALPHA;

	target.blendColorMask = gfx::ColorMask::NONE;
	if (writeMaskRed)
		target.blendColorMask |= gfx::ColorMask::R;
	if (writeMaskGreen)
		target.blendColorMask |= gfx::ColorMask::G;
	if (writeMaskBlue)
		target.blendColorMask |= gfx::ColorMask::B;
	if (writeMaskAlpha)
		target.blendColorMask |= gfx::ColorMask::A;
}

CC_BACKEND_END