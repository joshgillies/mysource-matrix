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
* $Id: test_user_permissions.inc,v 1.3 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Test_User_Permissions
*
*	A test to run to find invalid users on the permission table 
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Test_User_Permissions
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'User Permissions Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for invalid users on the permission table in Matrix';

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
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$sql = 'SELECT DISTINCT userid FROM sq_ast_perm ORDER BY userid';
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
		$assetids = MatrixDAL::executeSqlAssoc($sql, 0);

		foreach ($assetids as $assetid) {
			// Does the user exist?
			$asset = $am->getAsset($assetid, '', TRUE);
			if (is_null($asset)) {
				$status=FALSE;
				$count++;
				$errors[] = 'User #'.$assetid.' does not exist';
			} else {
				$am->forgetAsset($asset, TRUE);
			}//end if
			unset($asset);
		}//end foreach

		if (!$status && !empty($count)) {
			$messages[] = 'There are '.$count.' users on the permissions table that do not exist';
		}//end if

		return $status;

	}//end test()

}//end class
?>
