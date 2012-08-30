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
* $Id: test_fragmented_attributes.inc,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Fragmented_Attributes
*
*	A test to check for fragmented attributes on the attributes table
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Test_Fragmented_Attributes
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Fragmented Attributes Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for fragmented attributes on the attributes table';

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
		// Missing assets
		$asset_sql = 'SELECT assetid FROM sq_ast_attr_val WHERE assetid NOT IN (SELECT DISTINCT assetid FROM sq_ast) OR attrid NOT IN (SELECT DISTINCT attrid FROM sq_ast_attr)';
		$asset_results = MatrixDAL::executeSqlAssoc($asset_sql);

		foreach ($asset_results as $index => $result) {
			$asset_results[$index] = $result['assetid'];
		}
		$asset_results = array_unique($asset_results);

		if (!empty($asset_results)) {
			$status = FALSE;
			foreach ($asset_results as $result) {
				$errors[] = 'Asset #'.$result.' on the attributes table does not exist';
			}//end foreach
		}//end if

		// Missing attributes
		$attr_sql = 'SELECT attrid FROM sq_ast_attr_val WHERE attrid NOT IN (SELECT DISTINCT attrid FROM sq_ast_attr)';
		$attr_results = MatrixDAL::executeSqlAssoc($attr_sql);

		foreach ($attr_results as $index => $result) {
			$attr_results[$index] = $result['attrid'];
		}
		$attr_results = array_unique($asset_results);

		if (!empty($attr_results)) {
			$status = FALSE;
			foreach ($attr_results as $result) {
				$errors[] = 'Attribute #'.$result.' on the attributes table does not exist';
			}//end foreach
		}//end if

		if (!$status) {
			$count = count($asset_results) + count($attr_results);
			$messages[] = 'There are '.$count.' fragmented attribute(s) on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
