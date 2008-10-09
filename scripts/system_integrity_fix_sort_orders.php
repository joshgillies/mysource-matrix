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
* $Id: system_integrity_fix_sort_orders.php,v 1.1.2.3 2008/10/09 23:45:17 bshkara Exp $
*
*/

/**
* Ensure the sort_order in the sq_ast_lnk table is linear.
* Takes into consideration the existing sort_order.
* You may specify a parent node to start from.  If omitted, the process will start from the root folder.
* Note: This will not sort shadow asset links.
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1.2.3 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
ini_set('memory_limit', '1024M');
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$system_root = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($system_root) || !is_dir($system_root)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $system_root.'/core/include/init.inc';

// check the number of arguments
if (count($argv) !== 3) {
	echo 'Usage: system_integrity_fix_sort_orders.php <SYSTEM_ROOT> <PARENT ASSET ID>'."\n";
	die;
}

$parentid = $_SERVER['argv'][2];
$parent =& $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
if (is_null($parent)) {
	trigger_error("ERROR: Unable to retrieve that asset.\n");
	exit();
}//end if

echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

// connect to the DB
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

$todo = Array($parentid);
$done = Array();

echo "\n".'---BEGIN---'."\n";

$success = sortAssets($todo, $done);

echo "\n".'---COMPLETED---'."\n";

// restore the Database connection
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


/**
* Ensure the sort order is linear and consistent taking into account the existing sort_order
* Begins from the provided root node and cleans all branches stemming from the provided root node
* Note: This is based on Tom's Tool_Asset_Sorter - the difference: Tom's tool is not based on existing sort_order and does not recurse
*
* @param array	$todo	Parents to sort
* @param array	$done	Parents done sorting
*
* @return boolean
* @access public
*/
function sortAssets($todo, $done)
{
	if (!empty($todo)) {

		$parentid = array_shift($todo);

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// order by existing sort_order
		// only concerned with TYPE_1 and TYPE_2
		// retrieve minorids as well because we need them for the recursive behaviour implemented towards the end of this routine
		$sql = 'SELECT linkid, minorid
				FROM sq_ast_lnk
				WHERE majorid = '.$db->quote($parentid).'
					AND link_type IN ('.$db->quote(SQ_LINK_TYPE_1).', '.$db->quote(SQ_LINK_TYPE_2).')
				ORDER BY sort_order ASC';
		$results = $db->getAssoc($sql);
		assert_valid_db_result($results);

		echo "\n".'- Updating the sort order for kids of: #'.$parentid.'...';

		// separate results
		$childids = $linkids = Array();
		foreach ($results as $linkid => $minorid) {
			// linkids used to update the sort_order
			$linkids[] = $linkid;
			// childids used to look for more parents
			$childids[] = $minorid;
		}

		if (!empty($linkids)) {
			// there is a limit to CASE statement size in Oracle, that limits it to
			// 127 WHEN-THEN pairs (in theory), so limit to 127 at a time on Oracle
			$chunk_size = ($db->phptype == 'oci8') ? 127 : 500;

			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			foreach (array_chunk($linkids, $chunk_size, TRUE) as $chunk) {
				$cases = '';
				foreach ($chunk as $i => $linkid) {
					$cases .= 'WHEN (linkid = '.$linkid.') THEN '.$i.' ';
				}
				$sql = 'UPDATE sq_ast_lnk
						SET sort_order = CASE '.$cases.' ELSE sort_order END
						WHERE linkid IN ('.implode(', ', $chunk).')';
				$res = $db->query($sql);
				assert_valid_db_result($res);
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}

		// ensure we do not update this parent again
		if (!in_array($parentid, $done)) {
			$done[] = $parentid;
		}

		echo ' [done]';

		// check each child of the parent to see if the parent is a grandparent (i.e. parent's children have children)
		// only examining 1 level deep at a time
		if (!empty($childids)) {
			echo "\n\t".'- Searching immediate children of: #'.$parentid.' for branches';
			foreach ($childids as $assetid) {
				// these are the kids that we have already sorted
				// check to see if they are parents as well
				// shadow asset links are ignored
				$sql = 'SELECT minorid
						FROM sq_ast_lnk
						WHERE majorid = '.$db->quote($assetid);
				$children = $db->getCol($sql);
				assert_valid_db_result($children);

				if ((!empty($children)) && count($children) > 1) {
					// we have a potential new parent
					// 1. check that we have not processed it yet
					if (!in_array($assetid, $done)) {
						// 2. check that the returned children contain at least one TYPE 1 or 2 linked asset
						// e.g. asset could just be tagged with a thesaurus term (shadow link), meaning it is not a valid parent
						$valid = FALSE;
						foreach ($children as $grandchild) {
							$link = $GLOBALS['SQ_SYSTEM']->am->getLink($grandchild, NULL, '', TRUE, NULL, 'minor');
							if (!empty($link) && (($link['link_type'] == SQ_LINK_TYPE_1) || ($link['link_type'] == SQ_LINK_TYPE_2))) {
								$valid = TRUE;
								break;
							}
						}

						if ($valid) {
							echo "\n\t\t#".$assetid.' is a parent with kids that will be sorted';
							$todo[] = $assetid;
						}
					}
				}
			}
		}

		echo "\n".'* '.count($todo).' items left to process'."\n";
		echo '* Using '.round((memory_get_usage()/1048576), 2).' MB'."\n";

		sortAssets($todo, $done);

	} else {
		// there are no more items to process
		return TRUE;
	}
}


?>
