#pragma once

#include "BedrockMemory.hpp"

#include <memory>
#include <vector>
#include <string>

namespace MFA::Asset
{

    class Texture final
    {
    public:
        // TODO: We could have just stored the vkFormat
        enum class Format : uint8_t
        {
            INVALID = 0,
            UNCOMPRESSED_UNORM_R8_LINEAR = 1,
            UNCOMPRESSED_UNORM_R8G8_LINEAR = 2,
            UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR = 3,
            UNCOMPRESSED_UNORM_R8_SRGB = 4,
            UNCOMPRESSED_UNORM_R8G8B8A8_SRGB = 5,
            UNCOMPRESSED_UNORM_R16G16B16A16_LINEAR = 6,

            BC7_UNorm_Linear_RGB,
            BC7_UNorm_Linear_RGBA,
            BC7_UNorm_sRGB_RGB,
            BC7_UNorm_sRGB_RGBA,

            BC6H_UFloat_Linear_RGB,
            BC6H_SFloat_Linear_RGB,

            BC5_UNorm_Linear_RG,
            BC5_SNorm_Linear_RG,

            BC4_UNorm_Linear_R,
            BC4_SNorm_Linear_R,

            ASTC_4x4_UNORM_BLOCK,
            ASTC_4x4_SRGB_BLOCK,
            ASTC_5x4_UNORM_BLOCK,
            ASTC_5x4_SRGB_BLOCK,
            ASTC_5x5_UNORM_BLOCK,
            ASTC_5x5_SRGB_BLOCK,
            ASTC_6x5_UNORM_BLOCK,
            ASTC_6x5_SRGB_BLOCK,
            ASTC_6x6_UNORM_BLOCK,
            ASTC_6x6_SRGB_BLOCK,
            ASTC_8x5_UNORM_BLOCK,
            ASTC_8x5_SRGB_BLOCK,
            ASTC_8x6_UNORM_BLOCK,
            ASTC_8x6_SRGB_BLOCK,
            ASTC_8x8_UNORM_BLOCK,
            ASTC_8x8_SRGB_BLOCK,
            ASTC_10x5_UNORM_BLOCK,
            ASTC_10x5_SRGB_BLOCK,
            ASTC_10x6_UNORM_BLOCK,
            ASTC_10x6_SRGB_BLOCK,
            ASTC_10x8_UNORM_BLOCK,
            ASTC_10x8_SRGB_BLOCK,
            ASTC_10x10_UNORM_BLOCK,
            ASTC_10x10_SRGB_BLOCK,
            ASTC_12x10_UNORM_BLOCK,
            ASTC_12x10_SRGB_BLOCK,
            ASTC_12x12_UNORM_BLOCK,
            ASTC_12x12_SRGB_BLOCK,

            Count
        };
        
        struct Dimensions
        {
            uint32_t width = 0;
            uint32_t height = 0;
            uint16_t depth = 0;
        };

        struct Mipmap
        {
            Dimensions dimension{};
            size_t offset = 0;
            size_t size = 0;

            std::shared_ptr<Blob> data{};
        };

