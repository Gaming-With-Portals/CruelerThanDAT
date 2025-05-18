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

	project "cURL"
		kind "Makefile"
		language "C"
		location "depends/curl/"

		if ("gmake" == _ACTION or
			"gmakelegacy" == _ACTION) then
				buildcommands {
					-- TODO: Filter each VS version separately for this command to match it.
					"cmake -S . -B ../../build-curl/%{cfg.longname}/ -G \"Unix Makefiles\" -DBUILD_SHARED_LIBS=OFF -DCURL_STATICLIB=ON -DCURL_USE_LIBPSL=OFF -DCMAKE_BUILD_TYPE=%{cfg.longname} -DCMAKE_C_COMPILER=clang",
					-- IMPORTANT TODO: ADD LIBPSL TO THE CULR BUILD INSTEAD OF SKIPPING IT.                                      ^~~~~~~~~~~~~~~~~~~~^
					--                 THIS IS IMPORTANT BECAUSE CURL USES LIBPSL TO PROTECT
					--                 AGAINST HACKING ATTEMPTS RELATED TO COOKIES.
					--                 DO NOT MERGE THIS BRANCH UNTIL THIS IS FIXED.
	
					-- TODO: Add an option somehow so -j4 can be changed by the user. -j4 means multi-threaded build with four threads.
					"cmake --build ../../build-curl/%{cfg.longname}/ --config %{cfg.longname} -j4",
				}
				filter { "configurations:Debug" }
					buildoutputs { "../../build-curl/%{cfg.longname}/lib/%{cfg.longname}/libcurl-d.lib" }
				filter { "configurations:Release" }
					buildoutputs { "../../build-curl/%{cfg.longname}/lib/%{cfg.longname}/libcurl.lib" }
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
			buildcommands {
				-- TODO: Filter each VS version separately for this command to match it.
				"cmake -S . -B ../../build-curl/%{cfg.longname}/ -G \"Visual Studio 17 2022\" -DBUILD_SHARED_LIBS=OFF -DCURL_STATICLIB=ON -DCURL_USE_LIBPSL=OFF -DCMAKE_BUILD_TYPE=%{cfg.longname}",
				-- IMPORTANT TODO: ADD LIBPSL TO THE CULR BUILD INSTEAD OF SKIPPING IT.                           ^~~~~~~~~~~~~~~~~~~~^
				--                 THIS IS IMPORTANT BECAUSE CURL USES LIBPSL TO PROTECT
				--                 AGAINST HACKING ATTEMPTS RELATED TO COOKIES.
				--                 DO NOT MERGE THIS BRANCH UNTIL THIS IS FIXED.

				-- TODO: Add an option somehow so -j4 can be changed by the user. -j4 means multi-threaded build with four threads.
				"cmake --build ../../build-curl/%{cfg.longname}/ --config %{cfg.longname} -j4",
			}
			filter { "configurations:Debug" }
				buildoutputs { "../../build-curl/%{cfg.longname}/lib/%{cfg.longname}/libcurl-d.lib" }
			filter { "configurations:Debug" }
				buildoutputs { "../../build-curl/%{cfg.longname}/lib/%{cfg.longname}/libcurl.lib" }
		else
			error("Action not supported")
		end

	project "CruelerThanDAT"
		-- For this project, WindowedApp might make more sense. Note there's no difference
		-- in Linux, but on Windows and MacOS, the distinction is important according to
		-- the Premake documentation.
		kind "ConsoleApp"

		if (
			"gmake" == _ACTION or
			"gmakelegacy" == _ACTION) then
			toolset "clang"
			linkoptions { "-fuse-ld=lld" }
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
		else
			error("Action not supported")
		end

		defines {
			"mLefttChild=mLeftChild", -- Fix typo in the FBX SDK from our end with macros.
		}

		postbuildcommands {
			'{COPY} "%{prj.location}/CruelerThanDAT/Assets" "%{cfg.buildtarget.directory}/Assets"'
		}

		includedirs {
			path.join(directx,	"Include/"),
			path.join(fbx,		"include/"),

			"depends/curl/include/"
		}

		files {
			"CruelerThanDAT/**.cpp",
			"CruelerThanDAT/**.hpp",
			"CruelerThanDAT/**.h",

			-- TODO: Filter this to add it to VS but not Make
			--"CruelerThanDAT/**.rc",
		}

		filter { "configurations:Debug" }
			links {
				"urlmon",
				"wldap32",
				"advapi32",
				"crypt32",
				"normaliz",
				"ws2_32",
				"comdlg32",

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

				"d3dx9d",
				"d3d9",
				"libfbxsdk",
				"libcurl",
			}
