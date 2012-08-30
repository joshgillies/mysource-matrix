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
* $Id: test_general_status.inc,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_General_Status
*
*	A basic test to get a general status and statistics on the system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Test_General_Status
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'General Status Check';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A basic test to get a general status and statistics on the system';

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
		$status = '2';

		$sql = 'SELECT COUNT(*) FROM sq_ast';
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$messages[] = 'There are '.$result.' asset(s) on the system';
		}//end if

		$sql = "SELECT COUNT(*) FROM sq_ast_lnk WHERE link_type<='4'";
		$result = MatrixDAL::executeSqlOne($sql);
		if (!empty($result)) {
			$messages[] = 'There are '.$result.' asset link(s) on the system';
		}//end if

		return $status;

	}//end test()

}//end class
?>
