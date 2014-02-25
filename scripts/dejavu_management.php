<?php
/**
* Enables, disables, forgets, prints the status of the dejavu memory management.
*
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 's:';
$longopt = Array('enable', 'disable', 'forget', 'status', 'disable_force', 'recaching_delay=', 'cache_storage=');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);
if ($options instanceof PEAR_Error) {
	usage();
}

if (empty($options[0])) usage();

$SYSTEM_ROOT = '';
$ACTION = '';
$RECACHING_DELAY = '0';
$CACHE_STORAGE = '';

foreach ($options[0] as $option) {
	switch ($option[0]) {
		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$SYSTEM_ROOT = $option[1];
		break;
		
		default:
			$ACTION = $option[0];
			if ($ACTION == '--recaching_delay') {
				if (empty($option[1]) || !is_numeric($option[1])) {
					usage();
				}
				$RECACHING_DELAY = $option[1];
				continue;
			}

			// Validation is done in Deja-Vu to make sure it's a valid type.
			if ($ACTION == '--cache_storage') {
				$CACHE_STORAGE = $option[1];
				continue;
			}
		break;
	}
}

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
}

if($ACTION === '--disable_force') {
	// forcibly disable deja vu before even include init.inc
	file_put_contents($SYSTEM_ROOT.'/data/private/conf/.dejavu', '0');
	echo "Deja Vu is forcibly disabled.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once $SYSTEM_ROOT.'/core/include/deja_vu.inc';

$deja_vu = new Deja_Vu();
switch ($ACTION) {
	case '--status':
		if ($deja_vu->enabled() == FALSE) {
			echo "Deja Vu is currently disabled.\n";
		} else {
			echo "Deja Vu is currently enabled.\n";
			echo "Recaching delay time: ".$deja_vu->getRecachingDelay()."s.\n";
			echo "Cache storage: ".$deja_vu->getCacheStorage().".\n";
		}
		break;
	case '--enable':
		if ($deja_vu->enabled() == TRUE) {
			echo "Deja Vu is already enabled.\n";
        } else {
            $ok = $deja_vu->checkRequirements();

			echo "Enabling Deja Vu...\n";
			if ($deja_vu->enable()) {
				$d_vu = new Deja_Vu();
				if ($d_vu) {
					echo "Forgetting everything previously remembered...\n";
					if ($d_vu->forgetAll()) {
						echo "[DONE]\n";
						break;
					}
				} else{
					echo "what?\n";
				}
			}
			echo "[FAILED]\n";
		}
		break;
	case '--disable':
		if ($deja_vu->enabled() == FALSE) {
			echo "Deja Vu is already disabled.\n";
		} else {
			echo "Disabling Deja Vu...\n";
			if ($deja_vu->disable()) {
				echo "[DONE]\n";
			} else {
				echo "[FAILED]\n";
			}
		}
		break;
	case '--forget':
		if ($deja_vu->enabled() == FALSE) {
			echo "Deja Vu is currently disabled.\n";
		} else {
			echo "Forgetting everything in Deja Vu...\n";
			if ($deja_vu->forgetAll()) {
				echo "[DONE]\n";
			} else {
				echo "[FAILED]\n";
			}
		}
		break;
	case '--recaching_delay':
		echo "Setting recaching delay time period ...\n";
		if ($deja_vu->setRecachingDelay($RECACHING_DELAY)) {
			echo "[DONE]\n";
		} else {
			echo "[FAILED]\n";
		}
		break;
	case '--cache_storage':
		echo "Setting cache storage type ...\n";
		if ($deja_vu->setCacheStorage($CACHE_STORAGE)) {
			echo "[DONE]\n";
		} else {
			echo "[FAILED]\n";
		}
		break;
	default:
		usage();
		break;
}


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "\nUSAGE: dejavu_management.php -s <system_root> [--enable] [--disable] [--forget] [--recache_delay <time_in_seconds>] [--cache_storage=memcache|redis]\n".
		"--enable              Enables Deja Vu in MySource Matrix\n".
		"--disable             Disables Deja Vu in MySource Matrix\n".
		"--disable_force       Forcibly disables Deja Vu by editing control file\n".
		"--forget              Forgets all Deja Vu data in MySource Matrix\n".
		"--status              Checks the current Deja Vu status\n".
		"--recaching_delay     Set the time period to delay the asset recaching after the asset has been updated\n".
		"--cache_storage       Set the cache storage to either memcache or redis\n";
		"\nNOTE: only one of [--enable --disable --forget] option is allowed to be specified\n";
	exit();

}//end usage()
?>
