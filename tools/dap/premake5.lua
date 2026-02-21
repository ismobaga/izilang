workspace "IziLangDAP"
configurations {"Debug", "Release"}
cppdialect "c++20"

outputdir = "%{cfg.buildcfg}"

-- Path to main izilang source
IZILANG_SRC = "../../src"

project "izilang-dap"
location "."
kind "ConsoleApp"
language "C++"
staticruntime "on"

targetdir("bin/" .. outputdir)
objdir("obj/" .. outputdir)

files {
    "src/**.h",
    "src/**.hpp",
    "src/**.cpp",
    IZILANG_SRC .. "/**.h",
    IZILANG_SRC .. "/**.hpp",
    IZILANG_SRC .. "/**.cpp"
}

removefiles {
    IZILANG_SRC .. "/main.cpp"
}

includedirs {
    "src",
    "third_party",
    IZILANG_SRC
}

filter "configurations:Debug"
runtime "Debug"
symbols "on"

filter "configurations:Release"
runtime "Release"
optimize "on"

filter "system:windows"
systemversion "latest"

filter "system:linux"
links {"m", "dl", "pthread"}

filter "system:macosx"
links {"m", "pthread"}
