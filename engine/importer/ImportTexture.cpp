#include "ImportTexture.hpp"

#include <fstream>
#include <iostream>

#include "AssetTexture.hpp"
#include "BedrockAssert.hpp"
#include "BedrockDeffer.hpp"
#include "BedrockFile.hpp"
#include "BedrockMemory.hpp"
#include "BedrockPath.hpp"

#include "stb_image.h"
#include "stb_image_resize.h"
// #include "ktx.h"
// #include "vulkan/vulkan_core.h"
// #include "ktxvulkan.h"

namespace MFA::Importer
{

    using Format = AS::Texture::Format;

    struct Data
    {
        int32_t width = 0;
        int32_t height = 0;
        int32_t stbi_components = 0;
        std::shared_ptr<Blob> stbi_pixels;
        Format format = Format::INVALID;
        std::shared_ptr<Blob> pixels;
        uint32_t components = 0;
        [[nodiscard]]
        bool valid() const
        {
            return stbi_pixels != nullptr &&
                stbi_pixels->IsValid() == true &&
                width > 0 &&
                height > 0 &&
                stbi_components > 0;
        }
    };

    enum class LoadResult
    {
        Invalid,
        Success,
        FileNotExists
        // Format not supported
    };

    static LoadResult LoadUncompressed(Data& outImageData, std::string const& path, bool prefer_srgb)
    {
        LoadResult ret = LoadResult::Invalid;

        auto const rawFile = File::Read(path);

        if (rawFile == nullptr)
        {
            return ret;
        }

        auto* readData = stbi_load_from_memory(
            rawFile->Ptr(),
            static_cast<int>(rawFile->Len()),
            &outImageData.width,
            &outImageData.height,
            &outImageData.stbi_components,
            0
        );

        MFA_DEFFER([readData](){stbi_image_free(readData);});
        if (readData != nullptr)
        {
            MFA_ASSERT(outImageData.width > 0);
            MFA_ASSERT(outImageData.height > 0);
            MFA_ASSERT(outImageData.stbi_components > 0);

            auto blobSize = static_cast<size_t>(outImageData.width) *
                outImageData.height *
                outImageData.stbi_components *
                sizeof(uint8_t);

            outImageData.stbi_pixels = Memory::Alloc(readData, blobSize);

            outImageData.components = outImageData.stbi_components;
            if (prefer_srgb)
            {
                switch (outImageData.stbi_components)
                {
                case 1:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8_SRGB;
                    break;
                case 2:
                case 3:
                case 4:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8B8A8_SRGB;
                    outImageData.components = 4;
                    break;
                default: MFA_NOT_IMPLEMENTED_YET("Mohammad Fakhreddin");
                }
            }
            else
            {
                switch (outImageData.stbi_components)
                {
                case 1:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8_LINEAR;
                    break;
                case 2:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8_LINEAR;
                    break;
                case 3:
                case 4:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR;
                    outImageData.components = 4;
                    break;
                default: MFA_LOG_WARN("Unhandled component count: %d", outImageData.stbi_components);
                }
            }
            MFA_ASSERT(outImageData.components >= static_cast<uint32_t>(outImageData.stbi_components));
            if (static_cast<int>(outImageData.components) == outImageData.stbi_components)
            {
                outImageData.pixels = outImageData.stbi_pixels;
            }
            else
            {
                auto const size = static_cast<size_t>(outImageData.width) *
                    outImageData.height *
                    outImageData.components *
                    sizeof(uint8_t);

                outImageData.pixels = Memory::AllocSize(size);

                auto* pixels_array = outImageData.pixels->As<uint8_t>();
                auto const* stbi_pixels_array = outImageData.stbi_pixels->As<uint8_t>();
                // This can be parallized using openMP
                for (int pixel_index = 0; pixel_index < outImageData.width * outImageData.height; pixel_index++)
                {
                    for (uint32_t component_index = 0; component_index < outImageData.components; component_index++)
                    {
                        pixels_array[pixel_index * outImageData.components + component_index] = static_cast<int64_t>(component_index) < outImageData.stbi_components
                            ? stbi_pixels_array[pixel_index * outImageData.stbi_components + component_index]
                            : 255u;
                    }
                }
            }
            ret = LoadResult::Success;
        }
        else
        {
            ret = LoadResult::FileNotExists;
        }
        return ret;
    }

    //-------------------------------------------------------------------------------------------------

    struct ResizeInputParams
    {
        BaseBlob inputImagePixels{};
        int32_t inputImageWidth{};
        int32_t inputImageHeight{};
        uint32_t componentsCount{};
        std::shared_ptr<Blob> outputImagePixels{};
        int32_t outputWidth{};
        int32_t outputHeight{};
        bool useSRGB{};
    };

