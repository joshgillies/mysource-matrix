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
* $Id: role.inc,v 1.4 2012/08/30 01:09:17 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Role
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Role extends User_Group
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Role($assetid=0)
	{
		$this->User_Group($assetid);

	}//end constructor


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['security']    = max($lock_types) * 2;
		$lock_types['preferences'] = max($lock_types) * 2;
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['role'] = Array('card' => 'M', 'exclusive' => FALSE);
		$allowed[SQ_LINK_TYPE_1]['role'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $allowed;

	}//end _getAllowedLinks()


}//end class
?>
