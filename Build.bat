Set vs_version=2022    :: 2013 or 2015 or 2017 or 2019

cd framework\                
goto case_%vs_version%      
:case_2013
  "%VS120COMNTOOLS%..\IDE\devenv" CS380.sln /build Release
  goto end_case
:case_2015
  "%VS140COMNTOOLS%..\IDE\devenv" CS380.sln /build Release
  goto end_case
:case_2017
  "%VS150COMNTOOLS%..\IDE\devenv" CS380.sln /build Release
  goto end_case
:case_2019
  if defined VS190COMNTOOLS call "%VS190COMNTOOLS%vsdevcmd.bat"
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv" CS380.sln /build Release
  goto end_case
:case_2022
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
  msbuild CS380.sln -verbosity:quiet -t:build /p:Configuration=Release /p:PlatformTarget=x86
  goto end_case
:end_case
  Copy x64\Release\CS380.exe .
  cd ..
  pause
