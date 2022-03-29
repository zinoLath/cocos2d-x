#pragma once
#include "renderer/backend/Texture.h"
#include "base/CCEventListenerCustom.h"
#include "gfx/backend/GFXDeviceManager.h"

CC_BACKEND_BEGIN

class Texture2DGFX : public Texture2DBackend
{
public:

	Texture2DGFX(const TextureDescriptor& descriptor);
	~Texture2DGFX();

	void updateData(uint8_t* data, std::size_t width, std::size_t height, std::size_t level) override;

	void updateCompressedData(uint8_t* data, std::size_t width, std::size_t height, std::size_t dataLen, std::size_t level) override;

	void updateSubData(std::size_t xoffset, std::size_t yoffset, std::size_t width, std::size_t height, std::size_t level, uint8_t* data) override;

	void updateCompressedSubData(std::size_t xoffset, std::size_t yoffset, std::size_t width, std::size_t height, std::size_t dataLen, std::size_t level, uint8_t* data) override;

	void updateSamplerDescriptor(const SamplerDescriptor& sampler)  override;

	void update(const cc::gfx::BufferDataList& buffers, const cc::gfx::BufferTextureCopyList& regions);

	void getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage, std::function<void(const unsigned char*, std::size_t, std::size_t)> callback) override;

	void generateMipmaps() override;

	void updateTextureDescriptor(const TextureDescriptor& descriptor) override;

	cc::gfx::Texture* getHandler() const { return _texture; }
	cc::gfx::Sampler* getSampler() const { return _sampler; }

private:
	bool isParameterValid();
	void initWithZeros();
	void resetTexture();

	cc::gfx::TextureInfo _info;
	cc::gfx::SamplerInfo _sinfo;
	cc::gfx::Texture* _texture = nullptr;
	cc::gfx::Sampler* _sampler = nullptr;
	EventListener* _backToForegroundListener = nullptr;
};

class TextureCubeGFX : public TextureCubemapBackend
{
public:

	TextureCubeGFX(const TextureDescriptor& descriptor);
	~TextureCubeGFX();

	void updateSamplerDescriptor(const SamplerDescriptor& sampler) override;

	void updateFaceData(TextureCubeFace side, void* data) override;

	void getBytes(std::size_t x, std::size_t y, std::size_t width, std::size_t height, bool flipImage, std::function<void(const unsigned char*, std::size_t, std::size_t)> callback) override;

	void generateMipmaps() override;

	void updateTextureDescriptor(const TextureDescriptor& descriptor) override;

	cc::gfx::Texture* getHandler() const { return _texture; }
	cc::gfx::Sampler* getSampler() const { return _sampler; }

private:
	cc::gfx::TextureInfo _info;
	cc::gfx::SamplerInfo _sinfo;
	cc::gfx::Texture* _texture = nullptr;
	cc::gfx::Sampler* _sampler = nullptr;
	EventListener* _backToForegroundListener = nullptr;
};

CC_BACKEND_END
