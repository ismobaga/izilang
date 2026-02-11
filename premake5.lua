workspace "IziLang"
-- architecture "x64"
configurations {"Debug", "Release"}
startproject "IziLang"
cppdialect "c++20"

outputdir = "%{cfg.buildcfg}"

project "izi"
location "."
kind "ConsoleApp"
language "C++"
staticruntime "on"

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("obj/" .. outputdir .. "/%{prj.name}")

files {"src/**.h", "src/**.hpp", "src/**.c", "src/**.cpp"}

includedirs {"src"}

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

project "tests"
location "tests"
kind "ConsoleApp"
language "C++"
staticruntime "on"

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("obj/" .. outputdir .. "/%{prj.name}")

files {
    "tests/**.cpp",
    "src/**.cpp"
}

removefiles {
    "src/main.cpp"  -- Exclude main.cpp as we have our own test main
}

includedirs {
    "src",
    "tests"
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
