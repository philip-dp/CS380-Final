cd framework
msbuild CS380.sln -t:Clean
DEL /F /S /Q /AH "*.suo"
DEL /F /S /Q "*.sdf"
DEL /F /S /Q "*.opensdf"
DEL /F /S /Q "*.sdf"
DEL /F /S /Q "*.db"
RMDIR /S /Q "x64"
RMDIR /S /Q "ipch"
RMDIR /S /Q ".vs"
RMDIR /S /Q ".\Output\"
cd ..
