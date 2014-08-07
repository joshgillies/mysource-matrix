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
* $Id: test_broken_sort_order.inc,v 1.4 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_Broken_Sort_Order
*
*	A test to find any broken sort order on the links table
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Test_Broken_Sort_Order
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Broken Sort Order Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to find any broken sort order on the links table';

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

		// Find the maximum sort order and count the links
		$sql = 'SELECT majorid, MAX(sort_order) AS max, count(*) AS count FROM sq_ast_lnk GROUP BY majorid';
		$results = MatrixDAL::executeSqlAssoc($sql);
		if (!empty($results)) {
			$count = 0;
			foreach ($results as $result) {
				if (($result['max'] + 1) != ($result['count'])) {
					$status = FALSE;
					$errors[] = 'The Asset #'.$result['majorid'].' has '.$result['count'].' assets underneath, but the sort order finishes at '.$result['max'];
					$count++;
				}//end if
			}//end foreach
			if (!$status && !empty($count)) {
				$messages[] = 'There are '.$count.' broken sort orders found on the system';
			}//end if
		}//end if

		return $status;

	}//end test()

}//end class
?>
