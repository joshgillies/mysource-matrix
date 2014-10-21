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
* $Id: condition_in_user_group.inc,v 1.8 2012/08/30 01:09:10 ewang Exp $
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
* @version $Revision: 1.8 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Evaluate this condition
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, $condition_data)
	{
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


	/**
	* Get a list of match keywords
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getMatchKeywords()
	{
		$match_options = Array(
							'1'	=> translate('The user is in any of the following user groups'),
							'0' => translate('The user is NOT in any of the following user groups'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>
