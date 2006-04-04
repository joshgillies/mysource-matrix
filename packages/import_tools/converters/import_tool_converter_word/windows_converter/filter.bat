@IF "%1" == "" GOTO NO_FILE

@IF "%1" == "." GOTO GRACEFUL
@IF "%1" == ".." GOTO GRACEFUL

@ECHO Filtering file "%1"

@CALL "filter.exe" -a -b -f -l -m -t -r "%1"

@GOTO SUCCESS

:NO_FILE
@ECHO No Input File Name Specified
@GOTO END

:NO_FILE_2
@ECHO No Output Directory Specified
@GOTO END

:SUCCESS
@ECHO "%1" successfully Converted
@ECHO FILE COMPLETED
@GOTO END

:GRACEFUL
@ECHO "%1" -> Directory or parent?
@GOTO END

:END

