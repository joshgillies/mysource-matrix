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
* $Id: recreate_link_tree.php,v 1.13 2005/07/12 03:36:11 lwright Exp $
*
*/

/**
* Use this script to (re)create the link tree.
* The main use of this script is recreate the treeids when the
* SQ_CONF_ASSET_TREE_BASE or SQ_CONF_ASSET_TREE_SIZE config options change
*
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.13 $
* @package MySource_Matrix
*/

require_once dirname(dirname(__FILE__)).'/core/include/init.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

$db = &$GLOBALS['SQ_SYSTEM']->db;


/**
* Re-creates the link tree table starting from th
*
* @param string	$majorid	the ID of the asset you wish to re-create the tree from
*
* @return void
* @access public
*/
function recurse_tree_create($majorid)
{
	$db =& $GLOBALS['SQ_SYSTEM']->db;

	$sql = 'SELECT
				COUNT(*)
			FROM
				sq_ast_lnk
			WHERE
				link_type & '.SQ_SC_LINK_SIGNIFICANT.' > 0';
	$link_count = $db->getOne($sql);
	assert_valid_db_result($link_count);

	bam('ANALYSING '.$link_count.' SIGNIFICANT LINKS - THIS MAY TAKE A WHILE...');

	// no link to our children; we need to (re-)create the tree, obviously... darn it
	// so grab the links we need (significant ones only)
	$top_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($majorid, SQ_SC_LINK_SIGNIFICANT);
	$links = Array();

	// reorder them so that it's indexed by their prospective treeid
	$child_index = 0;
	foreach ($top_links as $link) {
		$treeid = asset_link_treeid_convert($child_index,true);
		if ($link['linkid'] != 0) {
			$links[$treeid] = $link;
			$child_index++;
		}
	}

	// now search for child links and give them treeids
	for (reset($links); null !== ($k = key($links)); next($links)) {
		$link =& $links[$k];

		$child_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($link['minorid'], SQ_SC_LINK_SIGNIFICANT);
		$child_index = 0;
		foreach ($child_links as $child_link) {
			$treeid = $k.asset_link_treeid_convert($child_index,true);
			if ($child_link['linkid'] != 0) {
				$links[$treeid] = $child_link;
				$child_index++;
			}
		}
		$link['num_kids'] = count($child_links);
	}

	bam('RE-CREATING TREE ENTRIES...');

	// adding root folder?
	if ($majorid == 1) {
		$sql = 'INSERT INTO
					sq_ast_lnk_tree
					(
						treeid,
						linkid,
						num_kids
					)
					VALUES
					(
						'.$db->quoteSmart('-').',
						'.$db->quoteSmart(1).',
						'.$db->quoteSmart(count($top_links)).'
					)';

		$result = $db->query($sql);
		assert_valid_db_result($result);
		echo sprintf('[ Link Id %10d ] %s', 1, '-')."\n";
		//echo 'Added treeid - for linkid 1'."\n";
	}

	foreach ($links as $treeid => $this_link) {
		// remove any 'zero linkid' entries
		$sql = 'DELETE FROM
					sq_ast_lnk_tree
				WHERE
					treeid = '.$db->quoteSmart($treeid);

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// now insert the tree entry
		$sql = 'INSERT INTO
				sq_ast_lnk_tree
				(
					treeid,
					linkid,
					num_kids
				)
				VALUES
				(
					'.$db->quoteSmart($treeid).',
					'.$db->quoteSmart($this_link['linkid']).',
					'.$db->quoteSmart($this_link['num_kids']).'
				)';

		$result = $db->query($sql);
		echo sprintf('[ Link Id %10d ] %s', $this_link['linkid'], $treeid)."\n";
		assert_valid_db_result($result);
	}

	bam((count($links)+($majorid == 1 ? 1 : 0)).' TREE ENTRIES CREATED');

}//end recurse_tree_create()


bam('TRUNCATING TREE...');

$sql = 'TRUNCATE sq_ast_lnk_tree';
$result = $db->query($sql);
assert_valid_db_result($result);

recurse_tree_create(1);

?>
