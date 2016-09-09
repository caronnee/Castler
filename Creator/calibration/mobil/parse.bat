set filename=%~n1
mkdir frames\%filename%

c:\Work\DP1\ffmpeg\ffmpeg\bin\ffmpeg.exe -i %1 -r 2 -an frames/%filename%/%%4d.jpg


