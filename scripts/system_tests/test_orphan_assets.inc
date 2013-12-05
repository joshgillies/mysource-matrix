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
* $Id: test_orphan_assets.inc,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Orphan_Asset
*
*	A test to run to find orphaned assets on the system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Test_Orphan_Assets
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Orphaned Assets Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for orphaned assets in Matrix';

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
		$sql = 'SELECT assetid FROM sq_ast a LEFT JOIN (SELECT minorid FROM sq_ast_lnk WHERE link_type < 8) l ON (a.assetid=l.minorid) WHERE l.minorid IS NULL';
		$results = MatrixDAL::executeSqlAssoc($sql);
		if (!empty($results)) {

			foreach ($results as $index => $result) {
				$results[$index] = $result['assetid'];
			}
			$results = array_unique($results);

			$status = FALSE;
			$messages[] = 'There are '.count($results).' orphaned asset(s) on the system';
			foreach ($results as $asset) {
				$errors[] = 'Asset #'.$asset.' is orphaned on the system';
			}//end foreach
		}//end if

		return $status;

	}//end test()

}//end class
?>
