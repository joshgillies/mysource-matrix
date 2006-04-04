@IF "%1" == "" GOTO END
@IF "%2" == "" GOTO END

mkdir "%1"intermediate

@FOR %%Z IN ("%1"*.doc) DO CALL process.bat %%Z "%1"intermediate\
@FOR %%Z IN ("%1"*.dot)  DO CALL process.bat %%Z "%1"intermediate\
@FOR %%Z IN ("%1"intermediate\*.htm) DO CALL filter.bat %%Z
@CALL COPY "%1"*.doc %2
@CALL COPY "%1"*.dot %2
XCOPY "%1"intermediate %2 /S /E /-Y
@CALL RMDIR "%1"intermediate\ /q /s

:END
