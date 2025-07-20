#pragma once

#include <memory>
#include <string>

//https://stackoverflow.com/questions/4815423/how-do-i-set-the-working-directory-to-the-solution-directory
namespace MFA
{
	class Path
	{
	public:

	    [[nodiscard]]
	    static std::unique_ptr<Path> Init();

		explicit Path();

		~Path();

		// Returns correct address based on platform
		[[nodiscard]]
		static std::string Get(std::string const& address);

	    [[nodiscard]]
        static std::string Get(char const * address);

	    [[nodiscard]]
	    static std::string Get(char const * address, char const *relativePath);

	    [[nodiscard]]
	    static std::string Relative(char const * address);

        [[nodiscard]]
        static std::string AssetPath();

    private:

		inline static Path * _instance {};
		std::string mAssetPath {};

	};
};