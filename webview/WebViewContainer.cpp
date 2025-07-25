#include "WebViewContainer.hpp"

#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"
#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "ScopeProfiler.hpp"
#include "renderer/BorderRenderer.hpp"
#include "renderer/CustomFontRenderer.hpp"
#include "renderer/ImageRenderer.hpp"
#include "renderer/SolidFillPipeline.hpp"
#include "renderer/SolidFillRenderer.hpp"

#include "litehtml/render_item.h"
#include "stb_image.h"

#include <ranges>

using namespace MFA;

namespace std
{
    template <>
    struct hash<litehtml::border_radiuses>
    {
        size_t operator()(const litehtml::border_radiuses &radii) const
        {
            size_t h1 = std::hash<int>()(radii.top_left_x);
            size_t h2 = std::hash<int>()(radii.top_left_y);
            size_t h3 = std::hash<int>()(radii.top_right_x);
            size_t h4 = std::hash<int>()(radii.top_right_y);
            size_t h5 = std::hash<int>()(radii.bottom_right_x);
            size_t h6 = std::hash<int>()(radii.bottom_right_y);
            size_t h7 = std::hash<int>()(radii.bottom_left_x);
            size_t h8 = std::hash<int>()(radii.bottom_left_y);

            // Combine the hash values
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6) ^ (h8 << 7);
        }
    };

    template <>
    struct hash<litehtml::position>
    {
        size_t operator()(const litehtml::position &box) const
        {
            size_t h1 = std::hash<int>()(box.x);
            size_t h2 = std::hash<int>()(box.y);
            size_t h3 = std::hash<int>()(box.width);
            size_t h4 = std::hash<int>()(box.height);

            // Combine the hash values
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    template <>
    struct hash<litehtml::background_layer>
    {
        size_t operator()(const litehtml::background_layer &layer) const
        {
            size_t h1 = std::hash<litehtml::position>()(layer.border_box);
            size_t h2 = std::hash<litehtml::border_radiuses>()(layer.border_radius);
            size_t h3 = std::hash<litehtml::position>()(layer.clip_box);
            size_t h4 = std::hash<litehtml::position>()(layer.origin_box);
            size_t h5 = std::hash<int>()(layer.attachment);
            size_t h6 = std::hash<int>()(layer.repeat);
            size_t h7 = std::hash<bool>()(layer.is_root);

            // Combine hashes
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6);
        }
    };
}; // namespace std

// Equality operator defined outside the class
bool operator==(const litehtml::border_radiuses &lhs, const litehtml::border_radiuses &rhs)
{
    return lhs.top_left_x == rhs.top_left_x && lhs.top_left_y == rhs.top_left_y && lhs.top_right_x == rhs.top_right_x &&
        lhs.top_right_y == rhs.top_right_y && lhs.bottom_right_x == rhs.bottom_right_x &&
        lhs.bottom_right_y == rhs.bottom_right_y && lhs.bottom_left_x == rhs.bottom_left_x &&
        lhs.bottom_left_y == rhs.bottom_left_y;
}

bool operator==(const litehtml::position &lhs, const litehtml::position &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
}

bool operator==(const litehtml::background_layer &lhs, const litehtml::background_layer &rhs)
{
    return lhs.border_box == rhs.border_box && lhs.border_radius == rhs.border_radius && lhs.clip_box == rhs.clip_box &&
        lhs.origin_box == rhs.origin_box && lhs.attachment == rhs.attachment && lhs.repeat == rhs.repeat &&
        lhs.is_root == rhs.is_root;
}

// TODO: For image rendering we have to use set Scissor and viewport.

//=========================================================================================

WebViewContainer::WebViewContainer(
    char const * htmlAddress,
    litehtml::position clip,
    Params params
)
	: litehtml::document_container()
    , _htmlAddress(htmlAddress)
	, _solidFillRenderer(std::move(params.solidFillRenderer))
    , _imageRenderer(std::move(params.imageRenderer))
    , _borderRenderer(std::move(params.borderRenderer))
	, _requestBlob(std::move(params.requestBlob))
    , _requestFont(std::move(params.requestFont))
    , _requestImage(std::move(params.requestImage))
{
    MFA_ASSERT(std::filesystem::exists(htmlAddress));
    _parentAddress = std::filesystem::path(htmlAddress).parent_path().string();
    MFA_ASSERT(_solidFillRenderer != nullptr);
    MFA_ASSERT(_imageRenderer != nullptr);
    MFA_ASSERT(_requestBlob != nullptr);
    MFA_ASSERT(_requestFont != nullptr);
    MFA_ASSERT(_requestImage != nullptr);

    OnReload(clip);
}

