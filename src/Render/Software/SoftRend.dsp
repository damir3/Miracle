# Microsoft Developer Studio Project File - Name="SoftRend" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SoftRend - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoftRend.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoftRend.mak" CFG="SoftRend - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SoftRend - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SoftRend - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SoftRend - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /out:"../../Miracle/SoftRend.dll"

!ELSEIF  "$(CFG)" == "SoftRend - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Debug/SoftRend.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SoftRend - Win32 Release"
# Name "SoftRend - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Draw_2D.cpp
# End Source File
# Begin Source File

SOURCE=.\Draw_3D.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawEnt.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawFace.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawModel.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawSpan.cpp
# End Source File
# Begin Source File

SOURCE=.\Math_3D.cpp
# End Source File
# Begin Source File

SOURCE=.\Portal.cpp
# End Source File
# Begin Source File

SOURCE=.\Render.cpp
# End Source File
# Begin Source File

SOURCE=.\SpanBuf.cpp
# End Source File
# Begin Source File

SOURCE=.\Surface.cpp
# End Source File
# Begin Source File

SOURCE=.\TexGrad.cpp
# End Source File
# Begin Source File

SOURCE=.\Video.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Const.h
# End Source File
# Begin Source File

SOURCE=.\Draw_2D.h
# End Source File
# Begin Source File

SOURCE=.\Draw_3D.h
# End Source File
# Begin Source File

SOURCE=.\DrawEnt.h
# End Source File
# Begin Source File

SOURCE=.\DrawFace.h
# End Source File
# Begin Source File

SOURCE=.\DrawModel.h
# End Source File
# Begin Source File

SOURCE=.\DrawSpan.h
# End Source File
# Begin Source File

SOURCE=.\MapFile.h
# End Source File
# Begin Source File

SOURCE=.\Math_3D.h
# End Source File
# Begin Source File

SOURCE=.\Portal.h
# End Source File
# Begin Source File

SOURCE=.\Render.h
# End Source File
# Begin Source File

SOURCE=.\SpanBuf.h
# End Source File
# Begin Source File

SOURCE=.\Struct.h
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# Begin Source File

SOURCE=.\TexGrad.h
# End Source File
# Begin Source File

SOURCE=.\Video.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\SoftRend.def
# End Source File
# End Target
# End Project