    static bool ResizeUncompressed(ResizeInputParams const& params)
    {
        MFA_ASSERT(params.inputImageWidth > 0);
        MFA_ASSERT(params.inputImageHeight > 0);
        MFA_ASSERT(params.inputImagePixels.Ptr() != nullptr);
        MFA_ASSERT(params.inputImagePixels.Len() > 0);

        MFA_ASSERT(params.componentsCount > 0);

        MFA_ASSERT(params.outputWidth > 0);
        MFA_ASSERT(params.outputHeight > 0);
        MFA_ASSERT(params.outputImagePixels->Ptr() != nullptr);
        MFA_ASSERT(params.outputImagePixels->Len() > 0);

        auto const resizeResult = params.useSRGB ? stbir_resize_uint8_srgb(
            params.inputImagePixels.Ptr(),
            params.inputImageWidth,
            params.inputImageHeight,
            0,
            params.outputImagePixels->Ptr(),
            params.outputWidth,
            params.outputHeight,
            0,
            params.componentsCount,
            params.componentsCount > 3 ? 3 : STBIR_ALPHA_CHANNEL_NONE,
            0
        ) : stbir_resize_uint8(
            params.inputImagePixels.Ptr(),
            params.inputImageWidth,
            params.inputImageHeight,
            0,
            params.outputImagePixels->Ptr(),
            params.outputWidth,
            params.outputHeight,
            0,
            params.componentsCount
        );
        return resizeResult > 0;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<AS::Texture> UncompressedImage(std::string const& path)
    {
        std::shared_ptr<AS::Texture> texture{};

        Data imageData{};
        auto const loadImageResult = LoadUncompressed(
            imageData,
            path,
            false
        );

        if (loadImageResult == LoadResult::Success)
        {
            MFA_ASSERT(imageData.valid());
            uint32_t const imageWidth = imageData.width;
            uint32_t const imageHeight = imageData.height;
            auto const pixels = imageData.pixels;
            // auto const components = imageData.components;
            auto const format = imageData.format;

            texture = std::make_shared<AS::Texture>(
                path,
                format,
                1,
                1,
                1
            );

            texture->SetMipmapDimension(0, AS::Texture::Dimensions {
                .width = imageWidth,
                .height = imageHeight,
                .depth = 1
            });
            texture->SetMipmapOffset(0, 0);
            texture->SetMipmapSize(0, pixels->Len());
            texture->SetMipmapData(0, pixels);
        }

        return texture;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<AS::Texture> ErrorTexture()
    {
        auto data = Memory::AllocSize(4);
        auto *pixels = data->As<uint8_t>();
        pixels[0] = 1;
        pixels[1] = 1;
        pixels[2] = 1;
        pixels[3] = 1;

        auto const texture = std::make_shared<AS::Texture>("", Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR, 1, 1, 1);

        texture->SetMipmapDimension(0, AS::Texture::Dimensions{.width = 1, .height = 1, .depth = 1});
        texture->SetMipmapOffset(0, 0);
        texture->SetMipmapSize(0, data->Len());
        texture->SetMipmapData(0, std::move(data));

        return texture;
    }

    //-------------------------------------------------------------------------------------------------

    // static Format MapVkFormat(ktx_uint32_t vkFormat) {
    //     switch (vkFormat) {
    //         case VK_FORMAT_R8G8B8A8_SRGB: //return Format::UNCOMPRESSED_UNORM_R8G8B8A8_SRGB;              // VK_FORMAT_R8G8B8A8_SRGB
    //         case VK_FORMAT_R8G8B8A8_UNORM: return Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR;           // VK_FORMAT_R8G8B8A8_UNORM
    //         case VK_FORMAT_R16G16B16A16_UNORM: return Format::UNCOMPRESSED_UNORM_R16G16B16A16_LINEAR;   // VK_FORMAT_R16G16B16A16_UNORM
    //         case VK_FORMAT_BC7_UNORM_BLOCK: return Format::BC7_UNorm_Linear_RGBA;                       // VK_FORMAT_BC7_UNORM_BLOCK
    //         case VK_FORMAT_BC7_SRGB_BLOCK: return Format::BC7_UNorm_sRGB_RGBA;                          // VK_FORMAT_BC7_SRGB_BLOCK
    //         case VK_FORMAT_BC6H_UFLOAT_BLOCK: return Format::BC6H_UFloat_Linear_RGB;                    // VK_FORMAT_BC6H_UFLOAT_BLOCK
    //         case VK_FORMAT_BC6H_SFLOAT_BLOCK: return Format::BC6H_SFloat_Linear_RGB;                    // VK_FORMAT_BC6H_SFLOAT_BLOCK
    //         case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return Format::ASTC_4x4_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return Format::ASTC_4x4_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return Format::ASTC_5x4_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return Format::ASTC_5x4_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return Format::ASTC_5x5_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return Format::ASTC_5x5_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return Format::ASTC_6x5_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return Format::ASTC_6x5_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return Format::ASTC_6x6_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return Format::ASTC_6x6_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return Format::ASTC_8x5_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return Format::ASTC_8x5_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return Format::ASTC_8x6_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return Format::ASTC_8x6_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return Format::ASTC_8x8_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return Format::ASTC_8x8_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return Format::ASTC_10x5_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return Format::ASTC_10x5_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return Format::ASTC_10x6_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return Format::ASTC_10x6_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return Format::ASTC_10x8_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return Format::ASTC_10x8_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return Format::ASTC_10x10_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return Format::ASTC_10x10_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return Format::ASTC_12x10_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return Format::ASTC_12x10_SRGB_BLOCK;
    //         case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return Format::ASTC_12x12_UNORM_BLOCK;
    //         case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return Format::ASTC_12x12_SRGB_BLOCK;
    //         default: MFA_ASSERT(false); return Format::INVALID;
    //     }
    // }
    //
    // KTX_error_code WorstMipmapLoadHandler(
    //     int mipLevel,
    //     int face,
    //     int width,
    //     int height,
    //     int depth,
    //     ktx_uint64_t faceLodSize,
    //     void* pixels,
    //     void* userData
    // )
    // {
    //     // TODO: Multiple face is not supported yet.
    //     auto * texture = (AS::Texture *)userData;
    //     texture->SetMipmapDimension(mipLevel, AS::Texture::Dimensions{
    //         .width = static_cast<uint32_t>(width),
    //         .height = static_cast<uint32_t>(height),
    //         .depth = (uint16_t)depth
    //     });
    //     texture->SetMipmapSize(mipLevel, faceLodSize);
    //     auto blob = Memory::AllocSize(faceLodSize);
    //     std::memcpy(blob->Ptr(), pixels, blob->Len());
    //     texture->SetMipmapData(mipLevel, std::move(blob));
    //
    //     return KTX_SUCCESS;
    // }
    //
    // std::shared_ptr<Asset::Texture> LoadKtxMetadata(char const *path)
    // {
    //     MFA_ASSERT(std::filesystem::exists(path));
    //
    //     ktxTexture* ktx;
    //
    //     FILE* fp = fopen(path, "rb");
    //     MFA_ASSERT(fp != nullptr);
    //     auto result = ktxTexture_CreateFromStdioStream(fp, KTX_TEXTURE_CREATE_NO_FLAGS, &ktx);
    //     // auto result = ktxTexture_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx);
    //
    //     MFA_DEFFER([&ktx]()->void
    //     {
    //         ktxTexture_Destroy(ktx);
    //     });
    //
    //     if (result != KTX_SUCCESS)
    //     {
    //         MFA_LOG_WARN(
    //             "Failed to load ktx file with name: %s\n Reason: %s\n"
    //             , path
    //             , ktxErrorString(result)
    //         );
    //         return nullptr;
    //     }
    //
    //     if (ktx->classId != ktxTexture2_c)
    //     {
    //         MFA_LOG_WARN("Only level 2 ktx is supported");
    //         return nullptr;
    //     }
    //
    //     ktxTexture2* ktx2;
    //     if (ktx->classId == ktxTexture2_c) {
    //         ktx2 = (ktxTexture2*)ktx;
    //         // iF TEXTURE NEEDS TRANSCODING THE WE HAVE TO LOAD THE WHOLE THING
    //         if (ktxTexture2_NeedsTranscoding(ktx2)) {
    //             MFA_ASSERT(false);
    //             KTX_error_code transcodeResult = ktxTexture2_TranscodeBasis(ktx2, KTX_TTF_RGBA32, 0);
    //             if (transcodeResult != KTX_SUCCESS) {
    //                 MFA_LOG_WARN("Transcoding failed: %s\n", ktxErrorString(transcodeResult));
    //                 return nullptr;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         MFA_LOG_WARN("Failed to load %s because only ktx2 is supported", path);
    //         return nullptr;
    //     }
    //
    //     auto vkFormat = ktx2->vkFormat;
    //     auto const format = MapVkFormat(vkFormat);
    //
    //     auto const texture = std::make_shared<AS::Texture>(path, format, ktx->numFaces, ktx->numLayers, ktx->numLevels);
    //
    //     ktxTexture_IterateLoadLevelFaces(ktx, WorstMipmapLoadHandler, texture.get());
    //
    //     return texture;
    // }

    //-------------------------------------------------------------------------------------------------

} // namespace MFA::Importer