//=========================================================================================

WebViewContainer::~WebViewContainer()
{
    litehtml::document::destroy_output(_gumboOutput);
    _gumboOutput = nullptr;
}

//=========================================================================================

void WebViewContainer::Update()
{
    if (_isDirty == true)
    {
        //SCOPE_Profiler("Invalidate took");
        _isDirty = false;
        SwitchActiveState();

        _html = std::make_shared<litehtml::document>(this);
        _html->update_output(_gumboOutput);
        _html->render(_clip.width, litehtml::render_all);
        _html->draw(_activeIdx, _clip.x, _clip.y, &_clip);
    }

    for (auto &state : _states)
    {
        if (state.lifeTime > 0)
        {
            state.lifeTime -= 1;
        }

        if (state.lifeTime <= 0)
        {
            std::vector<size_t> invalidKeys{};
            for (auto &[key, value] : state.imageMap)
            {
                // It is not being used anywhere
                if (value.use_count() == 1)
                {
                    _imageRenderer->FreeImageData(*value);
                    invalidKeys.emplace_back(key);
                }
            }
            for (auto const &key : invalidKeys)
            {
                state.imageMap.erase(key);
            }
            invalidKeys.clear();

            for (auto &[key, value] : state.textMap)
            {
                // It is not being used anywhere
                if (value.use_count() == 1)
                {
                    invalidKeys.emplace_back(key);
                }
            }
            for (auto const &key : invalidKeys)
            {
                state.imageMap.erase(key);
            }
            invalidKeys.clear();

            for (auto &[key, value] : state.solidMap)
            {
                // It is not being used anywhere
                if (value.use_count() == 1)
                {
                    invalidKeys.emplace_back(key);
                }
            }
            for (auto const &key : invalidKeys)
            {
                state.solidMap.erase(key);
            }
            invalidKeys.clear();

            for (auto &[key, value] : state.borderMap)
            {
                // It is not being used anywhere
                if (value.use_count() == 1)
                {
                    invalidKeys.emplace_back(key);
                }
            }
            for (auto const &key : invalidKeys)
            {
                state.borderMap.erase(key);
            }
            invalidKeys.clear();
        }
    }
    // _freeData = false;


}

//=========================================================================================

void WebViewContainer::UpdateBuffer(RT::CommandRecordState & recordState)
{
    for (auto &bufferCall : _activeState->bufferCalls)
    {
        bufferCall(recordState);
    }
}

//=========================================================================================

