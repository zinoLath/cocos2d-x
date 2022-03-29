#pragma once

#include "renderer/backend/Macros.h"
#include "renderer/backend/Types.h"
#include "renderer/backend/RenderPipelineDescriptor.h"
#include "base/CCRef.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCMap.h"
#include "base/CCVector.h"
#include "renderer/backend/Program.h"
#include "gfx/backend/GFXDeviceManager.h"
#include "glslang/glslang/Public/ShaderLang.h"
#include "BufferGFX.h"

#include <string>
#include <vector>
#include <unordered_map>


CC_BACKEND_BEGIN

class ShaderModuleGFX;
class ProgramStateGFX;

class ProgramGFX final : public Program
{
public:

	ProgramGFX(const cc::gfx::ShaderInfo& shaderInfo);

	~ProgramGFX();

	/**
	 * Get program object.
	 * @return Program object.
	 */
	cc::gfx::Shader* getHandler() const { return _program; }
	bool isValid() const { return getHandler(); }

	const cc::gfx::ShaderInfo& getShaderInfo() const { return _info; }

	/**
	 * Get uniform location by name.
	 * @param uniform Specifies the uniform name.
	 * @return The uniform location.
	 */
	UniformLocation getUniformLocation(const std::string& uniform) const override;

	/**
	 * Get uniform location by engine built-in uniform enum name.
	 * @param name Specifies the engine built-in uniform enum name.
	 * @return The uniform location.
	 */
	UniformLocation getUniformLocation(backend::Uniform name) const override;

	/**
	 * Get attribute location by attribute name.
	 * @param name Specifies the attribute name.
	 * @return The attribute location.
	 */
	int getAttributeLocation(const std::string& name) const override;

	/**
	 * Get attribute location by engine built-in attribute enum name.
	 * @param name Specifies the engine built-in attribute enum name.
	 * @return The attribute location.
	 */
	int getAttributeLocation(Attribute name) const override;

	/**
	 * Get maximum vertex location.
	 * @return Maximum vertex locaiton.
	 */
	int getMaxVertexLocation() const override;

	/**
	 * Get maximum fragment location.
	 * @return Maximum fragment location.
	 */
	int getMaxFragmentLocation() const override;

	/**
	 * Get active vertex attributes.
	 * @return Active vertex attributes. key is active attribute name, Value is corresponding attribute info.
	 */
	const std::unordered_map<std::string, AttributeBindInfo> getActiveAttributes() const override;

	//
	std::size_t getUniformBufferSize(ShaderStage stage) const override;
	std::size_t getUniformBlockSize(const std::string& blockName) const;

	/**
	 * Get a uniformInfo in given location from the specific shader stage.
	 * @param stage Specifies the shader stage. The symbolic constant can be either VERTEX or FRAGMENT.
	 * @param location Specifies the uniform locaion.
	 * @return The uniformInfo.
	 */
	const UniformInfo& getActiveUniformInfo(ShaderStage stage, int location) const override;

	/**
	 * Get all uniformInfos.
	 * @return The uniformInfos.
	 */
	const std::unordered_map<std::string, UniformInfo>& getAllActiveUniformInfo(ShaderStage stage) const override;

	struct BlockInfoEx
	{
		std::string name;
		uint32_t size = 0; // buffer size
		size_t index = 0; // index in list
		std::unordered_map<std::string, UniformInfo> members;
	};

	const std::unordered_map<std::string, BlockInfoEx>& getBlockInfoEx() const { return blockInfoEx; }
	// get default (set=0)
	const cc::gfx::DescriptorSetLayoutBindingList& getUniformLayoutBindings() const { return uniformLayoutBindings; }
	cc::gfx::DescriptorSetLayoutBindingList getUniformLayoutBindings(uint32_t set) const;

	cc::gfx::DescriptorSetLayout* createDescriptorSetLayout(
		const cc::gfx::DescriptorSetLayoutBindingList& extraBindings,
		uint32_t set = 0);
	cc::gfx::DescriptorSetLayout* getDefaultDescriptorSetLayout() const { return defaultDescriptorSetLayout; }
	cc::gfx::PipelineLayout* getDefaultPipelineLayout();

	ProgramStateGFX* getState(ProgramState* key) { return _states.at(key); }

protected:

#if CC_ENABLE_CACHE_TEXTURE_DATA
	/**
	 * In case of EGL context lost, the engine will reload shaders. Thus location of uniform may changed.
	 * The engine will maintain the relationship between the original uniform location and the current active uniform location.
	 */
	virtual int getMappedLocation(int location) const { return location; }
	virtual int getOriginalLocation(int location) const { return location; }

