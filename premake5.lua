-- This variable is gonna be set automatically by the DirectX 2010 SDK installer and
-- therefore we get this in whatever path it is just from it being installed.
-- If for whatever weird reason, the SDK is installed but this variable wasn't set, or
-- the user unset it somehow, they'll have to set it manually.
local directx = os.getenv("DXSDK_DIR")

-- The FBX SDK doesn't set a variable to get the path to it, so let's just agree to set
-- it manually ourselves for the sake of having a sane, portable project script.
local fbx = os.getenv("FBXSDK_DIR")

-- As far as I'm aware, on Windows, you don't do installing libcurl, only the curl command,
-- so we're gonna do the same as with the FBX SDK.
local curl = os.getenv("CURL_DIR")

if not directx then
	error("DirectX 2010 SDK not found. To solve, install it if you haven't already, and then make sure DXSDK_DIR is set properly.")
end

if not fbx then
	error("FBX SDK not found. Install it if you haven't already, and then set the FBXSDK_DIR variable to the path for it manually.")
end

if not curl then
	error("cURL not found. Install it if you haven't already, and then set the CURL_DIR variable to the path for it manually. This project expects cURL to be compiled with MSVC.")
end

workspace "CruelerThanDAT" -- Visual Studio SLN equivalent.
	language "C++"
	cppdialect "C++20"

	-- I'm not sure how to add x86 support, but I'm not gonna look into it and I'll leave it at x64 only
	-- because nobody is running an x86 machine anyway, and supporting both can present difficulties with
	-- type sizes sometimes, on the kind of language C++ is. Zig for example would not have this problem
	-- as it's designed with portability across architectures and basic-type consistency in mind.
	architecture "x86_64"

	configurations { "Debug", "Release", } -- You can add more if you want.

	filter { "configurations:Debug" }
		symbols "On"
		optimize "Off"
		linktimeoptimization "Off"

	filter { "configurations:Release" }
		symbols "On"
		optimize "Full"
		linktimeoptimization "On"

	filter {} -- Maybe unnecessary?

	targetdir ("build-%{cfg.longname}/")
	objdir ("build-obj/%{cfg.longname}/")

	project "CruelerThanDAT"
		-- For this project, WindowedApp might make more sense. Note there's no difference
		-- in Linux, but on Windows and MacOS, the distinction is important according to
		-- the Premake documentation.
		kind "ConsoleApp"

		-- This is the compiler. msc means MSVC, but it can also be set to gcc, clang or dotnet.
		toolset "msc"

		-- This resolves to a better post build command than the original file, as Premake will
		-- make it check that Assets exists before the copy command. Also this is cross platform
		-- which is always nice, but for this project we don't care about that part.
		postbuildcommands {
			'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"'
		}

		-- Ideally you'd wanna have a separate folder for headers, and place it here.
		includedirs {
			path.join(directx,	"Include/"),
			path.join(fbx,		"include/"),
			path.join(curl,		"include/"),
		}

		libdirs {
			-- We can put this in a filter if we wanna link the debug build
			-- of libfbxsdk for the debug configuration.
			path.join(fbx,		"lib/x64/release/"),

			path.join(directx,	"Lib/x64/"),
			path.join(curl,		"lib/"),
		}

		links {
			"urlmon",
			"wldap32",
			"advapi32",
			"crypt32",
			"normaliz",
			"ws2_32",

			"d3dx9d",
			"d3d9",
			"libfbxsdk",
			"libcurl_a", -- Not sure why but when *I* built libcurl, the file came out as libcurl_a.lib, not libcurl.lib.
		}
		
		files {
			"CruelerThanDAT/**.cpp",
			"CruelerThanDAT/**.hpp",
			"CruelerThanDAT/**.h",

			-- Ideally let's avoid Microsoft-y things like this for the sake of portability, but that'll be a separate PR.
			"CruelerThanDAT/**.rc",
		}