void WebViewContainer::DisplayPass(RT::CommandRecordState &recordState)
{
    for (auto &drawCall : _activeState->drawCalls)
    {
        drawCall(recordState);
    }
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::create_element(
	const char* tag_name,
	const litehtml::string_map& attributes,
	const std::shared_ptr<litehtml::document>& doc
)
{
    // TODO: Ideas: Include other html files with a custom tag
    // TODO: We can have custom elements like scene and game
    //MFA_LOG_INFO("create_element: %s", tag_name);
	return nullptr;
}

//=========================================================================================

litehtml::uint_ptr WebViewContainer::create_font(
	const char* faceName,
	int const size,
	int const weight,
	litehtml::font_style italic,
	unsigned int decoration,
	litehtml::font_metrics* fm
)
{
    auto fontRenderer = _requestFont(faceName);
    MFA_ASSERT(fontRenderer != nullptr);

    fm->height = static_cast<int>(fontRenderer->TextHeight((float)size));
    fm->draw_spaces = false;

    _fontList.emplace_back();
    auto & font = _fontList.back();
    font.renderer = fontRenderer;
    font.size = size;
    font.id = (int)_fontList.size();
	return font.id;
}

//=========================================================================================

void WebViewContainer::del_clip()
{
	// TODO:
}

//=========================================================================================

void WebViewContainer::delete_font(litehtml::uint_ptr hFont)
{
	// TODO
}

//=========================================================================================
// TODO: Rewrite lithtml to pass id
void WebViewContainer::draw_borders(
	litehtml::uint_ptr hdc,
	const litehtml::borders& borders,
	const litehtml::position& draw_pos,
	bool root
)
{
    std::shared_ptr<LocalBufferTracker> bufferTracker = nullptr;

    auto hash = std::hash<litehtml::position>()(draw_pos);

    auto const x = static_cast<float>(draw_pos.x);
    auto const y = static_cast<float>(draw_pos.y);
    auto const width = static_cast<float>(draw_pos.width);
    auto const height = static_cast<float>(draw_pos.height);

    glm::vec2 const topLeftPos{x, y};
    glm::vec2 const bottomLeftPos = topLeftPos + glm::vec2{0.0f, height};
    glm::vec2 const topRightPos = topLeftPos + glm::vec2{width, 0.0f};
    glm::vec2 const bottomRightPos = topLeftPos + glm::vec2{width, height};

    glm::vec4 const topColor = ConvertColor(borders.top.color);
    glm::vec4 const bottomColor = ConvertColor(borders.bottom.color);
    glm::vec4 const rightColor = ConvertColor(borders.right.color);
    glm::vec4 const leftColor = ConvertColor(borders.left.color);
    // Note: This is not 100% correct, but it works for now.
    glm::vec4 topLeftColor = glm::mix(topColor, leftColor, 0.5f);
    glm::vec4 bottomLeftColor = glm::mix(bottomColor, leftColor, 0.5f);
    glm::vec4 topRightColor = glm::mix(topColor, rightColor, 0.5f);
    glm::vec4 bottomRightColor = glm::mix(bottomColor, rightColor, 0.5f);

    auto const topLeftRadius = glm::vec2{borders.radius.top_left_x, borders.radius.top_left_y};
    auto const bottomLeftRadius = glm::vec2{borders.radius.bottom_left_x, borders.radius.bottom_left_y};
    auto const topRightRadius = glm::vec2{borders.radius.top_right_x, borders.radius.top_right_y};
    auto const bottomRightRadius = glm::vec2{borders.radius.bottom_right_x, borders.radius.bottom_right_y};

    float leftWidth = borders.left.width;
    float topWidth = borders.top.width;
    float rightWidth = borders.right.width;
    float bottomWidth = borders.bottom.width;

    auto const findResult = _activeState->borderMap.find(hash);
    if (findResult == _activeState->borderMap.end())
    {
        bufferTracker = _borderRenderer->AllocateBuffer(
            topLeftPos,
            bottomLeftPos,
            topRightPos,
            bottomRightPos,

            topLeftColor,
            bottomLeftColor,
            topRightColor,
            bottomRightColor,

            topLeftRadius,
            bottomLeftRadius,
            topRightRadius,
            bottomRightRadius,

            leftWidth,
            topWidth,
            rightWidth,
            bottomWidth
        );
        _activeState->borderMap[hash] = bufferTracker;
    }
    else
    {
        bufferTracker = findResult->second;
        _borderRenderer->UpdateBuffer(
            *bufferTracker,

            topLeftPos,
            bottomLeftPos,
            topRightPos,
            bottomRightPos,

            topLeftColor,
            bottomLeftColor,
            topRightColor,
            bottomRightColor,

            topLeftRadius,
            bottomLeftRadius,
            topRightRadius,
            bottomRightRadius,

            leftWidth,
            topWidth,
            rightWidth,
            bottomWidth
        );
    }
    // TODO: Start from here replace all these with secondary command buffer
	_activeState->drawCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
	{
	    _borderRenderer->Draw(
            recordState,
            BorderPipeline::PushConstants{.model = _modelMat},
            *bufferTracker
        );
	});

    _activeState->bufferCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
    {
        bufferTracker->Update(recordState);
    });
}

//=========================================================================================

void WebViewContainer::draw_conic_gradient(
	litehtml::uint_ptr hdc,
	const litehtml::background_layer& layer,
	const litehtml::background_layer::conic_gradient& gradient
)
{
    MFA_LOG_INFO("draw_conic_gradient");
}

