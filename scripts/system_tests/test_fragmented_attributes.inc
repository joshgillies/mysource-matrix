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
		$asset_sql  = 'SELECT v.assetid FROM sq_ast_attr_val v LEFT JOIN sq_ast a ON (v.assetid=a.assetid) WHERE a.assetid IS NULL';
		$asset_sql .= ' UNION ';
		// Assets with missing attributes.
		$asset_sql .= 'SELECT v.assetid FROM sq_ast_attr_val v LEFT JOIN sq_ast_attr a ON (v.attrid=a.attrid) WHERE a.attrid IS NULL';
		$asset_results = MatrixDAL::executeSqlAssoc($asset_sql);

		if (!empty($asset_results)) {
			$status = FALSE;
			foreach ($asset_results as $result) {
				$errors[] = 'Asset #'.$result['assetid'].' on the attributes table does not exist';
			}//end foreach
		}//end if

		// Missing attributes
		$attr_sql = 'SELECT v.attrid FROM sq_ast_attr_val v LEFT JOIN sq_ast_attr a ON (v.attrid=a.attrid) WHERE a.attrid IS NULL';
		$attr_results = MatrixDAL::executeSqlAssoc($attr_sql);

		if (!empty($attr_results)) {
			$status = FALSE;
			foreach ($attr_results as $result) {
				$errors[] = 'Attribute #'.$result['attrid'].' on the attributes table does not exist';
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
