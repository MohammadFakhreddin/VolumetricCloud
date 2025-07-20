#include "AssetTexture.hpp"

#include "BedrockAssert.hpp"

namespace MFA::Asset
{

	//-------------------------------------------------------------------------------------------------

    Texture::Texture(Format format, uint16_t slices, uint16_t depth, uint8_t mipCount)
        : Texture("", format, slices, depth, mipCount)
    {}

    Texture::Texture(
	    std::string address,
	    Format const format,
        uint16_t const slices,
        uint16_t const depth,
        uint8_t const mipCount
	)
	    : mAddress(std::move(address))
        , mFormat(format)
        , mSlices(slices)
        , mMipCount(mipCount)
        , mDepth(depth)
	{
	    MFA_ASSERT(format != Format::INVALID);
		MFA_ASSERT(slices > 0);
		MFA_ASSERT(depth > 0);
	MFA_ASSERT(mipCount > 0);

	    mMipmaps.resize(mMipCount);
	}

	//-------------------------------------------------------------------------------------------------

	Texture::~Texture()
	{
	    mMipmaps.clear();
	}

	//-------------------------------------------------------------------------------------------------

	size_t Texture::MipSizeBytes(Format format, uint16_t slices, Dimensions const& mipLevelDimension)
	{
		auto const& d = mipLevelDimension;
		size_t const p = FormatTable[static_cast<unsigned>(format)].bits_total / 8;
		return p * slices * d.width * d.height * d.depth;
	}

	//-------------------------------------------------------------------------------------------------

	Texture::Dimensions Texture::MipDimensions(
		uint8_t const mipLevel, 
		uint8_t const mipCount,
		Dimensions const originalImageDims
	)
	{
		Dimensions ret = {};
		if (mipLevel < mipCount)
		{
			uint32_t const pow = mipLevel;
			uint32_t const add = (1 << pow) - 1;
			ret.width = (originalImageDims.width + add) >> pow;
			ret.height = (originalImageDims.height + add) >> pow;
			ret.depth = static_cast<uint16_t>((originalImageDims.depth + add) >> pow);
		}
		return ret;
	}

	//-------------------------------------------------------------------------------------------------

    void Texture::SetMipmapDimension(uint8_t const mipLevel, Dimensions const &dimension)
    {
        MFA_ASSERT(mipLevel < mMipCount);
        mMipmaps[mipLevel].dimension = dimension;
    }

    void Texture::SetMipmapOffset(uint8_t const mipLevel, size_t const offset)
	{
	    MFA_ASSERT(mipLevel < mMipCount);
	    mMipmaps[mipLevel].offset = offset;
	}

    void Texture::SetMipmapSize(uint8_t const mipLevel, size_t const size)
	{
        MFA_ASSERT(mipLevel < mMipCount);
	    mMipmaps[mipLevel].size = size;
	}

	void Texture::SetMipmapData(uint8_t const mipLevel, std::shared_ptr<Blob> const &data)
    {
        MFA_ASSERT(mipLevel < mMipCount);
        mMipmaps[mipLevel].data = data;
        mMipmaps[mipLevel].size = data->Len();
    }

    //-------------------------------------------------------------------------------------------------

    void Texture::ClearMipmapBuffer(uint8_t const mipLevel)
	{
	    mMipmaps[mipLevel].data = nullptr;
	}

    void Texture::ClearMipmapBuffer()
    {
        for (auto &mipmap : mMipmaps)
        {
            mipmap.data = nullptr;
        }
    }

    //-------------------------------------------------------------------------------------------------

    std::string Texture::GetAddress() const
	{
	    return mAddress;
	}

    Texture::Format Texture::GetFormat() const noexcept
	{
		return mFormat;
	}

	uint16_t Texture::GetSlices() const noexcept
	{
		return mSlices;
	}

	uint8_t Texture::GetMipCount() const noexcept
	{
		return mMipCount;
	}

	//-------------------------------------------------------------------------------------------------

    size_t Texture::GetMipmapSize(uint8_t const mipLevel) const noexcept
	{
	    MFA_ASSERT(mipLevel < mMipCount);
	    return mMipmaps[mipLevel].size;
	}

    size_t Texture::GetMipmapOffset(uint8_t const mipLevel) const noexcept
	{
	    MFA_ASSERT(mipLevel < mMipCount);
	    return mMipmaps[mipLevel].offset;
	}

	Texture::Dimensions const& Texture::GetMipmapDimension(uint8_t const mipLevel) const noexcept
	{
	    MFA_ASSERT(mipLevel < mMipCount);
		return mMipmaps[mipLevel].dimension;
	}

	std::shared_ptr<Blob> const & Texture::GetMipmapBuffer(uint8_t const mipLevel) const noexcept
	{
	    MFA_ASSERT(mipLevel < mMipCount);
		return mMipmaps[mipLevel].data;
	}

	//-------------------------------------------------------------------------------------------------

	uint16_t Texture::GetDepth() const noexcept { return mDepth; }

    //-------------------------------------------------------------------------------------------------

}
