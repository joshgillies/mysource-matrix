@REM This will be performed before processing
@REM Use this if you cannot directly access the UNIX server from the current file system
@REM pscp uname:pass@myserver.com/import_dir/word_uploads INPUT_DIR
@REM **Pre-processing

@REM These must be reachable with drive letters.
@REM Use the commands above and below to retrieve files if they are located
@REM on a server where scp or something similar must be used
@REM Don't forget your trailing slashes

@REM ***************************************************
@REM This is the main process call. This processes all word files to html files
@REM It should look like:
@REM call dir_process.bat INPUT_DIR OUTPUT_DIR

CALL dir_process.bat INPUT_DIR OUTPUT_DIR

@REM ***************************************************
@REM **Post-processing

@REM This will be performed after processing
@REM pscp OUTPUT_DIR uname:pass@myserver.com/import_dir