//=========================================================================================

void WebViewContainer::draw_image(
	litehtml::uint_ptr hdc,
	const litehtml::background_layer& layer,
	const std::string& url,
	const std::string& base_url
)
{
    auto hash = std::hash<litehtml::background_layer>()(layer);
    auto const imagePath = Path::Get(url.c_str(), _parentAddress.c_str());
    auto [gpuTexture, _] = _requestImage(imagePath.c_str());

    if (gpuTexture == nullptr)
    {
        return;
    }

    auto const borderX = static_cast<float>(layer.border_box.x);
    auto const borderY = static_cast<float>(layer.border_box.y);

    auto const solidWidth = static_cast<float>(layer.border_box.width);
    auto const solidHeight = static_cast<float>(layer.border_box.height);

    glm::vec2 const topLeftPos{borderX, borderY};
    auto const topLeftX = (float)layer.border_radius.top_left_x;
    auto const topLeftY = (float)layer.border_radius.top_left_y;
    auto const topLeftRadius = glm::vec2{topLeftX, topLeftY};

    glm::vec2 const topRightPos = topLeftPos + glm::vec2{solidWidth, 0.0f};
    auto const topRightX = (float)layer.border_radius.top_right_x;
    auto const topRightY = (float)layer.border_radius.top_right_y;
    auto const topRightRadius = glm::vec2{topRightX, topRightY};
    ;

    glm::vec2 const bottomLeftPos = topLeftPos + glm::vec2{0.0f, solidHeight};
    auto const bottomLeftX = (float)layer.border_radius.bottom_left_x;
    auto const bottomLeftY = (float)layer.border_radius.bottom_left_y;
    auto const bottomLeftRadius = glm::vec2{bottomLeftX, bottomLeftY};

    glm::vec2 const bottomRightPos = topLeftPos + glm::vec2{solidWidth, solidHeight};
    auto const bottomRightX = (float)layer.border_radius.bottom_right_x;
    auto const bottomRightY = (float)layer.border_radius.bottom_right_y;
    auto const bottomRightRadius = glm::vec2{bottomRightX, bottomRightY};

    std::shared_ptr<ImageRenderer::ImageData> imageData = nullptr;
    auto const findResult = _activeState->imageMap.find(hash);
    if (findResult == _activeState->imageMap.end())
    {
        imageData = _imageRenderer->AllocateImageData(
            *gpuTexture,
            glm::vec3{topLeftPos, 0.0f}, glm::vec3{bottomLeftPos, 0.0f}, glm::vec3{topRightPos, 0.0f}, glm::vec3{bottomRightPos, 0.0f},
            topLeftRadius, bottomLeftRadius, topRightRadius, bottomRightRadius,
            ImagePipeline::UV{0.0f, 0.0f}, ImagePipeline::UV{0.0f, 1.0f}, ImagePipeline::UV{1.0f, 0.0f}, ImagePipeline::UV{1.0f, 1.0f}
        );
        _activeState->imageMap[hash] = imageData;
    }
    else
    {
        imageData = findResult->second;
        _imageRenderer->UpdateImageData(
            *imageData,
            *gpuTexture,
            glm::vec3{topLeftPos, 0.0f}, glm::vec3{bottomLeftPos, 0.0f}, glm::vec3{topRightPos, 0.0f}, glm::vec3{bottomRightPos, 0.0f},
            topLeftRadius, bottomLeftRadius, topRightRadius, bottomRightRadius,
            ImagePipeline::UV{0.0f, 0.0f}, ImagePipeline::UV{0.0f, 1.0f}, ImagePipeline::UV{1.0f, 0.0f}, ImagePipeline::UV{1.0f, 1.0f}
        );

    }

    _activeState->drawCalls.emplace_back([this, imageData](RT::CommandRecordState & recordState)->void
    {
        _imageRenderer->Draw(recordState, ImagePipeline::PushConstants {.model = _modelMat}, *imageData);
    });

    _activeState->bufferCalls.emplace_back([this, imageData](RT::CommandRecordState & recordState)->void
    {
        imageData->vertexData->Update(recordState);
    });
}

//=========================================================================================

