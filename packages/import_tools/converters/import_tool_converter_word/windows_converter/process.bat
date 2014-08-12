:: +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
:: | ACN 084 670 600                                                    |
:: +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
:: +--------------------------------------------------------------------+

@IF "%1" == "" GOTO NO_FILE
@IF "%2" == "" GOTO NO_FILE_2

@IF "%1" == "." GOTO GRACEFUL
@IF "%1" == ".." GOTO GRACEFUL

@ECHO Processing file "%1"

@CALL "doc2html.exe" "%1" /d:%2 /l:c:\scripts\log.log

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

