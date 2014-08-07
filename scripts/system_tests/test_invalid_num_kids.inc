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
* $Id: test_invalid_num_kids.inc,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Invalid_Num_Kids
*
*	A test to detect invalid num kids entry on link tree
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Test_Invalid_Num_Kids
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Invalid Number of Kids on Link Tree Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to detect invalid number of kids entries on link tree';

	}//end getDescription()


	/**
	 * The test to run
	 *
	 * @param array	$messages	The variable to 'chuck'	all the messages
	 * @param array	$errors		The variable to 'chuck'	all the verbose messages
	 *
	 * @return boolean|int
	 * @access public
	 */
	public static function test(&$messages, &$errors)
	{
		$status = TRUE;
		$count = 0;

		// Check the root folder first
		$root_sql = "SELECT pt.linkid, pt.treeid, pt.num_kids, count(ct.treeid) AS count FROM sq_ast_lnk_tree ct, sq_ast_lnk_tree pt WHERE pt.linkid = '1' AND ct.treeid LIKE '0000%' AND LENGTH(ct.treeid) = (LENGTH(pt.treeid) + ".SQ_CONF_ASSET_TREE_SIZE.") GROUP BY pt.linkid, pt.treeid, pt.num_kids";
		$root_results = MatrixDAL::executeSqlAssoc($root_sql);
		foreach ($root_results as $result) {
			if ($result['num_kids'] != $result['count']) {
				$status = FALSE;
				$count++;
				$errors[] = 'Link #'.$result['linkid'].' has '.$result['num_kids'].' kids listed, but actually '.$result['count'].' kids were found';
			}//end if
		}//end foreach

		// Next the rest of the tree
		$sql = "SELECT pt.linkid, pt.treeid, pt.num_kids, count(ct.treeid) AS count FROM sq_ast_lnk_tree ct, sq_ast_lnk_tree pt WHERE pt.linkid IN (SELECT DISTINCT linkid FROM sq_ast_lnk_tree WHERE linkid > '1' AND num_kids>0) AND ct.treeid LIKE pt.treeid || '%' AND LENGTH(ct.treeid) = (LENGTH(pt.treeid) + ".SQ_CONF_ASSET_TREE_SIZE.") GROUP BY pt.linkid, pt.treeid, pt.num_kids";
		$results = MatrixDAL::executeSqlAssoc($sql);
		foreach ($results as $result) {
			if ($result['num_kids'] != $result['count']) {
				$status = FALSE;
				$count++;
				$errors[] = 'Link #'.$result['linkid'].' has '.$result['num_kids'].' kids listed, but actually '.$result['count'].' kids were found';
			}//end if
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' invalid number of kids found on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