void WebViewContainer::draw_linear_gradient(
	litehtml::uint_ptr hdc,
	const litehtml::background_layer& layer,
	const litehtml::background_layer::linear_gradient& gradient
)
{
    MFA_LOG_INFO("draw_linear_gradient");
}

//=========================================================================================

void WebViewContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    MFA_LOG_INFO("draw_list_marker");
}

//=========================================================================================

void WebViewContainer::draw_radial_gradient(
	litehtml::uint_ptr hdc,
	const litehtml::background_layer& layer,
	const litehtml::background_layer::radial_gradient& gradient
)
{
    MFA_LOG_INFO("draw_radial_gradient");
}

//=========================================================================================

void WebViewContainer::draw_solid_fill(
	litehtml::uint_ptr hdc,
	const litehtml::background_layer& layer,
	const litehtml::web_color& color
)
{
    std::shared_ptr<LocalBufferTracker> bufferTracker = nullptr;

    auto hash = std::hash<litehtml::background_layer>()(layer);

    auto const borderX = static_cast<float>(layer.border_box.x);
    auto const borderY = static_cast<float>(layer.border_box.y);

    auto const solidWidth = static_cast<float>(layer.border_box.width);
    auto const solidHeight = static_cast<float>(layer.border_box.height);

    glm::vec2 const topLeftPos{borderX, borderY};
    auto const topLeftColor = ConvertColor(color);
    auto const topLeftX = (float)layer.border_radius.top_left_x;
    auto const topLeftY = (float)layer.border_radius.top_left_y;
    auto const topLeftRadius = glm::vec2{topLeftX, topLeftY};

    glm::vec2 const topRightPos = topLeftPos + glm::vec2{solidWidth, 0.0f};
    auto const topRightColor = topLeftColor;
    auto const topRightX = (float)layer.border_radius.top_right_x;
    auto const topRightY = (float)layer.border_radius.top_right_y;
    auto const topRightRadius = glm::vec2{topRightX, topRightY};

    glm::vec2 const bottomLeftPos = topLeftPos + glm::vec2{0.0f, solidHeight};
    auto const bottomLeftColor = topLeftColor;
    auto const bottomLeftX = (float)layer.border_radius.bottom_left_x;
    auto const bottomLeftY = (float)layer.border_radius.bottom_left_y;
    auto const bottomLeftRadius = glm::vec2{bottomLeftX, bottomLeftY};

    glm::vec2 const bottomRightPos = topLeftPos + glm::vec2{solidWidth, solidHeight};
    auto const bottomRightColor = topLeftColor;
    auto const bottomRightX = (float)layer.border_radius.bottom_right_x;
    auto const bottomRightY = (float)layer.border_radius.bottom_right_y;
    auto const bottomRightRadius = glm::vec2{bottomRightX, bottomRightY};

    auto const findResult = _activeState->solidMap.find(hash);
    if (findResult == _activeState->solidMap.end())
    {
        bufferTracker = _solidFillRenderer->AllocateBuffer(
            topLeftPos, bottomLeftPos, topRightPos, bottomRightPos,
            topLeftColor, bottomLeftColor, topRightColor, bottomRightColor,
            topLeftRadius, bottomLeftRadius,
            topRightRadius, bottomRightRadius
        );
        _activeState->solidMap[hash] = bufferTracker;
    }
    else
    {
        bufferTracker = findResult->second;
        _solidFillRenderer->UpdateBuffer(
            *bufferTracker,
            topLeftPos, bottomLeftPos, topRightPos, bottomRightPos,
            topLeftColor, bottomLeftColor, topRightColor, bottomRightColor,
            topLeftRadius, bottomLeftRadius, topRightRadius, bottomRightRadius
        );
    }

	_activeState->drawCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
	{
	    _solidFillRenderer->Draw(
            recordState,
            SolidFillPipeline::PushConstants{.model = _modelMat},
            *bufferTracker
        );
	});

    _activeState->bufferCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
    {
        bufferTracker->Update(recordState);
    });
}

//=========================================================================================

