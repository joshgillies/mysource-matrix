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
* $Id: recreate_link_tree.php,v 1.11.2.5 2005/09/05 23:23:45 amiller Exp $
*
*/

/**
* Use this script to (re)create the link tree.
*
* Usage: php scripts/recreate_link_tree.php [SYSTEM ROOT] > tmp.sql
*
* Sends the SQL Commands that need to be run to STDOUT
* and send status information to STDERR
*
* The main use of this script is recreate the treeids when the
* SQ_CONF_ASSET_TREE_BASE or SQ_CONF_ASSET_TREE_SIZE config options change
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.11.2.5 $
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
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

error_reporting(E_ALL);
ini_set('memory_limit', '-1');

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

//--        MAIN()        --//

$db = &$GLOBALS['SQ_SYSTEM']->db;

$pgdb = ($db->phptype == 'pgsql');

$script_start = time();

echo_headline('TRUNCATING TREE');

if ($pgdb) {
	$sql = 'TRUNCATE sq_ast_lnk_tree;';
} else {
	$sql = 'TRUNCATE TABLE sq_ast_lnk_tree;';
}

echo $sql,"\n";

$sql = 'SELECT l.majorid, l.linkid, l.minorid
		FROM
			sq_ast_lnk l
		WHERE
			'.db_extras_bitand($db, 'l.link_type', $db->quote(SQ_SC_LINK_SIGNIFICANT)).' > 0
		ORDER BY l.sort_order';

$result = $db->query($sql);
assert_valid_db_result($result);

echo_headline('ANALYSING '.$result->numRows().' SIGNIFICANT LINKS');

$echo_i = 0;
$index = Array();
while (null !== ($data = $result->fetchRow())) {
	$majorid = $data['majorid'];
	unset($data['majorid']);
	if (!isset($index[$majorid])) $index[$majorid] = Array();
	$index[$majorid][] = $data;

	$echo_i++;
	if ($echo_i % 200 == 0) fwrite(STDERR, '.');
}
fwrite(STDERR, "\n");

$result->free();

echo_headline('CREATING INSERTS');

// if the DB is postgres use the COPY syntax for quicker insert
if ($pgdb) {
	$sql = "COPY sq_ast_lnk_tree (treeid, linkid, num_kids) FROM stdin;\n"
			.$db->quoteSmart('-')."\t".$db->quoteSmart(1)."\t".$db->quoteSmart(count($index[1]));

} else {
	$sql = 'INSERT INTO sq_ast_lnk_tree (treeid, linkid, num_kids) VALUES ('.$db->quoteSmart('-').', '.$db->quoteSmart(1).', '.$db->quoteSmart(count($index[1])).');';

}

echo $sql,"\n";

$echo_i = 0;
recurse_tree_create(1, '');
fwrite(STDERR, "\n");

if ($pgdb) {
	echo "\\.\n";
} else {
	echo "COMMIT;\n";
}

echo_headline($echo_i.' TREE ENTRIES CREATED');

$script_end = time();
$script_duration = $script_end - $script_start;
echo '-- Script Start : ', $script_start, '    Script End : ', $script_end, "\n";
echo '-- Script Duration: '.floor($script_duration / 60).'mins '.($script_duration % 60)."seconds\n";
fwrite(STDERR, '-- Script Duration: '.floor($script_duration / 60).'mins '.($script_duration % 60)."seconds\n");


//--        FUNCTIONS        --//


/**
* Print a headline to STDERR
*
* @param string		$s	the headline
*
* @return void
* @access public
*/
function echo_headline($s)
{
	static $start = 0;

	if ($start) {
		$end = time();
		$duration = $end - $start;
		fwrite(STDERR, '-- Duration: '.floor($duration / 60).'mins '.($duration % 60)."seconds\n");
	}

	fwrite(STDERR, "--------------------------------------\n$s\n--------------------------------------\n");

	$start = time();

}//end echo_headline()


/**
* Re-creates the link tree table starting from th
*
* @param string		$majorid	the ID of the asset whose children to recreate
* @param string		$path		the path so far
*
* @return void
* @access public
*/
function recurse_tree_create($majorid, $path)
{
	global $db, $index, $echo_i, $pgdb;

	foreach ($index[$majorid] as $i => $data) {
		$treeid   = $path.asset_link_treeid_convert($i, true);
		$num_kids = (empty($index[$data['minorid']])) ? 0 : count($index[$data['minorid']]);

		if ($pgdb) {
			$sql = $db->quoteSmart($treeid)."\t".$db->quoteSmart((int) $data['linkid'])."\t".$db->quoteSmart($num_kids);
		} else {
			$sql = 'INSERT INTO sq_ast_lnk_tree (treeid, linkid, num_kids) VALUES ('.$db->quoteSmart($treeid).','.$db->quoteSmart((int) $data['linkid']).','.$db->quoteSmart($num_kids).');';
		}

		echo $sql,"\n";

		$echo_i++;
		if ($echo_i % 200 == 0) fwrite(STDERR, '.');

		if ($num_kids) {
			recurse_tree_create($data['minorid'], $treeid);
		}

	}//end foreach

}//end recurse_tree_create()


?>