    private:
        struct InternalFormatTableType
        {
            Format texture_format;
            uint8_t compression;                            // 0: uncompressed, 1: basis?, ...
            uint8_t component_count;                        // 1..4
            uint8_t component_format;                       // 0: UNorm, 1: SNorm, 2: UInt, 3: SInt, 4: UFloat, 5: SFloat
            uint8_t color_space;                            // 0: Linear, 1: sRGB
            uint8_t bits_r, bits_g, bits_b, bits_a;         // each 0..32
            uint8_t bits_total;                             // 1..128
        };
    public:
        static constexpr InternalFormatTableType FormatTable[] = {
            {Format::INVALID                                 , 0, 0, 0, 0, 0, 0, 0, 0,  0},

            {Format::UNCOMPRESSED_UNORM_R8_LINEAR            , 0, 1, 0, 0, 8, 0, 0, 0,  8},
            {Format::UNCOMPRESSED_UNORM_R8G8_LINEAR          , 0, 2, 0, 0, 8, 8, 0, 0, 16},
            {Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR      , 0, 4, 0, 0, 8, 8, 8, 8, 32},
            {Format::UNCOMPRESSED_UNORM_R8_SRGB              , 0, 1, 0, 1, 8, 0, 0, 0,  8},
            {Format::UNCOMPRESSED_UNORM_R8G8B8A8_SRGB        , 0, 4, 0, 1, 8, 8, 8, 8, 32},

            {Format::BC7_UNorm_Linear_RGB                    , 7, 3, 0, 0, 8, 8, 8, 0,  8},
            {Format::BC7_UNorm_Linear_RGBA                   , 7, 4, 0, 0, 8, 8, 8, 8,  8},
            {Format::BC7_UNorm_sRGB_RGB                      , 7, 3, 0, 1, 8, 8, 8, 0,  8},
            {Format::BC7_UNorm_sRGB_RGBA                     , 7, 4, 0, 1, 8, 8, 8, 8,  8},

            {Format::BC6H_UFloat_Linear_RGB                  , 6, 3, 4, 0, 16, 16, 16, 0, 8},
            {Format::BC6H_SFloat_Linear_RGB                  , 6, 3, 5, 0, 16, 16, 16, 0, 8},

            {Format::BC5_UNorm_Linear_RG                     , 5, 2, 0, 0, 8, 8, 0, 0, 8},
            {Format::BC5_SNorm_Linear_RG                     , 5, 2, 0, 1, 8, 8, 0, 0, 8},

            {Format::BC4_UNorm_Linear_R                      , 5, 2, 0, 0, 8, 8, 0, 0, 16},
            {Format::BC4_SNorm_Linear_R                      , 5, 2, 0, 1, 8, 8, 0, 0, 16},

        };
        
    public:

        explicit Texture(
            Format format,
            uint16_t slices,
            uint16_t depth,
            uint8_t mipCount
        );

        explicit Texture(
            std::string address,
            Format format,
            uint16_t slices,
            uint16_t depth,
            uint8_t mipCount
        );

        ~Texture();

        Texture(Texture const&) noexcept = delete;
        Texture(Texture&&) noexcept = delete;
        Texture& operator= (Texture const& rhs) noexcept = delete;
        Texture& operator= (Texture&& rhs) noexcept = delete;

        /*
         * This function result is only correct for uncompressed data
         */
        [[nodiscard]]
        static size_t MipSizeBytes(
            Format format,
            uint16_t slices,
            Dimensions const& mipLevelDimension
        );

        // TODO Consider moving this function to util_image
        /*
         * This function result is only correct for uncompressed data
         */
         // NOTE: 0 is the *smallest* mipmap level, and "mip_count - 1" is the *largest*.
        [[nodiscard]]
        static Dimensions MipDimensions(
            uint8_t mipLevel,
            uint8_t mipCount,
            Dimensions originalImageDims
        );

        void SetMipmapDimension(
            uint8_t mipLevel,
            Dimensions const& dimension
        );

        void SetMipmapOffset(
            uint8_t mipLevel,
            size_t offset
        );

        void SetMipmapSize(
            uint8_t mipLevel,
            size_t size
        );

        void SetMipmapData(
            uint8_t mipLevel,
            std::shared_ptr<Blob> const& data
        );

        void ClearMipmapBuffer(uint8_t mipLevel);

        void ClearMipmapBuffer();

        [[nodiscard]]
        std::string GetAddress() const;

        [[nodiscard]]
        Format GetFormat() const noexcept;

        [[nodiscard]]
        uint16_t GetSlices() const noexcept;

        [[nodiscard]]
        uint8_t GetMipCount() const noexcept;

        [[nodiscard]]
        uint16_t GetDepth() const noexcept;

        [[nodiscard]]
        size_t GetMipmapSize(uint8_t mipLevel) const noexcept;

        [[nodiscard]]
        size_t GetMipmapOffset(uint8_t mipLevel) const noexcept;

        [[nodiscard]]
        Dimensions const & GetMipmapDimension(uint8_t mipLevel) const noexcept;

        [[nodiscard]]
        std::shared_ptr<Blob> const & GetMipmapBuffer(uint8_t mipLevel) const noexcept;

    private:

        std::string const mAddress;
        Format const mFormat;
        uint16_t const mSlices;
        uint8_t const mMipCount;
        uint16_t const mDepth;

        std::vector<Mipmap> mMipmaps{};
    };

}

namespace MFA
{
    namespace AS = Asset;
}