void WebViewContainer::draw_text(
	litehtml::uint_ptr hdc,
	const char* text,
	litehtml::uint_ptr hFont,
	litehtml::web_color color,
	const litehtml::position& pos
)
{
    auto const hash = std::hash<litehtml::position>()(pos);
    auto & fontData = _fontList[hFont - 1];

    auto const x = static_cast<float>(pos.x);
    auto const y = static_cast<float>(pos.y);

    FontRenderer::TextParams textParams{};
    textParams.color = ConvertColor(color);
    textParams.hTextAlign = FontRenderer::HorizontalTextAlign::Left;
    textParams.fontSizeInPixels = (float)fontData.size;

    std::shared_ptr<FontRenderer::TextData> textData = nullptr;
    auto const findResult = _activeState->textMap.find(hash);
    if (findResult == _activeState->textMap.end())
    {
        textData = fontData.renderer->AllocateTextData(1024);
        _activeState->textMap[hash] = textData;
    }
    else
    {
        textData = findResult->second;
    }

    fontData.renderer->ResetText(*textData);
    fontData.renderer->AddText(*textData, text, x, y, textParams);

    _activeState->drawCalls.emplace_back([this, textData, &fontData](RT::CommandRecordState &recordState) -> void
	{
	    fontData.renderer->Draw(
            recordState,
            TextOverlayPipeline::PushConstants{ .model = _modelMat },
            *textData
        );
	});

    _activeState->bufferCalls.emplace_back([this, textData](RT::CommandRecordState &recordState) -> void
    {
        textData->vertexData->Update(recordState);
    });
}

//=========================================================================================

void WebViewContainer::get_client_rect(litehtml::position& client) const
{
	client.width = LogicalDevice::GetWindowWidth();
	client.height = LogicalDevice::GetWindowHeight();
	client.x = 0;
	client.y = 0;
}

//=========================================================================================

const char* WebViewContainer::get_default_font_name() const
{
	return "consolos";
}

//=========================================================================================

int WebViewContainer::get_default_font_size() const
{
	return (int)FontRenderer::DefaultFontSize;
}

//=========================================================================================

void WebViewContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    auto const imagePath = Path::Get(src, _parentAddress.c_str());
    int width, height, channels;
    if (stbi_info(imagePath.c_str(), &width, &height, &channels) > 0) {
        sz.width = width;
        sz.height = height;
    }
}

//=========================================================================================

void WebViewContainer::get_language(litehtml::string& language, litehtml::string& culture) const
{
}

//=========================================================================================

void WebViewContainer::get_media_features(litehtml::media_features& media) const
{
    media.width = LogicalDevice::GetWindowWidth();
    media.height = LogicalDevice::GetWindowHeight();
    media.device_width = media.width;
    media.device_height = media.height;
    media.color = 4;
    media.type = litehtml::media_type_screen;
    media.resolution = 1;
}

//=========================================================================================

void WebViewContainer::import_css(
    litehtml::string& text,
    const litehtml::string& url,
    litehtml::string& baseurl
)
{
    // MFA_LOG_INFO(
    //     "Requested import css.\nText: %s\nurl: %s\nbaseurl: %s"
    //     , text.c_str(), url.c_str(), baseurl.c_str()
    // );
    auto const cssPath = Path::Get(url.c_str(), _parentAddress.c_str());
    auto const cssBlob = _requestBlob(cssPath.c_str(), false);
    text = litehtml::string(cssBlob->As<char const>(), cssBlob->Len());
}

//=========================================================================================

void WebViewContainer::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
}

//=========================================================================================

void WebViewContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{

}

//=========================================================================================

void WebViewContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
}

//=========================================================================================

void WebViewContainer::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
	
}

//=========================================================================================

int WebViewContainer::pt_to_px(int pt) const
{
    auto const windowWidth = static_cast<float>(LogicalDevice::GetWindowWidth());
    auto const windowHeight = static_cast<float>(LogicalDevice::GetWindowHeight());

    float bodyWidth = _bodyWidth;
    if (bodyWidth <= 0)
    {
        bodyWidth = windowWidth;
    }
    float const wScale = (float)windowWidth / bodyWidth;

    float bodyHeight = _bodyHeight;
    if (bodyHeight <= 0)
    {
        bodyHeight = windowHeight;
    }
    float const hScale = (float)windowHeight / bodyHeight;

    float scaleFactor = std::min(wScale, hScale);
    scaleFactor = std::max(scaleFactor, 1.0f);

    return pt * scaleFactor;
}

