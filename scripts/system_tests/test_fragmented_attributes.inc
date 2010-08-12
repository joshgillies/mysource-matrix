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
* $Id: test_fragmented_attributes.inc,v 1.1.2.2 2010/08/12 23:11:48 mhaidar Exp $
*
*/

/**
* Test_Fragmented_Attributes
*
*	A test to check for fragmented attributes on the attributes table
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.2.2 $
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
		$asset_result = MatrixDAL::executeSqlAssoc($asset_sql);
		if (!empty($asset_result)) {
			$status = FALSE;
			foreach ($asset_result as $result) {
				$errors[] = 'Asset #'.$result['assetid'].' on the attributes table does not exist';
			}//end foreach
		}//end if

		// Missing attributes
		$attr_sql = 'SELECT attrid FROM sq_ast_attr_val WHERE attrid NOT IN (SELECT DISTINCT attrid FROM sq_ast_attr)';
		$attr_result = MatrixDAL::executeSqlAssoc($attr_sql);
		if (!empty($attr_result)) {
			$status = FALSE;
			foreach ($attr_result as $result) {
				$errors[] = 'Attribute #'.$result['attrid'].' on the attributes table does not exist';
			}//end foreach
		}//end if

		if (!$status) {
			$count = count($asset_result) + count($attr_result);
			$messages[] = 'There are '.$count.' fragmented attribute(s) on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
