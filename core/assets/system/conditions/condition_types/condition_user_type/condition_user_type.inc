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
* $Id: condition_user_type.inc,v 1.3 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';


/**
* Condition_User_Type
*
* Purpose
*     This condition compares the asset type of the currently logged
*     in user against a known value.
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Type extends Condition
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
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		if (!isset($condition_data['user_types'])) {
			return FALSE;
		}

		$user_asset_types = explode(',', $condition_data['user_types']);
		foreach ($user_asset_types as $asset_type) {
			if (strcasecmp(trim($asset_type), get_class($GLOBALS['SQ_SYSTEM']->user)) === 0) {
				return TRUE;
			}
		}
		return FALSE;

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
							'1'	=> translate('The user is of any of the following user types'),
							'0' => translate('The user is NOT of any of the following user types'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>