#pragma once
#include "renderer/backend/Device.h"

CC_BACKEND_BEGIN

/**
 * Use to create resoureces.
 */
class DeviceGFX : public Device
{
	void* windowHandle = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	bool vsync = true;
public:
	static void setSwapchainInfo(void* windowHandle, bool vsync, uint32_t width, uint32_t height);

	static bool isAvailable();

	DeviceGFX();
	~DeviceGFX();

	/**
	 * New a CommandBuffer object, not auto released.
	 * @return A CommandBuffer object.
	 */
	CommandBuffer* newCommandBuffer() override;

	/**
	 * New a Buffer object, not auto released.
	 * @param size Specifies the size in bytes of the buffer object's new data store.
	 * @param type Specifies the target buffer object. The symbolic constant must be BufferType::VERTEX or BufferType::INDEX.
	 * @param usage Specifies the expected usage pattern of the data store. The symbolic constant must be BufferUsage::STATIC, BufferUsage::DYNAMIC.
	 * @return A Buffer object.
	 */
	Buffer* newBuffer(std::size_t size, BufferType type, BufferUsage usage) override;
	Buffer* newBuffer(uint32_t size, uint32_t stride, BufferType type, BufferUsage usage);

	/**
	 * New a TextureBackend object, not auto released.
	 * @param descriptor Specifies texture description.
	 * @return A TextureBackend object.
	 */
	TextureBackend* newTexture(const TextureDescriptor& descriptor) override;

	/**
	 * Create an auto released DepthStencilState object.
	 * @param descriptor Specifies depth and stencil description.
	 * @return An auto release DepthStencilState object.
	 */
	DepthStencilState* createDepthStencilState(const DepthStencilDescriptor& descriptor) override;

	/**
	 * New a RenderPipeline object, not auto released.
	 * @return A RenderPipeline object.
	 */
	RenderPipeline* newRenderPipeline() override;

	/**
	 * Design for metal.
	 */
	void setFrameBufferOnly(bool frameBufferOnly) override {}

	/**
	 * New a Program, not auto released.
	 * @param vertexShader Specifes this is a vertex shader source.
	 * @param fragmentShader Specifes this is a fragment shader source.
	 * @return A Program instance.
	 */
	Program* newProgram(const std::string& vertexShader, const std::string& fragmentShader) override;

	void* getWindowHandle() const { return windowHandle; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	bool getVsync() const { return vsync; }

protected:
	/**
	 * New a shaderModule, not auto released.
	 * @param stage Specifies whether is vertex shader or fragment shader.
	 * @param source Specifies shader source.
	 * @return A ShaderModule object.
	 */
	ShaderModule* newShaderModule(ShaderStage stage, const std::string& source) override;

	static void initBuiltinShaderInfo();
};

CC_BACKEND_END
