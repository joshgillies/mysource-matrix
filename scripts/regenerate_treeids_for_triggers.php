<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: regenerate_treeids_for_triggers.php,v 1.1.2.2 2009/08/27 01:45:33 akarelia Exp $
*
*/

/**
* Bug fix #3864  	Rebuilding Link Tree breaks triggers
* This script will get all the triggers installed int he system and regenerate them
* this will re-enter values in table, thus fxing up any issue with inconsistent tree_ids
*
* @author  Ashish Karelia <akarelia@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if (php_sapi_name() != 'cli') {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

error_reporting(E_ALL);
ini_set('memory_limit', '-1');

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

//--        MAIN()        --//


$script_start = time();

echo_headline(' GETTING ALL THE TRIGGERS INSTALLED ON THE SYSTEM');

// get trigger manager and all the triggers installed on the system
$tm =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager'); 	 
$trigger_list = MatrixDAL::executeAll('core', 'getTriggerList');

foreach ($trigger_list as $index => $trigger_data) {
	echo_headline( " REGENERATING TRIGGER ".$tm->id.":".$trigger_data['id']);
	// load the trigger and regenerate it, same as clicking commit on the backend :)
	$trigger = $tm->_loadTrigger($trigger_data['id']); 	 
    $result = $tm->_saveTrigger($trigger);
	
	if (!$result) echo_headline(' ERROR OCCURED WHILE TRYING TO SAVE TRIGGER '.$tm->id.':'.$trigger_data['id']);
}



fwrite(STDERR, "\n");

echo_headline(' TREE ENTRIES CREATED');

$script_end = time();
$script_duration = $script_end - $script_start;
echo '-- Script Start : ', $script_start, '    Script End : ', $script_end, "\n";
echo '-- Script Duration: '.floor($script_duration / 60).' mins  '.($script_duration % 60)." seconds\n";
fwrite(STDERR, '-- Script Duration: '.floor($script_duration / 60).' mins  '.($script_duration % 60)." seconds\n");


//--        FUNCTIONS        --//


/**
* Print a headline to STDERR
*
* @param string		$s	the headline
*
* @return void
* @access public
*/
function echo_headline($s , $echo_time = FALSE)
{
	static $start = 0;

	if ($start && $echo_time) {
		$end = time();
		$duration = $end - $start;
		fwrite(STDERR, '-- Duration: '.floor($duration / 60).'mins '.($duration % 60)."seconds\n");
	}

	fwrite(STDERR, "--------------------------------------\n$s\n--------------------------------------\n");

	$start = time();

}//end echo_headline()


?>