	/**
	 * Get all uniform locations.
	 * @return All uniform locations.
	 */
	virtual const std::unordered_map<std::string, int> getAllUniformsLocation() const { return {}; }
#endif
	
	static void initDefaultInfo();
	static ShaderModuleGFX* getShaderModule(const cc::gfx::ShaderStage& stage);
	static void computeShaderInfo(
		glslang::TProgram* program,
		cc::gfx::ShaderInfo& info,
		std::unordered_map<std::string, BlockInfoEx>& blockInfoEx);
	static void updateBlockInfoEx(
		const cc::gfx::ShaderInfo& info,
		std::unordered_map<std::string, BlockInfoEx>& blockInfoEx);
	static void fixForGLES2(cc::gfx::ShaderInfo& info);
	static cc::gfx::DescriptorSetLayoutBindingList computeUniformLayoutBindings(
		const cc::gfx::ShaderInfo& info, uint32_t set = 0);
	bool compileSpirv();

	cc::gfx::Shader* _program = nullptr;
	cc::gfx::ShaderInfo _info;
	cocos2d::Vector<ShaderModuleGFX*> modules;
	glslang::TProgram* glslangProgram = nullptr;
	size_t totalUniformSize = 0;
	cc::gfx::DescriptorSetLayoutBindingList uniformLayoutBindings;
	cc::gfx::DescriptorSetLayout* defaultDescriptorSetLayout = nullptr;
	cc::gfx::PipelineLayout* defaultPipelineLayout = nullptr;
	std::unordered_map<cc::gfx::ShaderStageFlagBit, std::vector<uint32_t>> spirvCodes;
	std::unordered_map<std::string, AttributeBindInfo> attributeInfos;
	std::unordered_map<std::string, UniformLocation> activeUniformLocations;
	std::unordered_map<std::string, UniformInfo> activeUniformInfos;
	std::unordered_map<std::string, BlockInfoEx> blockInfoEx;
	std::unordered_map<std::string, cc::gfx::UniformSamplerTexture> samplers;
	// for ProgramState
	cocos2d::Map<void*, ProgramStateGFX*> _states;

	static cc::gfx::AttributeList builtinAttributes;
	static cc::gfx::UniformBlockList builtinUniforms;
	static cc::gfx::UniformSamplerTextureList builtinSamplerTextures;
	static cocos2d::Map<size_t, ShaderModuleGFX*> cachedModules;

	friend class ProgramState;
	friend class CommandBufferGFX;
};

class ProgramStateGFX : public Ref
{
public:
	ProgramStateGFX(
		ProgramGFX* program_,
		const cocos2d::Map<std::string, BufferGFX*>& buffers_ = {});
	virtual ~ProgramStateGFX();

	bool setUniformBuffer(const std::string& blockName, BufferGFX* buffer);
	BufferGFX* getUniformBuffer(const std::string& blockName);
	bool setUniform(const std::string& name, const void* data, std::size_t size);
	bool setUniform(const UniformLocation& loc, const void* data, std::size_t size);
	bool setAllBuffer(const void* data, std::size_t size);
	int32_t getAllBufferOffset(const UniformLocation& loc);
	bool syncTextures(ProgramStateGFX* dst);

	bool setTexture(const std::string& name, TextureBackend* texture, uint32_t index = 0);
	bool setTexture(const UniformLocation& loc, TextureBackend* texture, uint32_t index = 0);
	// bind to external DescriptorSet
	void bindUniformBuffers(cc::gfx::DescriptorSet* ds, uint32_t set);
	void bindUniformTextures(cc::gfx::DescriptorSet* ds, uint32_t set);
	// bind to internal DescriptorSet
	void bindUniformBuffers(uint32_t set = 0);
	void bindUniformTextures(uint32_t set = 0);

	cc::gfx::DescriptorSet* getDescriptorSet() const { return descriptorSet; }

protected:
	ProgramGFX* program = nullptr;
	cc::gfx::DescriptorSet* descriptorSet = nullptr;
	// { bname: buffer }
	cocos2d::Map<std::string, BufferGFX*> buffers;
	// { uname: { index: texture } }
	std::unordered_map<std::string, cocos2d::Map<uint32_t, TextureBackend*>> textures;
	std::vector<uint32_t> allBufferOffsets;
	std::map<uint32_t, std::vector<std::string>> blocksByBinding;

	friend class ProgramState;
};

CC_BACKEND_END
