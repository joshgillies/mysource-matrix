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
* $Id: condition_in_user_group.inc,v 1.5 2006/12/06 04:23:58 emcdonald Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Logged In Condition
*
* This condition determines whether the current user is a member of the selected user groups
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_In_User_Group extends Condition
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Condition_In_User_Group($assetid=0)
	{
		$this->Condition($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param object	&$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	function evaluate(&$asset, $condition_data)
	{
		if ($GLOBALS['SQ_SYSTEM']->userPublic()) return FALSE;
		if (empty($condition_data['groupids'])) return FALSE;

		// because a user can theoretically use conditions to be part of groups
		// we need to make sure that we aren't going to recurse infinetly
		static $evaluate_called = FALSE;
		if ($evaluate_called) return FALSE;
		$evaluate_called = TRUE;

		$ret_val = array_intersect($condition_data['groupids'], $GLOBALS['SQ_SYSTEM']->user->getUserGroups());

		$evaluate_called = FALSE;

		return !empty($ret_val);

	}//end evaluate()


}//end class

?>