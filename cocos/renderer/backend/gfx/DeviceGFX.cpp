#include "DeviceGFX.h"
#include "RenderPipelineGFX.h"
#include "BufferGFX.h"
#include "ShaderModuleGFX.h"
#include "CommandBufferGFX.h"
#include "TextureGFX.h"
#include "DepthStencilStateGFX.h"
#include "ProgramGFX.h"
#include "DeviceInfoGFX.h"
#include "UtilsGFX.h"
#include "base/CCConfiguration.h"
#include "renderer/ccShaders.h"

CC_BACKEND_BEGIN

Device* Device::getInstance()
{
    if (!_instance)
        _instance = new (std::nothrow) DeviceGFX();
    return _instance;
}

void DeviceGFX::setSwapchainInfo(void* windowHandle, bool vsync, uint32_t width, uint32_t height)
{
	const auto instance = (DeviceGFX*)getInstance();
	CC_ASSERT(instance);
	if (instance)
	{
		instance->windowHandle = windowHandle;
		instance->vsync = vsync;
		instance->width = width;
		instance->height = height;
	}
}

bool DeviceGFX::isAvailable()
{
	return cc::gfx::Device::getInstance() != nullptr;
}

DeviceGFX::DeviceGFX()
{
    if (!cc::gfx::Device::getInstance())
    {
		ccMessageBox("Please call createDevice()", "Error");
		CC_ASSERT(false);
    }
    _deviceInfo = new (std::nothrow) DeviceInfoGFX();
    if(!_deviceInfo || _deviceInfo->init() == false)
    {
        delete _deviceInfo;
        _deviceInfo = nullptr;
    }
}

DeviceGFX::~DeviceGFX()
{
    ProgramCache::destroyInstance();
    delete _deviceInfo;
    _deviceInfo = nullptr;
}

CommandBuffer* DeviceGFX::newCommandBuffer()
{
    return new (std::nothrow) CommandBufferGFX();
}

Buffer* DeviceGFX::newBuffer(std::size_t size, BufferType type, BufferUsage usage)
{
    return new (std::nothrow) BufferGFX(size, type, usage);
}

Buffer* DeviceGFX::newBuffer(uint32_t size, uint32_t stride, BufferType type, BufferUsage usage)
{
	cc::gfx::BufferInfo info;
	switch (type)
	{
	case BufferType::VERTEX:
		info.usage = cc::gfx::BufferUsageBit::VERTEX;
		break;
	case BufferType::INDEX:
		info.usage = cc::gfx::BufferUsageBit::INDEX;
		break;
	default:;
	}
	info.memUsage = UtilsGFX::toMemoryUsage(usage);
	info.size = size;
	info.stride = stride;
	return new (std::nothrow) BufferGFX(info);
}

TextureBackend* DeviceGFX::newTexture(const TextureDescriptor& descriptor)
{
    switch (descriptor.textureType)
    {
    case TextureType::TEXTURE_2D:
        return new (std::nothrow) Texture2DGFX(descriptor);
    case TextureType::TEXTURE_CUBE:
        return new (std::nothrow) TextureCubeGFX(descriptor);
    default:
        return nullptr;
    }
}

ShaderModule* DeviceGFX::newShaderModule(ShaderStage stage, const std::string& source)
{
    return new (std::nothrow) ShaderModuleGFX(stage, source);
}

DepthStencilState* DeviceGFX::createDepthStencilState(const DepthStencilDescriptor& descriptor)
{
    auto ret = new (std::nothrow) DepthStencilStateGFX(descriptor);
    if (ret)
        ret->autorelease();
    return ret;
}

RenderPipeline* DeviceGFX::newRenderPipeline()
{
    return new (std::nothrow) RenderPipelineGFX();
}

static std::unordered_map<std::size_t, cc::gfx::AttributeList> BuiltinShaderAttributes;
static std::unordered_map<std::size_t, cc::gfx::UniformList> BuiltinShaderUniforms;
static std::unordered_map<std::size_t, cc::gfx::UniformList> BuiltinShaderLightCommonUniforms;
static std::unordered_map<std::size_t, cc::gfx::UniformSamplerTextureList> BuiltinShaderTextures;
static std::unordered_map<std::size_t, std::string> BuiltinShaderNames;

