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
* $Id: condition_has_children.inc,v 1.2 2013/09/02 01:39:07 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Has_Children
*
* Purpose
*     This condition compares the asset type of the current asset's children against a known value.
*
* Based on by condition_asset_type
* @author Anh Ta <ata@squiz.co.uk>
* @author  Keith Tait <keith.tait@ed.ac.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Has_Children extends Condition
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
	* @param Asset	$asset			the parent of the assets to evaluate against
	* @param array  $condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		if (isset($condition_data['types'])) {
			$asset_types = explode(',', $condition_data['types']);
			foreach ($asset_types as $index => $asset_type) {
			    $asset_types[$index] = trim($asset_type);
			}
		}
		else {
		    return FALSE;
		}


		if (isset($condition_data['link_types'])) {
			$link_types = explode(',', $condition_data['link_types']);
			foreach ($link_types as $index => $link_type) {
			    $link_types[$index] = trim($link_type);
			}
		}
		else {
		    $link_types = Array('1', '2');
		}

		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, $asset_types, TRUE, NULL, NULL, NULL, TRUE, NULL, NULL, TRUE, NULL, $link_types);
		return !empty($children);
		
	}//end evaluate()


}//end class

?>