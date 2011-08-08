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
* $Id: recreate_link_tree.php,v 1.27.6.1 2011/08/08 23:08:22 akarelia Exp $
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
* @author  Luke Wright <lwright@squiz.net>
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.27.6.1 $
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
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

//--        MAIN()        --//

$db = MatrixDAL::getDb();
$pgdb = (MatrixDAL::getDbType() == 'pgsql');

$script_start = time();

echo_headline('TRUNCATING TREE');

if ($pgdb) {
	$sql = 'TRUNCATE sq_ast_lnk_tree;';
} else {
	$sql = 'TRUNCATE TABLE sq_ast_lnk_tree;';
}

echo $sql,"\n";

// Work out how many significant links we actually have
$sql = 'SELECT COUNT(*) FROM sq_ast_lnk l WHERE	'.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', MatrixDAL::quote(SQ_SC_LINK_SIGNIFICANT)).' > 0';
$num_links = MatrixDAL::executeSqlOne($sql);
echo_headline('ANALYSING '.$num_links.' SIGNIFICANT LINKS');

// Because DAL doesn't support row-by-row fetch, we'll do chunked fetch instead,
// 2000 at a time. If we don't get that many results, we'll do one last query
// in case anything had been added since - if we get zero results, we are done
$base_sql = 'SELECT l.majorid, l.linkid, l.minorid
		FROM
			sq_ast_lnk l
		WHERE
			'.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', MatrixDAL::quote(SQ_SC_LINK_SIGNIFICANT)).' > 0
		ORDER BY
			l.sort_order, l.linkid, l.majorid, l.minorid';

$offset = 0;
$chunk_size = 2000;
$echo_i = 0;

$index = Array();

while (TRUE) {
	$sql = db_extras_modify_limit_clause($base_sql, MatrixDAL::getDbType(), $chunk_size, $offset);
	$result = MatrixDAL::executeSqlAssoc($sql);

	// If no further results, we're done.
	if (count($result) == 0) break;

	foreach ($result as $data) {
		$majorid = $data['majorid'];
		unset($data['majorid']);
		if (!isset($index[$majorid])) $index[$majorid] = Array();
		$index[$majorid][] = $data;

		$echo_i++;
		if ($echo_i % 200 == 0) fwrite(STDERR, '.');
	}

	// advance the chains by as many results we actually got
	$offset += count($result);

}//end while

fwrite(STDERR, "\n");

echo_headline('CREATING INSERTS');

// if the DB is postgres use the COPY syntax for quicker insert
if ($pgdb) {
	$sql = "COPY sq_ast_lnk_tree (treeid, linkid, num_kids) FROM stdin;\n"
			.'-'."\t".'1'."\t".(string)count($index[1]);

} else {
	$sql = 'INSERT INTO sq_ast_lnk_tree (treeid, linkid, num_kids) VALUES ('.MatrixDAL::quote('-').', '.MatrixDAL::quote(1).', '.MatrixDAL::quote(count($index[1])).');';

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

// as a part of bug fix #3864 Rebuilding Link Tree breaks triggers , ask user runnin the script to 
// also run regenerate_treeids_for_triggers.php script as well
fwrite(STDERR, "\n\n*************************************************************************
 PLEASE RUN regenerate_treeids_for_triggers.php SCRIPT AFTER RECREATING
 LINK TREE IF YOU HAVE ANY TRIGGERS INSTALLED ON THE SYSTEM
*************************************************************************\n\n");

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
			$sql = $treeid."\t".(string)$data['linkid']."\t".(string)$num_kids;
		} else {
			$sql = 'INSERT INTO sq_ast_lnk_tree (treeid, linkid, num_kids) VALUES ('.MatrixDAL::quote($treeid).','.MatrixDAL::quote((int) $data['linkid']).','.MatrixDAL::quote($num_kids).');';
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
