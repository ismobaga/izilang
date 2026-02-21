workspace "IziLang"
-- architecture "x64"
configurations {"Debug", "Release"}
startproject "IziLang"
cppdialect "c++20"

outputdir = "%{cfg.buildcfg}"

-- Optional raylib support: set RAYLIB_DIR environment variable or pass --raylib=path
newoption {
    trigger = "raylib",
    value = "PATH",
    description = "Path to raylib installation (enables HAVE_RAYLIB)"
}

project "izi"
location "."
kind "ConsoleApp"
language "C++"
staticruntime "on"

targetdir("bin/" .. outputdir .. "/%{prj.name}")
objdir("obj/" .. outputdir .. "/%{prj.name}")

files {"src/**.h", "src/**.hpp", "src/**.c", "src/**.cpp"}

includedirs {"src"}

if _OPTIONS["raylib"] then
    defines {"HAVE_RAYLIB"}
    includedirs {_OPTIONS["raylib"] .. "/include"}
    libdirs {_OPTIONS["raylib"] .. "/lib"}
    links {"raylib"}
end

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
if _OPTIONS["raylib"] then
    links {"GL", "X11"}
end

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

if _OPTIONS["raylib"] then
    defines {"HAVE_RAYLIB"}
    includedirs {_OPTIONS["raylib"] .. "/include"}
    libdirs {_OPTIONS["raylib"] .. "/lib"}
    links {"raylib"}
end

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
if _OPTIONS["raylib"] then
    links {"GL", "X11"}
end
