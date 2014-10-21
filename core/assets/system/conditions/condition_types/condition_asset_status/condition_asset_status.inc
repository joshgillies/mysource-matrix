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
* $Id: condition_asset_status.inc,v 1.3 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Asset_Status
*
* Purpose
*     This conditional checks to see if the current asset matches the
*     specified status
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Asset_Status extends Condition
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
		$required_statii = explode(',', $condition_data['status']);
		$current_status = $asset->status;

		$status_in_list = FALSE;

		foreach ($required_statii as $required_status) {

			if (!is_numeric($required_status)) {
				$required_status = get_status_value(trim($required_status));
			}//end if

			if ($current_status == $required_status) {
				$status_in_list = TRUE;
			}//end if


		}//end foreach

		return $status_in_list;

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
							'1'	=> translate('The current asset is in any of the following statuses'),
							'0' => translate('The current asset is NOT in any of the following statuses'),
						 );

		return $match_options;

	}//end getMatchKeywords()




}//end class
?>