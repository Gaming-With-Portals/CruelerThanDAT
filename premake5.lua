local fbx = os.getenv("FBXSDK_DIR")

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
			
			filter { "configurations:Debug" }
				libdirs {
					path.join(fbx,		"lib/x64/debug/"),
					"depends/SDL3/lib/x64/",
					"build-curl/%{cfg.longname}/lib/",
				}
				
				local fbxsdkdll = path.join(fbx, "lib/x64/debug/libfbxsdk.dll")
				postbuildcommands {
					('{COPYFILE} "' .. fbxsdkdll .. '" "%{cfg.buildtarget.directory}/libfbxsdk.dll"'),
					'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/lib/x64/SDL3.dll" "%{cfg.buildtarget.directory}/SDL3.dll"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/LICENSE.txt" "%{cfg.buildtarget.directory}/SDL3_LICENSE"',
				}
			filter { "configurations:Release" }
				libdirs {
					path.join(fbx,		"lib/x64/release/"),
					"depends/SDL3/lib/x64/",
					"build-curl/%{cfg.longname}/lib/",
				}
				
				local fbxsdkdll = path.join(fbx, "lib/x64/release/libfbxsdk.dll")
				postbuildcommands {
					('{COPYFILE} "' .. fbxsdkdll .. '" "%{cfg.buildtarget.directory}/libfbxsdk.dll"'),
					'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"',
					'{COPYFILE} "%{prj.location}/depends/curl/COPYING" "%{cfg.buildtarget.directory}/CURL_LICENSE"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/lib/x64/SDL3.dll" "%{cfg.buildtarget.directory}/SDL3.dll"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/LICENSE.txt" "%{cfg.buildtarget.directory}/SDL3_LICENSE"',
				}
			filter {}

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
			filter { "configurations:Debug" }
				libdirs {
					path.join(fbx,		"lib/x64/debug/"),
					"depends/SDL3/lib/x64/",
					"build-curl/%{cfg.longname}/lib/%{cfg.longname}/",
				}
				
				local fbxsdkdll = path.join(fbx, "lib/x64/debug/libfbxsdk.dll")
				postbuildcommands {
					('{COPYFILE} "' .. fbxsdkdll .. '" "%{cfg.buildtarget.directory}/libfbxsdk.dll"'),
					'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"',
					'{COPYFILE} "%{prj.location}/depends/curl/COPYING" "%{cfg.buildtarget.directory}/CURL_LICENSE"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/lib/x64/SDL3.dll" "%{cfg.buildtarget.directory}/SDL3.dll"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/LICENSE.txt" "%{cfg.buildtarget.directory}/SDL3_LICENSE"',
				}
			filter { "configurations:Release" }
				libdirs {
					path.join(fbx,		"lib/x64/release/"),
					"depends/SDL3/lib/x64/",
					"build-curl/%{cfg.longname}/lib/%{cfg.longname}/",
				}
				
				local fbxsdkdll = path.join(fbx, "lib/x64/release/libfbxsdk.dll")
				postbuildcommands {
					('{COPYFILE} "' .. fbxsdkdll .. '" "%{cfg.buildtarget.directory}/libfbxsdk.dll"'),
					'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"',
					'{COPYFILE} "%{prj.location}/depends/curl/COPYING" "%{cfg.buildtarget.directory}/CURL_LICENSE"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/lib/x64/SDL3.dll" "%{cfg.buildtarget.directory}/SDL3.dll"',
					'{COPYFILE} "%{prj.location}/depends/SDL3/LICENSE.txt" "%{cfg.buildtarget.directory}/SDL3_LICENSE"',
				}
			filter {}

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

		includedirs {
			"CruelerThanDAT/inc/",
		}

		externalincludedirs {
			path.join(fbx,		"include/"),
			"depends/GLAD/src",
			"depends/curl/include/curl/",
			"depends/SDL3/include",
			"depends/imgui/inc/",
			"depends/imstb/",
			"depends/json/",
			"depends/GLAD/include",
			"depends/gli",
			"depends/glm",
		}

		filter { "toolset:clang" }
			buildoptions {"-Wno-all","-Wno-error",}
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
				"SDL3",
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
				"SDL3",
				"libfbxsdk",
				"libcurl",
			}