//=========================================================================================

void WebViewContainer::set_base_url(const char* base_url)
{
}

//=========================================================================================

void WebViewContainer::set_caption(const char* caption)
{
}

//=========================================================================================

void WebViewContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
}

//=========================================================================================

void WebViewContainer::set_cursor(const char* cursor)
{
}

//=========================================================================================

int WebViewContainer::text_width(const char* text, litehtml::uint_ptr hFont)
{
    auto & fontData = _fontList[hFont - 1];
    auto const textWidth = fontData.renderer->TextWidth(std::string_view{text, strlen(text)}, fontData.size);
    return static_cast<int>(textWidth);
}

//=========================================================================================

void WebViewContainer::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    MFA_LOG_INFO("transform_text");
}

//=========================================================================================

glm::vec4 WebViewContainer::ConvertColor(litehtml::web_color const& webColor)
{
	return glm::vec4
	{
		static_cast<float>(webColor.red) / 255.0f,
		static_cast<float>(webColor.green) / 255.0f,
		static_cast<float>(webColor.blue) / 255.0f,
		static_cast<float>(webColor.alpha) / 255.0f
	};
}

//=========================================================================================

void WebViewContainer::SwitchActiveState()
{
    if (_activeState != nullptr)
    {
        _activeState->lifeTime = LogicalDevice::GetMaxFramePerFlight();
    }
    _activeIdx = -1;
    for (int i = 0; i < _states.size(); ++i)
    {
        if (_states[i].lifeTime <= 0)
        {
            _activeIdx = i;
        }
    }
    if (_activeIdx < 0)
    {
        _activeIdx = _states.size();
        _states.emplace_back();
    }
    _activeState = &_states[_activeIdx];
    _activeState->drawCalls.clear();
    _activeState->bufferCalls.clear();
}

//=========================================================================================

GumboNode *WebViewContainer::GetElementById(const char *id) { return GetElementById(id, _gumboOutput->root); }
GumboNode *WebViewContainer::GetElementByTag(const char *tag) {return GetElementByTag(tag, _gumboOutput->root); }

//=========================================================================================

void WebViewContainer::SetText(GumboNode *node, char const *text)
{
    if (node && node->type == GUMBO_NODE_ELEMENT)
    {
        GumboVector *children = &node->v.element.children;

        for (unsigned int i = 0; i < children->length; ++i)
        {
            GumboNode *child = static_cast<GumboNode *>(children->data[i]);

            if (child->type == GUMBO_NODE_TEXT)
            {
                if (child->v.text.text != nullptr)
                {
                    free((void *)child->v.text.text);
                }
                // Replace the text by modifying the C string (not ideal)
                child->v.text.text = strdup(text); // Replace with your own string
                _isDirty = true;
                break;
            }
        }
    }
}

//=========================================================================================

void WebViewContainer::AddClass(GumboNode *node, char const *keyword)
{
    // _isDirty = true;
    // return;
    if (node == nullptr || node->type != GUMBO_NODE_ELEMENT || keyword == nullptr || strlen(keyword) == 0)
        return;

    GumboVector *attributes = &node->v.element.attributes;
    GumboAttribute *class_attr = gumbo_get_attribute(attributes, "class");

    if (class_attr == nullptr)
    {
        // Class doesn't exist, so we create a new one
        class_attr = (GumboAttribute *)malloc(sizeof(GumboAttribute));
        class_attr->name = strdup("class");
        class_attr->value = strdup(keyword);
        // Allocate new array for attributes (length + 1)
        void** new_data = (void**)malloc(sizeof(void*) * (attributes->length + 1));

        // Copy existing attribute pointers
        for (unsigned int i = 0; i < attributes->length; ++i) {
            new_data[i] = attributes->data[i];
        }

        // Add new attribute pointer
        new_data[attributes->length] = class_attr;

        // Free old attributes->data array (not the attributes themselves!)
        free(attributes->data);

        // Assign new array and update length
        attributes->data = new_data;
        attributes->length += 1;

        _isDirty = true;
    }
    else
    {
        // Check if keyword already exists
        std::string existing = class_attr->value;
        std::istringstream iss(existing);
        std::string word;
        bool found = false;
        while (iss >> word)
        {
            if (word == keyword)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Append the keyword to existing class list
            std::string updated = existing + " " + keyword;

            free((void *)class_attr->value);  // Free old value
            class_attr->value = strdup(updated.c_str());

            _isDirty = true;
        }
    }
}

