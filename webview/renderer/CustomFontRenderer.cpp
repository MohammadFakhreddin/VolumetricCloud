#include "CustomFontRenderer.hpp"

#include "BedrockFile.hpp"
#include "LogicalDevice.hpp"

namespace MFA
{

    //------------------------------------------------------------------------------------------------------------------

    CustomFontRenderer::CustomFontRenderer(
        std::shared_ptr<Pipeline> pipeline,
        Alias const & fontData,
        float const fontHeight
    )
        : _pipeline(std::move(pipeline))
        , _fontHeight(fontHeight)
    {
        auto const buffer = fontData.As<uint8_t>();

        int const glyphHeight = (int)std::ceil(fontHeight);
        int const glyphWidth = (int)glyphHeight;
        int const bitmapSize = glyphWidth * glyphHeight * FontNumberOfChars;
        int const atlasWidth = std::ceil(sqrt(bitmapSize) + 1);
        int const atlasHeight = atlasWidth;

        // int result = stbtt_InitFont(&_font, buffer, 0);
        // MFA_ASSERT(result >= 0);

        auto bitmap = Memory::AllocSize(atlasWidth * atlasHeight * sizeof(uint8_t));
        _stbFontData.resize(FontNumberOfChars);
        // Bake font into the bitmap
        int const result = stbtt_BakeFontBitmap(
            buffer,                                     // Font data
            0,                                          // Offset to the start of the font
            fontHeight,                                 // Font pixel height
            bitmap->Ptr(),                              // Bitmap output
            atlasWidth, atlasHeight,                 // Bitmap dimensions
            FontFirstChar, FontNumberOfChars,           // ASCII range (inclusive)
            _stbFontData.data()                         // Output character data
        );
        MFA_ASSERT(result >= 0);
        CreateFontTextureBuffer(atlasWidth, atlasHeight, std::move(bitmap));
        _descriptorSet = _pipeline->CreateDescriptorSet(*_fontTexture);

        _atlasWidth = (float)atlasWidth;
        _atlasHeight = (float)atlasHeight;
    }

    //------------------------------------------------------------------------------------------------------------------

    std::unique_ptr<CustomFontRenderer::TextData> CustomFontRenderer::AllocateTextData(int maxCharCount)
    {
        auto const vertexBuffer = RB::CreateVertexBufferGroup(
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            maxCharCount * sizeof(Pipeline::Vertex),
            LogicalDevice::GetMaxFramePerFlight()
        );

        auto const vertexStageBuffer = RB::CreateStageBuffer(
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            vertexBuffer->bufferSize,
            vertexBuffer->buffers.size()
        );

        std::unique_ptr<TextData> textData = std::make_unique<TextData>(TextData{
            .letterRange = {},
            .vertexData = LocalBufferTracker(vertexBuffer, vertexStageBuffer),
            .maxLetterCount = maxCharCount
        });

        return textData;
    }

    //------------------------------------------------------------------------------------------------------------------

    bool CustomFontRenderer::AddText(
        TextData &inOutData,
        std::string_view const &text,
        float x, float y,
        TextParams const & params
    )
    {
        int letterRange = inOutData.letterRange.empty() == false ? inOutData.letterRange.back() : 0;
        int const letterRangeBegin = letterRange;

        auto * mapped = &reinterpret_cast<Pipeline::Vertex*>(inOutData.vertexData->Data())[letterRange * 4];
        auto * mappedBegin = mapped;

        float const scale = params.fontSizeInPixels / _fontHeight;

        bool success = true;

        // Generate a uv mapped quad per char in the new text
        for (auto const letter : text)
        {
            if (letterRange + 1 >= inOutData.maxLetterCount)
            {
                success = false;
                break;
            }

            int letterIdx = (int)(letter) - FontFirstChar;
            if (letterIdx >= 0 && letterIdx < FontNumberOfChars)
            {
                // I think x0, y0 is actually the uv coordinates and I should use xOffset and YOffset for other things
                auto const * charData = &_stbFontData[letterIdx];

                auto const pixelWidth = charData->x1 - charData->x0;
                auto const pixelHeight = charData->y1 - charData->y0;

                float const charW = (float)pixelWidth * scale;
                float const charH = (float)pixelHeight * scale;

                float const x0Offset = (float)charData->xoff * scale;
                float const y0Offset = (float)charData->yoff * scale;
                float const x1Offset = x0Offset + charW;
                float const y1Offset = y0Offset + charH;

                mapped->position.x = x + x0Offset;
                mapped->position.y = y + y0Offset;
                mapped->uv.x = (float)charData->x0 / _atlasWidth;
                mapped->uv.y = (float)charData->y0 / _atlasHeight;
                mapped->color = params.color;

                mapped++;

                mapped->position.x = x + x1Offset;
                mapped->position.y = y + y0Offset;
                mapped->uv.x = (float)charData->x1 / _atlasWidth;
                mapped->uv.y = (float)charData->y0 / _atlasHeight;
                mapped->color = params.color;

                mapped++;

                mapped->position.x = x + x0Offset;
                mapped->position.y = y + y1Offset;
                mapped->uv.x = (float)charData->x0 / _atlasWidth;
                mapped->uv.y = (float)charData->y1 / _atlasHeight;
                mapped->color = params.color;

                mapped++;

                mapped->position.x = x + x1Offset;
                mapped->position.y = y + y1Offset;
                mapped->uv.x = (float)charData->x1 / _atlasWidth;
                mapped->uv.y = (float)charData->y1 / _atlasHeight;
                mapped->color = params.color;

                mapped++;

                x += charData->xadvance * scale;

                letterRange++;
            }
        }

        mapped = mappedBegin;
        int const itrCount = std::min<int>(inOutData.maxLetterCount - letterRangeBegin, text.size() * 4);

        for (int i = 0; i < itrCount; ++i)
        {
            mapped[i].position.y += _fontHeight * scale * 0.75f;
        }

        auto const width = TextWidth(text, params.fontSizeInPixels);
        switch (params.hTextAlign)
        {
        case HorizontalTextAlign::Right:
            for (int i = 0; i < itrCount; ++i)
            {
                mappedBegin[i].position.x -= width;
            }
            break;
        case HorizontalTextAlign::Center:
            auto const halfWidth = width * 0.5f;
            for (int i = 0; i < itrCount; ++i)
            {
                mappedBegin[i].position.x -= halfWidth;
            }
            break;
        }

        inOutData.letterRange.emplace_back(letterRange);

        return success;
    }

