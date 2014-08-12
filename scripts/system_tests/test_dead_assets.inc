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
* $Id: test_dead_assets.inc,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Dead_Assets
*
*	A test to run to find dead assets on the system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Test_Dead_Assets
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Dead Assets Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for dead assets in the link table in Matrix';

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
		$minor_sql = 'SELECT DISTINCT minorid FROM sq_ast_lnk l LEFT JOIN sq_ast a ON (l.minorid=a.assetid) WHERE a.assetid IS NULL AND linkid <> 1';
		$minor_result = MatrixDAL::executeSqlAssoc($minor_sql);

		if (!empty($minor_result)) {
			$status = FALSE;
			foreach ($minor_result as $result) {
				$errors[] = 'Asset #'.$result['minorid'].' was not found on the system';
			}//end foreach
		}//end if

		$major_sql = 'SELECT DISTINCT majorid FROM sq_ast_lnk l LEFT JOIN sq_ast a ON (l.majorid=a.assetid) WHERE a.assetid IS NULL AND linkid <> 1';
		$major_result = MatrixDAL::executeSqlAssoc($major_sql);

		if (!empty($major_result)) {
			$status = FALSE;
			foreach ($major_result as $result) {
				$errors[] = 'Asset #'.$result['majorid'].' was not found on the system';
			}//end foreach
		}//end if
		
		if (!$status) {
			$count = count($minor_result) + count($major_result);
			$messages[] = 'There are '.$count.' dead asset(s) on the system';
		}//end if
		
		return $status;

	}//end test()

}//end class
?>
