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
