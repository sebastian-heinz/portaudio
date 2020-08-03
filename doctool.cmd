mkdir doc_tmp
cd ../../
call scons -j6 platform=windows vsproj=yes
cd bin
godot.windows.tools.64.exe --doctool "./../modules/portaudio/doc_tmp"
rmdir "./../modules/portaudio/doc_classes" /q /s 
xcopy "./../modules/portaudio/doc_tmp/modules/portaudio/doc_classes" "./../modules/portaudio/doc_classes" /e /i /h
rmdir "./../modules/portaudio/doc_tmp" /q /s 
cmd