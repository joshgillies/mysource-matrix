<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: condition_in_user_group.inc,v 1.4 2006/01/26 22:34:05 lwright Exp $
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
* @version $Revision: 1.4 $
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