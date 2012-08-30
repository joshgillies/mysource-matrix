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
* $Id: system_integrity_check_links.php,v 1.7 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Go through all WYSIWYG content types and ensure all ./?a=xx links are valid
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

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

// login as root user to avoid problems with safe edit assets
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed login in as root user\n";
	exit();
}

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but it may take a long time\n\nYOU HAVE 5 SECONDS TO CANCEL THIS SCRIPT... ";
	for ($i = 1; $i <= 5; $i++) {
		sleep(1);
		echo $i.' ';
	}
	echo "\n\n";
}


// go trough each wysiwyg in the system and validate the links
$wysiwygids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'content_type_wysiwyg', false);
foreach ($wysiwygids as $wysiwygid => $type_code_data) {
	$type_code = $type_code_data[0]['type_code'];
	$wysiwyg = &$GLOBALS['SQ_SYSTEM']->am->getAsset($wysiwygid, $type_code);
	$html = $wysiwyg->attr('html');

	// extract the ./?a=xx style links
	$e = '/\\.\\/\\?a=([0-9]+)/';
	$matches = Array();
	preg_match_all($e, $html, $matches);
	$internal_assetids = $matches[1];

	foreach ($internal_assetids as $assetid) {
		printWYSIWYGName('WYSIWYG #'.$wysiwyg->id.' - LINK #'.$assetid);
		if(!empty($assetid)) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
		}
		else {
			$asset = NULL;
		}
		
		if (is_null($asset)) {
			// the asset was invalid
			printUpdateStatus('INVALID');
		} else if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid, true)) {
			// the asset is in the trash
			printUpdateStatus('TRASH');
		} else {
			printUpdateStatus('OK');
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
	}

	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);

}//end foreach


/**
* Prints the name of the WYSIWYG Link as a padded string
*
* Padds to 35 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printWYSIWYGName($name)
{
	printf ('%s%'.(35 - strlen($name)).'s', $name, '');

}//end printWYSIWYGName()


/**
* Prints the status of the link integrity check
*
* @param string	status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