Program* DeviceGFX::newProgram(const std::string& vertexShader, const std::string& fragmentShader)
{
	initBuiltinShaderInfo();
	cc::gfx::ShaderInfo info;
	cc::gfx::ShaderStage stage;
	stage.source = vertexShader;
	stage.stage = cc::gfx::ShaderStageFlagBit::VERTEX;
	info.stages.emplace_back(stage);
	stage.source = fragmentShader;
	stage.stage = cc::gfx::ShaderStageFlagBit::FRAGMENT;
	info.stages.emplace_back(stage);

	const auto vertKey = std::hash<std::string>{}(vertexShader);
	const auto fragKey = std::hash<std::string>{}(fragmentShader);

	const auto it1 = BuiltinShaderAttributes.find(vertKey);
	if (it1 != BuiltinShaderAttributes.end())
		info.attributes = it1->second;
	auto it2 = BuiltinShaderUniforms.find(vertKey);
	if (it2 != BuiltinShaderUniforms.end() && !it2->second.empty())
	{
		cc::gfx::UniformBlock block;
		block.name = "VSBlock";
		block.members = it2->second;
		block.binding = 0;
		info.blocks.emplace_back(block);
	}
	it2 = BuiltinShaderUniforms.find(fragKey);
	if (it2 != BuiltinShaderUniforms.end() && !it2->second.empty())
	{
		cc::gfx::UniformBlock block;
		block.name = "FSBlock";
		block.members = it2->second;
		block.binding = 1;
		info.blocks.emplace_back(block);
	}
	it2 = BuiltinShaderLightCommonUniforms.find(fragKey);
	if (it2 != BuiltinShaderLightCommonUniforms.end() && !it2->second.empty())
	{
		cc::gfx::UniformBlock block;
		block.name = "CommonLightBlock";
		block.members = it2->second;
		block.binding = 2;
		info.blocks.emplace_back(block);
	}
	const auto it3 = BuiltinShaderTextures.find(fragKey);
	if (it3 != BuiltinShaderTextures.end())
		info.samplerTextures = it3->second;
	if (info.blocks.size() > 1)
	{
		info.name = "[Builtin]";
		if (BuiltinShaderNames.count(vertKey))
			info.name += " " + BuiltinShaderNames.at(vertKey);
		if (BuiltinShaderNames.count(fragKey))
			info.name += " " + BuiltinShaderNames.at(fragKey);
	}
	auto ret = new (std::nothrow) ProgramGFX(info);
	if (!ret->isValid())
	{
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

struct BuiltinShaderHelper
{
	cc::gfx::AttributeList* alist = nullptr;
	cc::gfx::UniformList* ulist = nullptr;
	cc::gfx::UniformSamplerTextureList* tlist = nullptr;
	std::size_t key = 0;
	BuiltinShaderHelper& set(const std::string& str)
	{
		key = std::hash<std::string>{}(str);
		alist = &(BuiltinShaderAttributes[key] = {});
		ulist = &(BuiltinShaderUniforms[key] = {});
		tlist = &(BuiltinShaderTextures[key] = {});
		return *this;
	}
	BuiltinShaderHelper& attr(
		const std::string & name,
		VertexFormat format,
		bool isNormalized = false,
		int32_t location = -1)
	{
		cc::gfx::Attribute attr;
		attr.name = name;
		attr.format = UtilsGFX::toAttributeType(format);
		attr.isNormalized = isNormalized;
		if (location < 0)
			location = alist->size();
		attr.location = location;
		alist->emplace_back(attr);
		return *this;
	}
	BuiltinShaderHelper& uniform(
		const std::string & name,
		cc::gfx::Type type,
		uint32_t count = 0)
	{
		cc::gfx::Uniform uniform;
		uniform.name = name;
		uniform.type = type;
		uniform.count = count;
		ulist->emplace_back(uniform);
		return *this;
	}
	BuiltinShaderHelper& texture(
		const std::string & name = "u_texture",
		cc::gfx::Type type = cc::gfx::Type::SAMPLER2D,
		uint32_t binding = 2)
	{
		cc::gfx::UniformSamplerTexture uniform;
		uniform.name = name;
		uniform.type = type;
		uniform.count = 1;
		uniform.binding = binding;
		tlist->emplace_back(uniform);
		return *this;
	}
	BuiltinShaderHelper& name(
		const std::string & name)
	{
		BuiltinShaderNames[key] = name;
		return *this;
	}
};

static std::string getShaderMacrosForLight()
{
	char def[256];
	auto conf = Configuration::getInstance();

	snprintf(def, sizeof(def) - 1, "\n#define MAX_DIRECTIONAL_LIGHT_NUM %d \n"
		"\n#define MAX_POINT_LIGHT_NUM %d \n"
		"\n#define MAX_SPOT_LIGHT_NUM %d \n",
		conf->getMaxSupportDirLightInShader(),
		conf->getMaxSupportPointLightInShader(),
		conf->getMaxSupportSpotLightInShader());

	return std::string(def);
}

void DeviceGFX::initBuiltinShaderInfo()
{
	if (!BuiltinShaderAttributes.empty())
		return;
	using cc::gfx::Type;
	auto helper = BuiltinShaderHelper();
	// attributes here only indicates shader info
	helper.set(positionColor_vert).name("positionColor_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_color", VertexFormat::FLOAT4)
		.uniform("u_MVPMatrix", Type::MAT4);
	//helper.set(positionColor_frag).name("positionColor_frag");
	helper.set(positionTexture_vert).name("positionTexture_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(positionTexture_frag).name("positionTexture_frag")
		.texture();
	helper.set(positionTextureColor_vert).name("positionTextureColor_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_color", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(positionTextureColor_frag).name("positionTextureColor_frag")
		.texture();
	helper.set(positionTextureColorAlphaTest_frag).name("positionTextureColorAlphaTest_frag")
		.uniform("u_alpha_value", Type::FLOAT)
		.texture();
	helper.set(label_normal_frag).name("label_normal_frag")
		.uniform("u_textColor", Type::FLOAT4)
		.texture();
	helper.set(label_distanceNormal_frag).name("label_distanceNormal_frag")
		.uniform("u_textColor", Type::FLOAT4)
		.texture();
	helper.set(labelOutline_frag).name("labelOutline_frag")
		.uniform("u_effectColor", Type::FLOAT4)
		.uniform("u_textColor", Type::FLOAT4)
		.uniform("u_effectType", Type::INT)
		.texture();
	helper.set(labelDistanceFieldGlow_frag).name("labelDistanceFieldGlow_frag")
		.uniform("u_effectColor", Type::FLOAT4)
		.uniform("u_textColor", Type::FLOAT4)
		.texture();
	//helper.set(lineColor3D_frag).name("lineColor3D_frag");
	helper.set(lineColor3D_vert).name("lineColor3D_vert")
		.attr("a_position", VertexFormat::FLOAT3)
		.attr("a_color", VertexFormat::FLOAT4)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(positionColorLengthTexture_vert).name("positionColorLengthTexture_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_color", VertexFormat::FLOAT4)
		.uniform("u_alpha", Type::FLOAT)
		.uniform("u_MVPMatrix", Type::MAT4);
	//helper.set(positionColorLengthTexture_frag).name("positionColorLengthTexture_frag");
	helper.set(positionColorTextureAsPointsize_vert).name("positionColorTextureAsPointsize_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_color", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_alpha", Type::FLOAT)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(position_vert).name("position_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(positionNoMVP_vert).name("positionNoMVP_vert")
		.attr("a_position", VertexFormat::FLOAT4);
	helper.set(layer_radialGradient_frag).name("layer_radialGradient_frag")
		.uniform("u_startColor", Type::FLOAT4)
		.uniform("u_endColor", Type::FLOAT4)
		.uniform("u_center", Type::FLOAT2)
		.uniform("u_radius", Type::FLOAT)
		.uniform("u_expand", Type::FLOAT);
	helper.set(grayScale_frag).name("grayScale_frag")
		.texture();
	helper.set(positionTextureUColor_vert).name("positionTextureUColor_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(positionTextureUColor_frag).name("positionTextureUColor_frag")
		.uniform("u_color", Type::FLOAT4)
		.texture();
	helper.set(positionUColor_vert).name("positionUColor_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.uniform("u_color", Type::FLOAT4)
		.uniform("u_MVPMatrix", Type::MAT4);
	//helper.set(positionUColor_frag).name("positionUColor_frag");
	helper.set(etc1_frag).name("etc1_frag")
		.texture("u_texture", Type::SAMPLER2D, 2)
		.texture("u_texture1", Type::SAMPLER2D, 3);
	helper.set(etc1Gray_frag).name("etc1Gray_frag")
		.texture("u_texture", Type::SAMPLER2D, 2)
		.texture("u_texture1", Type::SAMPLER2D, 3);
	helper.set(cameraClear_vert).name("cameraClear_vert")
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_color", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("depth", Type::FLOAT);
	//helper.set(cameraClear_frag).name("cameraClear_frag");
	helper.set(CC3D_color_frag).name("CC3D_color_frag")
		.uniform("u_color", Type::FLOAT4);

	std::string def = getShaderMacrosForLight();
	std::string normalMapDef = "\n#define USE_NORMAL_MAPPING 1 \n";
	const auto nDir = Configuration::getInstance()->getMaxSupportDirLightInShader();
	const auto nPoint = Configuration::getInstance()->getMaxSupportPointLightInShader();
	const auto nSpot = Configuration::getInstance()->getMaxSupportSpotLightInShader();
	const int SKINNING_JOINT_COUNT = 60;

	helper.set(def + CC3D_colorNormal_frag);
	if (nDir > 0)
		helper.uniform("u_DirLightSourceColor", Type::FLOAT3, nDir);
		//.uniform("u_DirLightSourceDirection", Type::FLOAT3, nDir);
	if (nPoint > 0)
		helper.uniform("u_PointLightSourceColor", Type::FLOAT3, nPoint)
		.uniform("u_PointLightSourceRangeInverse", Type::FLOAT, nPoint);
	if (nSpot > 0)
		helper.uniform("u_SpotLightSourceColor", Type::FLOAT3, nSpot)
		//.uniform("u_SpotLightSourceDirection", Type::FLOAT3, nSpot)
		.uniform("u_SpotLightSourceInnerAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceOuterAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceRangeInverse", Type::FLOAT, nSpot);
	helper.uniform("u_AmbientLightSourceColor", Type::FLOAT3)
		.uniform("u_color", Type::FLOAT4);

	helper.set(def + CC3D_colorNormalTexture_frag);
	if (nDir > 0)
		helper.uniform("u_DirLightSourceColor", Type::FLOAT3, nDir);
		//.uniform("u_DirLightSourceDirection", Type::FLOAT3, nDir);
	if (nPoint > 0)
		helper.uniform("u_PointLightSourceColor", Type::FLOAT3, nPoint)
		.uniform("u_PointLightSourceRangeInverse", Type::FLOAT, nPoint);
	if (nSpot > 0)
		helper.uniform("u_SpotLightSourceColor", Type::FLOAT3, nSpot)
		//.uniform("u_SpotLightSourceDirection", Type::FLOAT3, nSpot)
		.uniform("u_SpotLightSourceInnerAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceOuterAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceRangeInverse", Type::FLOAT, nSpot);
	helper.uniform("u_AmbientLightSourceColor", Type::FLOAT3)
		.uniform("u_color", Type::FLOAT4)
		.texture("u_texture", Type::SAMPLER2D, 3);

	helper.set(def + normalMapDef + CC3D_colorNormalTexture_frag);
	if (nDir > 0)
		helper.uniform("u_DirLightSourceColor", Type::FLOAT3, nDir);
		//.uniform("u_DirLightSourceDirection", Type::FLOAT3, nDir);
	if (nPoint > 0)
		helper.uniform("u_PointLightSourceColor", Type::FLOAT3, nPoint)
		.uniform("u_PointLightSourceRangeInverse", Type::FLOAT, nPoint);
	if (nSpot > 0)
		helper.uniform("u_SpotLightSourceColor", Type::FLOAT3, nSpot)
		//.uniform("u_SpotLightSourceDirection", Type::FLOAT3, nSpot)
		.uniform("u_SpotLightSourceInnerAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceOuterAngleCos", Type::FLOAT, nSpot)
		.uniform("u_SpotLightSourceRangeInverse", Type::FLOAT, nSpot);
	helper.uniform("u_AmbientLightSourceColor", Type::FLOAT3)
		.uniform("u_color", Type::FLOAT4)
		.texture("u_normalTex", Type::SAMPLER2D, 4)
		.texture("u_texture", Type::SAMPLER2D, 3);

	helper.set(CC3D_colorTexture_frag)
		.uniform("u_color", Type::FLOAT4)
		.texture();

	helper.set(CC3D_particleTexture_frag)
		.uniform("u_color", Type::FLOAT4)
		.texture();

	helper.set(CC3D_particleColor_frag)
		.uniform("u_color", Type::FLOAT4);

	helper.set(CC3D_particle_vert)
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_color", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_PMatrix", Type::MAT4);

	helper.set(def + CC3D_positionNormalTexture_vert);
	helper.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_normal", VertexFormat::FLOAT3)
		.uniform("u_MVPMatrix", Type::MAT4)
		.uniform("u_MVMatrix", Type::MAT4)
		.uniform("u_PMatrix", Type::MAT4)
		.uniform("u_NormalMatrix", Type::MAT3);

	helper.set(def + normalMapDef + CC3D_positionNormalTexture_vert);
	helper.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_normal", VertexFormat::FLOAT3)
		.attr("a_tangent", VertexFormat::FLOAT3)
		.attr("a_binormal", VertexFormat::FLOAT3)
		.uniform("u_MVPMatrix", Type::MAT4)
		.uniform("u_MVMatrix", Type::MAT4)
		.uniform("u_PMatrix", Type::MAT4)
		.uniform("u_NormalMatrix", Type::MAT3);

	helper.set(def + CC3D_skinPositionNormalTexture_vert);
	helper.attr("a_position", VertexFormat::FLOAT3)
		.attr("a_blendWeight", VertexFormat::FLOAT4)
		.attr("a_blendIndex", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_normal", VertexFormat::FLOAT3)
		.uniform("u_matrixPalette", Type::FLOAT4, SKINNING_JOINT_COUNT * 3)
		.uniform("u_MVMatrix", Type::MAT4)
		.uniform("u_PMatrix", Type::MAT4)
		.uniform("u_NormalMatrix", Type::MAT3);

	helper.set(def + normalMapDef + CC3D_skinPositionNormalTexture_vert);
	helper.attr("a_position", VertexFormat::FLOAT3)
		.attr("a_blendWeight", VertexFormat::FLOAT4)
		.attr("a_blendIndex", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_normal", VertexFormat::FLOAT3)
		.attr("a_tangent", VertexFormat::FLOAT3)
		.attr("a_binormal", VertexFormat::FLOAT3)
		.uniform("u_matrixPalette", Type::FLOAT4, SKINNING_JOINT_COUNT * 3)
		.uniform("u_MVMatrix", Type::MAT4)
		.uniform("u_PMatrix", Type::MAT4)
		.uniform("u_NormalMatrix", Type::MAT3);

	helper.set(CC3D_positionTexture_vert)
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(CC3D_skinPositionTexture_vert)
		.attr("a_position", VertexFormat::FLOAT3)
		.attr("a_blendWeight", VertexFormat::FLOAT4)
		.attr("a_blendIndex", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.uniform("u_matrixPalette", Type::FLOAT4, SKINNING_JOINT_COUNT * 3)
		.uniform("u_MVPMatrix", Type::MAT4);
	helper.set(CC3D_skybox_frag)
		.texture("u_Env", Type::SAMPLER_CUBE, 2)
		.uniform("u_color", Type::FLOAT4);
	helper.set(CC3D_skybox_vert)
		.attr("a_position", VertexFormat::FLOAT3)
		.uniform("u_cameraRot", Type::MAT4);
	helper.set(CC3D_terrain_frag)
		.uniform("u_color", Type::FLOAT3)
		.uniform("u_has_alpha", Type::INT)
		.uniform("u_has_light_map", Type::INT)
		.uniform("u_detailSize", Type::FLOAT, 4)
		.uniform("u_lightDir", Type::FLOAT3)
		.texture("u_alphaMap", Type::SAMPLER2D, 2)
		.texture("u_texture0", Type::SAMPLER2D, 3)
		.texture("u_texture1", Type::SAMPLER2D, 4)
		.texture("u_texture2", Type::SAMPLER2D, 5)
		.texture("u_texture3", Type::SAMPLER2D, 6)
		.texture("u_lightMap", Type::SAMPLER2D, 7);
	helper.set(CC3D_terrain_vert)
		.attr("a_position", VertexFormat::FLOAT4)
		.attr("a_texCoord", VertexFormat::FLOAT2)
		.attr("a_normal", VertexFormat::FLOAT3)
		.uniform("u_MVPMatrix", Type::MAT4);

	const std::string CommonLightBlock = ".CommonLightBlock";
	const auto CommonLightBlockKey = std::hash<std::string>{}(CommonLightBlock);
	helper.set(CommonLightBlock);
	if (nDir > 0)
		helper.uniform("u_DirLightSourceDirection", Type::FLOAT3, nDir);
	if (nPoint > 0)
		helper.uniform("u_PointLightSourcePosition", Type::FLOAT3, nPoint);
	if (nSpot > 0)
		helper.uniform("u_SpotLightSourcePosition", Type::FLOAT3, nSpot)
		.uniform("u_SpotLightSourceDirection", Type::FLOAT3, nSpot);

	BuiltinShaderLightCommonUniforms[std::hash<std::string>{}(def + CC3D_colorNormal_frag)] =
		BuiltinShaderUniforms[CommonLightBlockKey];
	BuiltinShaderLightCommonUniforms[std::hash<std::string>{}(def + CC3D_colorNormalTexture_frag)] =
		BuiltinShaderUniforms[CommonLightBlockKey];
	BuiltinShaderLightCommonUniforms[std::hash<std::string>{}(def + normalMapDef + CC3D_colorNormalTexture_frag)] =
		BuiltinShaderUniforms[CommonLightBlockKey];
}

CC_BACKEND_END
