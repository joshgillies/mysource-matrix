:: +--------------------------------------------------------------------+
:: | This MySource Matrix Module file is Copyright © Squiz Pty Ltd      |
:: | ACN 084 670 600                                                    |
:: +--------------------------------------------------------------------+
:: | IMPORTANT: This Module is not available under an open source       |
:: | license and consequently distribution of this and any other files  |
:: | that comprise this Module is prohibited. You may only use this     |
:: | Module if you have the written consent of Squiz.                   |
:: +--------------------------------------------------------------------+

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
@CALL DEL "%1"*.doc /Q
@CALL DEL "%1"*.dot /Q

:END
