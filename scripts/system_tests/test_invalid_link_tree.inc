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
* $Id: test_invalid_link_tree.inc,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Invalid_Link_Tree Asset
*
*	A test to run to find invalid link ids on the link tree in the system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Test_Invalid_Link_Tree
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Invalid Links on Link Tree Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for invalid links on the link tree in Matrix';

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
		$sql = 'SELECT DISTINCT lt.linkid FROM sq_ast_lnk_tree lt LEFT JOIN (SELECT linkid FROM sq_ast_lnk WHERE link_type <= 4) l ON (lt.linkid=l.linkid) WHERE l.linkid IS NULL';
		$results = MatrixDAL::executeSqlAssoc($sql);
		if (!empty($results)) {
			$status = FALSE;

			foreach ($results as $result) {
				$errors[] = 'Link #'.$result['linkid'].' on the link tree does not exist on the system';
			}

			$messages[] = 'There are '.count($results).' invalid link(s) in the link tree on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
