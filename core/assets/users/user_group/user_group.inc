<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Group extends Folder
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function User_Group($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$allowed = parent::_getAllowedLinks();
		$allowed[SQ_LINK_TYPE_1]['user_group'] = Array('card' => 'M', 'exclusive' => false);
		$allowed[SQ_LINK_TYPE_1]['user']       = Array('card' => 'M', 'exclusive' => false);
		$allowed[SQ_LINK_TYPE_2]['user_group'] = Array('card' => 'M', 'exclusive' => false);
		$allowed[SQ_LINK_TYPE_2]['user']       = Array('card' => 'M', 'exclusive' => false);
		return $allowed;

	}//end _getAllowedLinks()

}//end class

?>