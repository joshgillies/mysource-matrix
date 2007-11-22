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
* $Id: condition_write_access.inc,v 1.12 2007/11/22 22:25:36 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';

/**
* Condition_Write_Access
*
* Purpose
*     This conditional checks to see if the currently logged in user has write
*     access to the current asset
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Write_Access extends Condition
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
		// first, check if someone's logged in
		if ($GLOBALS['SQ_SYSTEM']->userPublic()) return FALSE;

		// next, check to see if the currently logged in user
		// has write access
		if (!$asset->writeAccess('')) return FALSE;

		return TRUE;

	}//end evaluate()


}//end class
?>