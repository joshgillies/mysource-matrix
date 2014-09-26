<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*/

/**
* After #6059 bug fix, the form actions 'rest action' and 'soap action'
* with validation enabled are supposed to execute before the submission is created
*
* @author Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/

// Usage: php upgrade_rest_and_soap_form_actions.php <SYSTEM_ROOT> [REPORT_ONLY]
// [REPORT_ONLY] = {y, n}

if (ini_get('memory_limit') != '-1') ini_set('memory_limit', -1);
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
$report_only = (isset($_SERVER['argv'][2]) && $_SERVER['argv'][2] == 'y') ? FALSE : TRUE;

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

if ($report_only) {
	echo "'REST call' and 'SOAP call' actions in following forms needs upgrading:\n";
} else {
	echo "Upgrading 'REST call' and 'SOAP call' actions \n";
}

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
        echo "ERROR: Failed logging in as root user\n";
        exit();
}

// Get all the bodycopy divs
$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren(1, 'form_email'));
$count = 0;
foreach($assetids as $assetid) {
	$form = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$actions = $form->attr('actions');
	$updated = FALSE;
	foreach($actions as &$action) {
		if ($action['type_code'] == 'form_action_call_rest_resource' || $action['type_code'] == 'form_action_soap_call') {
			if (isset($action['settings']['before_submit'])) {
				unset($action['settings']['before_submit']);
				$action['settings']['before_create'] = TRUE;
				$updated = TRUE;
			}
		}
	}//end foreach
	if ($updated) {
		$count++;
		if (!$report_only) {
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$form->setAttrValue('actions', $actions);
				$form->saveAttributes(TRUE);
			 $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			 echo ".";
		} else {
			echo $assetid."\n";
		}
	}
}//end foreach

echo $report_only ? $count. " asset(s) requires upgrading" : $count. " asset(s) were upgraded";
echo "\n";
?>
