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
* $Id: recreate_link_tree.php,v 1.6 2003/11/26 00:51:16 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Use this script to (re)create the link tree.
* The main use of this script is recreate the treeids when the 
* SQ_CONF_ASSET_TREE_BASE or SQ_CONF_ASSET_TREE_SIZE config options change
*
* 
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

require_once dirname(dirname(__FILE__)).'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$db = &$GLOBALS['SQ_SYSTEM']->db;

$sql = 'TRUNCATE sq_asset_link_tree';
$result = $db->query($sql);
if (DB::isError($result)) {
	pre_echo($result);
	exit();
}

$sql = 'SELECT linkid, majorid, minorid
		FROM sq_asset_link';

$all = $db->getAll($sql);
if (DB::isError($all)) {
	pre_echo($all);
	exit();
}

$index = Array();
foreach($all as $data) {
	$majorid = $data['majorid'];
	unset($data['majorid']);
	if (!isset($index[$majorid])) $index[$majorid] = Array();
	$index[$majorid][] = $data;
}


function recurse_tree_create($majorid, $path) {
	global $index, $db;
	static $e = 0;
	$e++;
	#if ($e > 100) return; // cheap recursion check
	foreach($index[$majorid] as $i => $data) {
		#printf("e : % 3d, i : % 3d, Majorid : % 3d, Minorid : % 3d, Linkid : % 3d\n", $e, $i, $majorid, $data['minorid'], $data['linkid']);
		$kid_path = $path.asset_link_treeid_convert($i, 1);
		$sql =	'INSERT INTO sq_asset_link_tree '.
				'(treeid, linkid) '.
				'VALUES '.
				'('.$db->quote($kid_path).', '.$db->quote($data['linkid']).');';
		echo $sql."\n";
		$result = $db->query($sql);
		if (DB::isError($result)) {
			pre_echo($result);
			exit();
		}
		if (!empty($index[$data['minorid']])) recurse_tree_create($data['minorid'], $kid_path);
	}
	$e--;
}

recurse_tree_create(1, '');

?>