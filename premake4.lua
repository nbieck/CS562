--[[
	Premake script created by Niklas Bieck
]]

-- set a bunch of variables here, so that the script can be easily modified

-- Any variable that is a table should have one entry per project, as it stores per-project settings
SOLUTION_NAME_ = "CS562"
PROJECTS_ = { "CS562" }
KINDS = {"ConsoleApp"}
OBJDIR = "obj"
BINDIR = "bin"
LIBDIR = "libs"
INCLUDEDIR = "include"
SOURCEDIR = "src"
DEFINES = {"GLM_FORCE_RADIANS"}
-- These are tables of tables, as we might want multiple links per project
-- LIBS are libraries that will always be linked
-- The other two are configuration specific
LIBS = {{"opengl32"}}
DEBUGLIBS = {{}}
RELEASELIBS = {{}}

--generate folders if they do not yet exist
folders = {OBJDIR, BINDIR, LIBDIR, INCLUDEDIR, SOURCEDIR, "dlls"}
for k,v in pairs(folders) do
	matches = os.matchdirs(v)
	if (#matches == 0) then
		os.mkdir(v)
	end
end

-- generate solution etc.
solution(SOLUTION_NAME_)
	configurations { "Debug", "Release", "ReleaseWithDebug" }
	platforms {"x32", "x64"}
	
	filter "platforms:x32"
		architecture "x32"
		
	filter "platforms:x64"
		architecture "x64"
	
	filter {}
	
	includedirs { INCLUDEDIR, SOURCEDIR }
	libdirs { LIBDIR }
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	filter {}
		
	-- generate all the projects specified
	for key,val in pairs(PROJECTS_) do
		project (val)
			kind (KINDS[key])
			defines (DEFINES)
            debugdir "$(SolutionDir)"
			location "./projects"
			language "C++"
			files { SOURCEDIR.."/"..val.."/**.h*", SOURCEDIR.."/"..val.."/**.c*", SOURCEDIR.."/"..val.."/**.inl" }
			vpaths { 
				["*"] = SOURCEDIR.."/"..val } 
                
            configuration "Debug"
                defines { "_DEBUG" }
                flags { "Symbols", "FatalWarnings" }
                targetdir (BINDIR.."/Debug")
                links (LIBS[key])
                links (DEBUGLIBS[key])
                objdir (OBJDIR.."/Debug")
				
            configuration "Release"
                defines {"NDEBUG"}
                flags {"Optimize", "FatalWarnings"}
                targetdir (BINDIR.."/Release")
                links (LIBS[key])
                links (RELEASELIBS[key])
                objdir (OBJDIR.."/Release")

            configuration "ReleaseWithDebug"
            	defines {"NDEBUG"}
            	flags {"Optimize", "FatalWarnings", "Symbols"}
            	targetdir (BINDIR.."/RelSymbols")
            	links(LIBS[key])
            	links(RELEASELIBS[key])
            	objdir(OBJDIR.."/RelSymbols")
        
			--check that the source subdirectory for the project exists
			--create it otherwise
			subdir = os.matchdirs(SOURCEDIR.."/"..val)
			if (#subdir == 0) then
				os.mkdir(SOURCEDIR.."/"..val)
			end
	end
	

--clean up generated directories
if _ACTION == "clean" then
	os.rmdir("projects")
	os.rmdir("bin")
	os.rmdir("obj")
end
