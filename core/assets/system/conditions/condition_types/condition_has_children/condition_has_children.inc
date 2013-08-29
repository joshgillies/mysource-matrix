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
* $Id: condition_has_children.inc,v 1.1 2013/08/29 06:48:33 ewang Exp $
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
* @version $Revision: 1.1 $
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
		}
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, $asset_types, TRUE);
		if (!empty($children)) {
		    return TRUE;    
		} else {
		    return FALSE;
		}

	}//end evaluate()


}//end class

?>