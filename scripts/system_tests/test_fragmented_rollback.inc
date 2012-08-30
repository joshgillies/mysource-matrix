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
* $Id: test_fragmented_rollback.inc,v 1.5 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Fragmented_Rollback
*
*	A test to find fragmented rollback
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
*/
class Test_Fragmented_Rollback
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Fragmented Rollback Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to find fragmented rollback entries';

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
		// TODO Add in extra verbose information
		$status = TRUE;

		// -- sq_ast_attr_val --
		$sql = 'SELECT count(*) FROM (SELECT assetid, attrid, count(*) AS id FROM sq_rb_ast_attr_val WHERE sq_eff_to is null GROUP BY assetid, attrid HAVING count(*) > 1)';
		if (MatrixDAL::getDbType() != 'oci') $sql.=' AS rollback';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$status = FALSE;
			$messages[] = 'There are '.$result.' fragmented entries on sq_ast_attr_val table';
		}//end if
		
		// -- sq_ast_lookup_value --
		$sql = 'SELECT count(*) FROM (SELECT url, name, count(*) AS id FROM sq_rb_ast_lookup_value WHERE sq_eff_to is null GROUP BY url, name HAVING count(*) > 1)';
		if (MatrixDAL::getDbType() != 'oci') $sql.=' AS rollback';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$status = FALSE;
			$messages[] = 'There are '.$result.' fragmented entries on sq_ast_lookup_value table';
		}//end if

		// -- sq_ast_lookup --
		$sql = 'SELECT count(*) FROM (SELECT url, assetid, root_urlid, count(*) AS id FROM sq_rb_ast_lookup WHERE sq_eff_to is null GROUP BY url, assetid, root_urlid HAVING count(*) > 1)';
		if (MatrixDAL::getDbType() != 'oci') $sql.=' AS rollback';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$status = FALSE;
			$messages[] = 'There are '.$result.' fragmented entries on sq_ast_lookup table';
		}//end if

		// -- sq_ast --
		$sql = 'SELECT count(*) FROM (SELECT assetid, count(*) AS id FROM sq_rb_ast WHERE sq_eff_to is null GROUP BY assetid HAVING count(*) > 1)';
		if (MatrixDAL::getDbType() != 'oci') $sql.=' AS rollback';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$status = FALSE;
			$messages[] = 'There are '.$result.' fragmented entries on sq_ast table';
		}//end if
		
		return $status;
		
	}//end test()

}//end class
?>
