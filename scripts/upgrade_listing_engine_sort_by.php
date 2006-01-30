<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_listing_engine_sort_by.php,v 1.1.2.1 2006/01/30 03:05:33 sdanis Exp $
*
*/

/**
* Script to upgrade the sort_by attribute from 3.4 to 3.6 of all listing_engine assets
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$db =& $GLOBALS['SQ_SYSTEM']->db;
$am =& $GLOBALS['SQ_SYSTEM']->am;

// lets get all the listing engine assetids

$page_asset_listings = $am->getTypeAssetids('listing_engine', FALSE);

$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// for each page asset listing:
// - get its old sort_by value
// - get the new default value
// - set the sort_by value to default_value
// - set the default_sort_by to old sort_by value (actualy the index of it in default_value)
foreach ($page_asset_listings as $assetid) {
	$upg_status = 'FAILED';

	$asset =& $am->getAsset($assetid);
	echo 'Upgrading Asset (#'.$assetid.') ';
	printName($asset->name);

	if ($asset->attr('default_sort_by') == 0) {
		// we need a better check for this
		$sql = 'SELECT custom_val, default_val FROM sq_ast_attr_val as s LEFT OUTER JOIN sq_ast_attr as a ON s.attrid=a.attrid WHERE s.assetid='.$db->quoteSmart($assetid).' AND a.name='.$db->quoteSmart('sort_by').'';
		$result = $db->getAll($sql);

		if (empty($result)) {
			// no sorting? try to get the default sort_by from asset
			$custom_val = '';
			$unserialized_def_val = $asset->attr('sort_by');
		} else {
			// new default value which is serialised and not selection
			$unserialized_def_val = unserialize($result[0]['default_val']);
			// custom value (i.e created, name, last_updated, etc.)
			$custom_val = $result[0]['custom_val'];
		}

		if (!empty($unserialized_def_val)) {
			if ($unserialized_def_val != FALSE) {
				$new_index = -1;

				// find the index of the old value in new default_val array
				if (is_array($unserialized_def_val)) {
					foreach ($unserialized_def_val as $index => $value) {
						if ($value['params']['field'] == $custom_val) {
							$new_index = $index;
							break;
						}
					}

					if ($new_index == -1) {
						// search pages have empty value for relevance
						if (get_class($asset) == 'search_page' && $custom_val == '') {
							$custom_val = '__relevance__';
						}
						// it could be in a custom sort_by
						$edit_fns = $asset->getEditFns();
						$sort_by_opts = $edit_fns->_getSortByOptions($asset);


						if (array_key_exists($custom_val, $sort_by_opts)) {
							// its a custom sort_by add it to default array
							$new_option = Array(
											'params'	=> Array(
															'field'	=> $custom_val,
														   ),
											'name'		=> $sort_by_opts[$custom_val],
											'type'		=> 'field',
										  );
							$unserialized_def_val[] = $new_option;
							$new_index = count($unserialized_def_val)-1;
						} else if ($custom_val == '') {
							$new_index = 0;	// must be the first index
						} else {
							echo 'Warning: Could not assign new index! Default sort by value assigned. (Old value: '.$custom_val;
							$upg_status = '**';
							$new_index = 0;
						}

					}
				}//end if (is_array)

				// update values
				$asset->setAttrValue('sort_by', serialize($unserialized_def_val));
				$asset->setAttrValue('default_sort_by', $new_index);
				if (!$asset->saveAttributes()) {
					echo 'Warning: Could not save asset attributes!';
				} else if ($upg_status!='**') {
					$upg_status = 'OK';
				}
			} else {
				echo 'Could not find default value.';
			}
		} else {
			echo 'Could not find default value.';
		}
	} else {
		// must have been upgraded already, skip
		$upg_status = 'SKIPPED';
	}

	printUpdateStatus($upg_status)."\n";

	$am->forgetAsset($asset);
	unset($asset);
}//end foreach

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

exit();


/**
* Print progress
*
* @param string	$name	Assets name
*
* @return void
* @access public
*/
function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


/**
* Print upgrade status
*
* @param string	$status	Upgrade status of the asset
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
