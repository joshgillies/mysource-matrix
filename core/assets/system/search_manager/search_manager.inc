<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager.inc,v 1.5 2003/10/21 06:30:43 dwong Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types. 
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.5 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

class Search_Manager extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->Asset($assetid);
	} // end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
	}//end _getName();


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()



}//end class
?>
