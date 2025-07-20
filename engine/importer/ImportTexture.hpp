#pragma once

#include "AssetTexture.hpp"

#include <memory>
#include <string>

namespace MFA::Importer
{

    [[nodiscard]]
    std::shared_ptr<Asset::Texture> UncompressedImage(std::string const& path);

    [[nodiscard]]
    std::shared_ptr<Asset::Texture> ErrorTexture();

    // std::shared_ptr<Asset::Texture> LoadKtxMetadata(char const * path);

}
