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
* $Id: system_integrity_fix_sort_orders.php,v 1.2 2008/08/22 06:24:44 bpearson Exp $
*
*/

/**
* This script resorts the sort order of the SQ_AST_LNK table
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$system_root = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($system_root) || !is_dir($system_root)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $system_root.'/core/include/init.inc';

// If none set, use the default root
$rootnodeid = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
$rootnode = $GLOBALS['SQ_SYSTEM']->am->getAsset($rootnodeid);
if (is_null($rootnode)) {
	trigger_error("ERROR: Unable to load that root node\n");
	exit();
}//end if

// Check for confirmation on doing the whole system ... it is going to take a bloody long time
if ($rootnodeid == '1') {
	$confirmation = NULL;
	while ($confirmation != 'y' && $confirmation != 'n') {
		echo "You have opted for running this script across the whole system. This may take a long time.\n";
		echo 'Are you sure you want to continue? (Y/N) ';
		$confirmation = rtrim(fgets(STDIN, 4094));
		$confirmation = strtolower($confirmation);
		if ($confirmation != 'y' && $confirmation != 'n') {
			echo "Please answer Y or N\n";
		} else if ($confirmation == 'n') {
			exit();
		}//end else
	}//end while
}//end if

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

// Connect to the DB
$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$db =& $GLOBALS['SQ_SYSTEM']->db;

///////////////////////////// START RESORTING ///////////////////////////////////////////
// Start message
echo 'Starting the re-sorting of "'.SQ_CONF_SYSTEM_NAME."\"\n";
echo "Please wait...\n";

$errors = Array();
$stats = Array();

$return_status = resort($rootnode->id, $errors, $stats);

// End message
echo 'Finished the re-sorting of "'.SQ_CONF_SYSTEM_NAME."\"\n";
echo count($stats)." Assets haved been processed\n";
if (!$return_status) {
	echo count($errors)." Errors occured:\n";
	if (empty($errors)) {
		echo "No errors reported\n";
	} else {
		foreach ($errors as $error) {
			echo 'ERROR: '.$error."\n";
		}//end foreach
	}//end else
}//end if
//////////////////////////// FINISH RESORTING //////////////////////////////////////////

// Restore the Database connection
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

// Finish


/**
* This function resorts the sort of the children under the root node supplied
*
* @param string	$node		The root node assetid to resort
* @param array	&$errors	The error message holding array (by reference)
* @param array	&$stats		The stats holding array (by reference)
*
* @return boolean
* @access public
*/
function resort($node, &$errors, &$stats)
{
	// Globalise
	global $db;

	// TRUE by default until something goes wrong
	$return_value = TRUE;

	// We need each link type done separately
	$link_types = Array(SQ_LINK_TYPE_1, SQ_LINK_TYPE_2, SQ_LINK_TYPE_3, SQ_LINK_NOTICE);

	$children = get_children($node, $errors);

	// If a valid asset do the sorting
	$is_asset = $GLOBALS['SQ_SYSTEM']->am->assetExists($node);
	if ($is_asset) {
		foreach ($link_types as $link_type) {
			$links_sql = 'SELECT linkid FROM sq_ast_lnk WHERE majorid = :majorid AND link_type = :link_type ORDER BY sort_order';
			$links_query = MatrixDAL::preparePdoQuery($links_sql);
			MatrixDAL::bindValueToPdo($links_query, 'majorid', $node);
			MatrixDAL::bindValueToPdo($links_query, 'link_type', $link_type);
			$links_result = MatrixDAL::executePdoAssoc($links_query);

			foreach ($links_result as $i => $linkid) {
				if (isset($linkid['linkid'])) {
					$link = $linkid['linkid'];
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					try {
						$sql = 'UPDATE sq_ast_lnk SET sort_order = :sort_order WHERE linkid = :linkid';
						$query = MatrixDAL::preparePdoQuery($sql);
						MatrixDAL::bindValueToPdo($query, 'sort_order', $i);
						MatrixDAL::bindValueToPdo($query, 'linkid', $link);
						MatrixDAL::execPdoQuery($query);

						// all good
						$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
						$stats[] = $node;
					} catch (DALException $e) {
						// no good
						$message = 'Unable to the sort_order for Link ID #'.$link;
						error_log($message);
						$errors[] = $message;
						$return_value = FALSE;
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					}//end try
				}//end if
			}//end foreach
		}//end foreach

	}//end if

	// Now descend into this node's children if any
	foreach ($children as $current_child) {
		// If a valid asset do the sorting
		$is_asset = $GLOBALS['SQ_SYSTEM']->am->assetExists($current_child);
		if ($is_asset) {
			$child_status = resort($current_child, $errors, $stats);
			if (!$child_status) {
				// Woo woo, I found an error, I better report it
				$return_value = FALSE;
			}//end if
		}//end if
	}//end foreach

	// Report my status
	return $return_value;

}//end resort()


/**
* This function returns a list of the children under the supplied root node
*
* @param string	$node		The root node
* @param array	&$errors	The error message holding array (by reference)
*
* @return array
* @access public
*/
function get_children($node, &$errors)
{
	// Globalise
	global $db;

	// Empty by default until something goes into it
	$children = Array();

	$children_sql = 'SELECT minorid FROM sq_ast_lnk WHERE majorid = :majorid ORDER BY link_type,sort_order';
	$children_query = MatrixDAL::preparePdoQuery($children_sql);
	MatrixDAL::bindValueToPdo($children_query, 'majorid', $node);
	$children_result = MatrixDAL::executePdoAssoc($children_query);

	// Sort the children into a format this script likes
	foreach ($children_result as $found_child) {
		if (isset($found_child['minorid'])) {
			$children[] = $found_child['minorid'];
		}//end if
	}//end foreach

	// Return all the children (none if none found)
	return $children;

}//end get_children()


?>