//=========================================================================================

void WebViewContainer::RemoveClass(GumboNode *node, char const *keyword)
{
    if (node == nullptr || node->type != GUMBO_NODE_ELEMENT || keyword == nullptr)
        return;

    auto keywordLength = strlen(keyword);
    if (keywordLength == 0)
    {
        return;
    }

    GumboVector *attributes = &node->v.element.attributes;
    GumboAttribute *class_attr = gumbo_get_attribute(attributes, "class");

    if (class_attr != nullptr)
    {
        // Check if keyword already exists
        std::string value = class_attr->value;
        auto const startPos = value.find(keyword);
        if (startPos != std::string::npos)
        {
            value.erase(startPos, keywordLength);
            free((void *)class_attr->value);  // Free old value
            class_attr->value = strdup(value.c_str());

            _isDirty = true;
        }
    }
}

//=========================================================================================

GumboNode *WebViewContainer::GetElementById(const char *id, GumboNode *node)
{
    if (GUMBO_NODE_ELEMENT != node->type)
    {
        return nullptr;
    }

    GumboAttribute *node_id = gumbo_get_attribute(&node->v.element.attributes, "id");
    if (node_id && 0 == strcmp(id, node_id->value))
    {
        return node;
    }

    // iterate all children
    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; i++)
    {
        GumboNode *node = GetElementById(id, (GumboNode *)children->data[i]);
        if (node)
            return node;
    }

    return nullptr;
}

//=========================================================================================

GumboNode *WebViewContainer::GetElementByTag(const char *tag, GumboNode *node)
{
    if (!node || GUMBO_NODE_ELEMENT != node->type)
        return nullptr;

    if (strcmp(gumbo_normalized_tagname(node->v.element.tag), tag) == 0)
        return node;

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* found = GetElementByTag(tag, static_cast<GumboNode*>(children->data[i]));
        if (found)
            return found;
    }

    return nullptr;
}

//=========================================================================================

void WebViewContainer::OnReload(litehtml::position clip)
{
    if (_gumboOutput != nullptr)
    {
        litehtml::document::destroy_output(_gumboOutput);
        _gumboOutput = nullptr;
    }
    _htmlBlob = _requestBlob(_htmlAddress.c_str(), false);
    char const *htmlText = _htmlBlob->As<char const>();
    _gumboOutput = litehtml::document::parse_html(htmlText);

    auto const bodyTag = GetElementByTag("body");
    if (bodyTag != nullptr)
    {
        GumboAttribute* widthAttr = gumbo_get_attribute(&bodyTag->v.element.attributes, "width");
        GumboAttribute* heightAttr = gumbo_get_attribute(&bodyTag->v.element.attributes, "height");

        try {
            _bodyWidth = widthAttr ? std::stoi(widthAttr->value) : -1;
            _bodyHeight = heightAttr ? std::stoi(heightAttr->value) : -1;
        }
        catch (const std::exception& e)
        {
            MFA_LOG_WARN("Failed to parse body width and height: %s", e.what());
        }
    }

    OnResize(std::move(clip));
}

//=========================================================================================

void WebViewContainer::OnResize(litehtml::position clip)
{
    auto const windowWidth = static_cast<float>(LogicalDevice::GetWindowWidth());
    auto const windowHeight = static_cast<float>(LogicalDevice::GetWindowHeight());

    float scaleFactor = 1.0f;

    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;
    float scaleX = (1.0f / halfWidth) * scaleFactor;
    float scaleY = (1.0f / halfHeight) * scaleFactor;

    _clip = std::move(clip);

    _modelMat = glm::transpose(glm::scale(glm::identity<glm::mat4>(), glm::vec3{scaleX, scaleY, 1.0f}) *
                               glm::translate(glm::identity<glm::mat4>(), glm::vec3{-halfWidth, -halfHeight, 0.0f}));

    _isDirty = true;
}

//=========================================================================================