    //------------------------------------------------------------------------------------------------------------------

    void CustomFontRenderer::ResetText(TextData &inOutData)
    {
        inOutData.letterRange.clear();
    }

    //------------------------------------------------------------------------------------------------------------------

    void CustomFontRenderer::Draw(
        RT::CommandRecordState &recordState,
        Pipeline::PushConstants const &pushConstants,
        TextData const &data
    ) const
    {
        _pipeline->BindPipeline(recordState);

        _pipeline->SetPushConstant(recordState, pushConstants);

        RB::AutoBindDescriptorSet(
            recordState,
            RB::UpdateFrequency::PerPipeline,
            _descriptorSet.descriptorSets[0]
        );

        RB::BindVertexBuffer(recordState, *data.vertexData->LocalBuffer().buffers[recordState.frameIndex]);

        int previousLetterRange = 0;
        for (auto const& letterRange : data.letterRange)
        {
            int const currentLetterRange = letterRange * 4;
            vkCmdDraw(recordState.commandBuffer, currentLetterRange - previousLetterRange, 1, previousLetterRange, 0);
            previousLetterRange = currentLetterRange;
        }
    }

    //------------------------------------------------------------------------------------------------------------------

    float CustomFontRenderer::TextWidth(std::string_view const & text, float const fontSizeInPixels) const
    {
        float const scale = fontSizeInPixels / _fontHeight;

        float textWidth = 0;
        for (auto const letter : text)
        {
            int const idx = static_cast<int>(letter) - FontFirstChar;
            if (idx >= 0 && idx < FontNumberOfChars)
            {
                auto const * charData = &_stbFontData[static_cast<uint32_t>(letter) - FontFirstChar];
                textWidth += charData->xadvance * scale;
            }
        }

        return textWidth;
    }

    //------------------------------------------------------------------------------------------------------------------

    float CustomFontRenderer::TextHeight(float const fontSizeInPixels) const
    {
        return fontSizeInPixels * HeightModifier;
    }

    //------------------------------------------------------------------------------------------------------------------
    // TODO: Single time command buffers might not be the best idea but I am confused at the moment.
    // TODO: Maybe instead I just have to make things when first called inside the main thread?
    void CustomFontRenderer::CreateFontTextureBuffer(
        uint32_t const width,
        uint32_t const height,
        std::unique_ptr<Blob> bytes
    )
    {
        AS::Texture cpuTexture{
            Asset::Texture::Format::UNCOMPRESSED_UNORM_R8_LINEAR,
            1, 1, 1
        };

        cpuTexture.SetMipmapDimension(
            0,
            Asset::Texture::Dimensions{.width = width, .height = height, .depth = 1}
        );

        cpuTexture.SetMipmapSize(0, bytes->Len());

        cpuTexture.SetMipmapData(
            0,
            std::move(bytes)
        );

        auto commandBuffer = RB::BeginSingleTimeCommand(LogicalDevice::GetVkDevice(), *LogicalDevice::GetGraphicCommandPool());

        std::vector<uint8_t> mipLevels{0};
        auto [fontTexture, stageBuffer] = RB::CreateTexture(
            cpuTexture,
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            commandBuffer,
            mipLevels.size(),
            mipLevels.data()
        );
        MFA_ASSERT(fontTexture != nullptr);
        _fontTexture = std::move(fontTexture);

        RB::EndAndSubmitSingleTimeCommand(
            LogicalDevice::GetVkDevice(),
            *LogicalDevice::GetGraphicCommandPool(),
            LogicalDevice::GetGraphicQueue(),
            commandBuffer
        );
    }

    //------------------------------------------------------------------------------------------------------------------

} // namespace MFA
