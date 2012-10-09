<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_future_trigger_actions.php,v 1.3.2.1 2012/10/09 01:04:43 akarelia Exp $
*
*/

/**
*
* @author Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "ERROR: Failed logging in as root user\n";
	exit();
}

$am = $GLOBALS['SQ_SYSTEM']->am;
$count = 0;

// Upgrade each Trigger where Trigger Set Future Permissions or Status is used
echo 'Upgrading Future Trigger Actions...';

// Grab each Trigger and its settings
$tm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trigger_manager');
$triggers = $tm->getTriggerList();

foreach ($triggers as $trigger_id => $trigger) {
	$trigger_asset = $am->getAsset($tm->id.':'.$trigger['id']);
	upgradeTrigger($trigger_asset);
}

echo "\n- Done\n";


/**
* Upgrade the Set Future Permissions and Set Future Status trigger actions
* present in existing Trigger assets to the new values expected in version 0.2
* of these assets
*
* @param Trigger	&$trigger	The Trigger Asset to upgrade
*
* @return void
* @access public
*/
function upgradeTrigger(Trigger &$trigger)
{
	$trigger_actions = $trigger->attr('actions');
	$trigger_modified = FALSE;

	foreach ($trigger_actions as $key => &$trigger_action) {
		$trigger_action_data =& $trigger_action['data'];

		// Set Future Permissions and Status triggers < v0.2 will not have an "offset_used" value
		if ((($trigger_action['type'] == 'trigger_action_set_future_permissions') ||
			($trigger_action['type'] == 'trigger_action_set_future_status')) &&
			(!isset($trigger_action_data['offset_used']))) {

			// Modify data associated with the Trigger Action (using a reference for ease and fun)

			// Add new "offset_used" value
			$trigger_action_data['offset_used'] = FALSE;

			// Convert to new "by_attr_value" when_type and enable offset if one was specified
			if (($trigger_action_data['when_type'] == 'attr_interval') || ($trigger_action_data['when_type'] == 'attr_exact')) {
				$trigger_action_data['when_type'] = 'by_attr_value';
				$trigger_action_data['offset_used'] = ($trigger_action_data['when_type'] == 'attr_interval');
			}

			// Convert "explicit_interval" to "explicit_exact", as the offset is now handled separately
			if ($trigger_action_data['when_type'] == 'explicit_interval') {
				$trigger_action_data['when_type'] = 'explicit_exact';
				$trigger_action_data['offset_used'] = TRUE;
			}

			// Restock the main Trigger Actions array for this Trigger
			$trigger_actions[$key] = $trigger_action;
			$trigger_modified = TRUE;
		}
	}

	if ($trigger_modified) {
		// Supply the new Trigger Actions values and save the Trigger if it was modified
		echo "\n- Upgrading Trigger ".$trigger->id.'... ';

		// Ok we need the Attributes lock now...
		$GLOBALS['SQ_SYSTEM']->acquireLock($trigger->id, 'attributes');

			$trigger->setAttrValue('actions', $trigger_actions);
			$trigger->saveAttributes();

		$GLOBALS['SQ_SYSTEM']->releaseLock($trigger->id, 'attributes');

		echo 'done';
	}

}//end upgradeTrigger()


?>
