<?

  ########################################
 # Set the level of PHP reported errors
# And some other PHP thingies we want
# done OUR way.
error_reporting(E_ALL);
set_magic_quotes_runtime(0);

 ################################################
# Set the current version of the system core
# We mustn't forget to update this (until someone
# comes up with an automated method)
define('SQ_RESOLVE_VERSION',   '0.1.0');
define('SQ_RESOLVE_LONG_NAME', 'Squiz Resolve v'.SQ_RESOLVE_VERSION);
define('SQ_RESOLVE_URL',       'http://www.squiz.net/');

define('SQ_REQUIRED_PHP_VERSION',   '4.3.0');
//define('SQ_REQUIRED_MYSQL_VERSION', '3.23');

//define('SQ_ERROR_CODE_NONE',    0);
//define('SQ_ERROR_CODE_WARNING', 1);
//define('SQ_ERROR_CODE_ERROR',   2);

 ############################################################
# Let's het our bearings as to where everything is from here.
# These paths may be relative or absolute

define('SQ_SYSTEM_ROOT',       dirname(dirname(dirname(realpath(__FILE__)))));
define('SQ_INCLUDE_PATH',      SQ_SYSTEM_ROOT.'/core/include');
define('SQ_CORE_PACKAGE_PATH', SQ_SYSTEM_ROOT.'/core/assets');
define('SQ_LIB_PATH',          SQ_SYSTEM_ROOT.'/core/lib');
define('SQ_DATA_PATH',         SQ_SYSTEM_ROOT.'/data');
define('SQ_PACKAGES_PATH',     SQ_SYSTEM_ROOT.'/packages');

define('SQ_WEB_PATH',      SQ_SYSTEM_ROOT.'/core/web');

//define('SQ_CACHE_PATH',    SQ_SYSTEM_ROOT.'/cache');


# Load general functions, including the error_reporter
require_once SQ_INCLUDE_PATH.'/general.inc';
ini_set('display_errors', 1); # we'll manage whether to display the errors or not in the error_handler
set_error_handler('sq_error_handler');

# include the main system config file
require_once SQ_DATA_PATH.'/conf/main.inc';

 #############################################
# Now that we've got error reporting lets 
# generate a few errors!
if(version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_RESOLVE_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br> You may need to upgrade.<br> Your current version is '.phpversion().'.', E_USER_ERROR);
}

if(!function_exists('mysql')) {
	trigger_error(SQ_RESOLVE_LONG_NAME.' requires PHP to be installed with MySQL support. You may have to recompile PHP.', E_USER_ERROR);
}

 #########################################
# Load general everyday handy functions for developers
require_once SQ_LIB_PATH.'/dev/dev.inc';
#speed_check(); mem_check();
//require_once SQ_LIB_PATH.'/general/general.inc';

# nice little boolean to use for testing whether we happen to be in the backend or not
define('SQ_IN_BACKEND', (basename($PHP_SELF) == SQ_CONF_BACKEND_SUFFIX));

define('SQ_WWW_BASE_DIR', ((SQ_IN_BACKEND) ? '..' : '.'));
define('SQ_WWW_EDIT_DIR', ((SQ_IN_BACKEND) ? '.'  : './'.SQ_CONF_BACKEND_SUFFIX));
define('SQ_WWW_LIB_DIR',  SQ_WWW_BASE_DIR.'/__lib');


 ############################################
# Link Types - used with bitwise operators
define('SQ_LINK_EXCLUSIVE', 1);
define('SQ_LINK_UNITE',     2);
define('SQ_LINK_NOTICE',    4);
define('SQ_LINK_ANY',       7); # what all the others add up to

//pre_echo(get_defined_constants());

 ##################################
# Create the system object
require_once SQ_INCLUDE_PATH.'/resolve.inc';
$GLOBALS['SQ_RESOLVE'] = new Resolve();

?>