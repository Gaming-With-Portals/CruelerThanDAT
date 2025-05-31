local directx = os.getenv("DXSDK_DIR")
local fbx = os.getenv("FBXSDK_DIR")

if not directx then
	error("DirectX 2010 SDK not found. To solve, install it if you haven't already, and then make sure DXSDK_DIR is set properly, if not, do it manually.")
end

if not fbx then
	error("FBX SDK not found. Install it if you haven't already, and then set the FBXSDK_DIR variable to the path for it manually.")
end

workspace "CruelerThanDAT"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"

	-- I'm not sure how to add x86 support, but I'm not gonna look into it and I'll leave it at x64 only
	-- because nobody is running an x86 machine anyway, and supporting both can present difficulties with
	-- type sizes sometimes, on the kind of language C++ is. Zig for example would not have this problem
	-- as it's designed with portability across architectures and basic-type consistency in mind.
	architecture "x86_64"

	configurations { "Debug", "Release", }

	targetdir ("build-%{cfg.longname}/")
	objdir ("build-obj/")

	filter { "configurations:Debug" }
		symbols "On"
		optimize "Off"
		linktimeoptimization "Off"
		
	filter { "configurations:Release" }
		symbols "On"
		optimize "Full"
		linktimeoptimization "On"

	include("premake5-curl.lua")

	project "CruelerThanDAT"
		-- For this project, WindowedApp might make more sense. Note there's no difference
		-- in Linux, but on Windows and MacOS, the distinction is important according to
		-- the Premake documentation.
		kind "ConsoleApp"

		pchsource "CruelerThanDAT/pch.cpp"
		pchheader "pch.hpp"

		if (
			"gmake" == _ACTION or
			"gmakelegacy" == _ACTION) then
			toolset "clang"
			linkoptions { "-fuse-ld=lld" }

			files {
				"CruelerThanDAT/CruelerThanDAT.cpp",
				"CruelerThanDAT/src/**.cpp",
				"CruelerThanDAT/inc/**.hpp",
				"CruelerThanDAT/inc/**.h",

				"depends/imgui/src/**.cpp",
				"depends/imgui/inc/**.h",
				"depends/imstb/**.h",
				"depends/json/**.hpp",
			}

			libdirs {
				-- We can put this in a filter if we wanna link the debug build
				-- of libfbxsdk for the debug configuration.
				path.join(fbx,		"lib/x64/release/"),
				path.join(directx,	"Lib/x64/"),

				"build-curl/%{cfg.longname}/lib/",
			}
		elseif (
			"vs2022" == _ACTION or
			"vs2019" == _ACTION or
			"vs2017" == _ACTION or
			"vs2015" == _ACTION or
			"vs2013" == _ACTION or
			"vs2012" == _ACTION or
			"vs2010" == _ACTION or
			"vs2008" == _ACTION or
			"vs2005" == _ACTION) then
			toolset "msc"
			libdirs {
				-- We can put this in a filter if we wanna link the debug build
				-- of libfbxsdk for the debug configuration.
				path.join(fbx,		"lib/x64/release/"),
				path.join(directx,	"Lib/x64/"),

				"build-curl/%{cfg.longname}/lib/%{cfg.longname}/",
			}

			files {
				"CruelerThanDAT/pch.cpp",
				"CruelerThanDAT/CruelerThanDAT.cpp",
				"CruelerThanDAT/src/**.cpp",
				"CruelerThanDAT/inc/**.hpp",
				"CruelerThanDAT/inc/**.h",
				"CruelerThanDAT/**.rc", -- TODO: Remove RC files and do it the standard way
	
				"depends/imgui/src/**.cpp",
				"depends/imgui/inc/**.h",
				"depends/imstb/**.h",
				"depends/json/**.hpp",
			}
		else
			error("Action not supported")
		end

		defines {
			"mLefttChild=mLeftChild", -- Fix typo in the FBX SDK from our end with macros.
			"UNICODE",
		}

		postbuildcommands {
			'{COPYFILE} "%{prj.location}/libfbxsdk.dll" "%{cfg.buildtarget.directory}/libfbxsdk.dll"',
			'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"',
			'{COPYFILE} "%{prj.location}/depends/curl/COPYING" "%{cfg.buildtarget.directory}/CURL_LICENSE"',
		}

		includedirs {
			"CruelerThanDAT/inc/",
		}

		externalincludedirs {
			path.join(directx,	"Include/"),
			path.join(fbx,		"include/"),

			"depends/curl/include/curl/",
			"depends/imgui/inc/",
			"depends/imstb/",
			"depends/json/",
		}

		filter { "toolset:clang" }
			buildoptions "-Wall"
		filter { "toolset:msc" }
			warnings "High"
		filter { "files:depends/imgui/**.cpp", "toolset:clang" }
			buildoptions "-Wno-everything"
		filter { "files:depends/imgui/**.cpp", "toolset:msc" }
			buildoptions "/w"

		filter { "configurations:Debug" }
			links {
				"urlmon",
				"wldap32",
				"advapi32",
				"crypt32",
				"normaliz",
				"ws2_32",
				"comdlg32",
				"ole32",

				"d3dx9d",
				"d3d9",
				"libfbxsdk",
				"libcurl-d",
			}
		
		filter { "configurations:Release" }
			links {
				"urlmon",
				"wldap32",
				"advapi32",
				"crypt32",
				"normaliz",
				"ws2_32",
				"comdlg32",
				"ole32",

				"d3dx9",
				"d3d9",
				"libfbxsdk",
				"libcurl",
			}
