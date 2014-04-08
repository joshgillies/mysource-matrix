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
* $Id: delete_assets_by_id.php,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Pass in any assetid's you want to move to the trash folder.
* It does not purge the trash (that is still a separate script).
*
* @author  Matt Keehan <mkeehan@squiz.co.uk>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

if (empty($argv[1])) {
	echo "Supply some assetid's to move to the trash.\n";
	echo "This only moves the assets to the trash, to really delete them, you still need to use the purge_trash.php script.\n\n";
	echo "Usage: " . __FILE__ . " id1 id2 id3\n";
	exit;
}

require dirname(__FILE__) . '/../core/include/init.inc';

// set up the root user
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed login in as root user\n";
	exit();
}

$trash_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('trash_folder');

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

$am = new Asset_Manager();

$bad_ids = array();

# argv[0] is the script name so take one off to see how many assets we're going to delete.
$asset_count = count($argv) - 1;
echo "Going to delete $asset_count assets .. \n";
for ($i = 1; $i <= $asset_count; $i++) {
	if ($i % 5 == 0) {
		echo "Have deleted $i / $asset_count assets so far .. \r";
	}
	$assetid = $argv[$i];

	$link = $am->getLink($assetid, NULL, '', TRUE, NULL, 'minor');
	if (empty($link)) {
		$bad_ids[] = $assetid;
		continue;
	}
	$am->moveLink($link['linkid'], $trash_folder->id, $link['link_type'], 0);
}

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

echo "\nTrashing complete.\n";
if (!empty($bad_ids)) {
	echo "I was unable to delete the following assetids:\n";
	echo str_repeat('-', 5) . "\n";
	echo implode($bad_ids, "\n") . "\n";
	echo str_repeat('-', 5) . "\n";
}

echo "Please run purge_trash now to really delete them.\n";

