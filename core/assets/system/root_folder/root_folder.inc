<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Root_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Root_Folder extends Folder
{

	/**
	* Indicates whether we are currently in create() or not
	*
	* @see setAttrValue()
	* @var boolean
	*/
	var $_in_create = false;


	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Root_Folder($assetid=0) 
	{
		$this->Folder($assetid);
	}

	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link)
	{
		$this->_in_create = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Root Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}
		
		$this->setAttrValue('name', '/');
		$ret_val = parent::create($link);
		$this->_in_create = false;
		return $ret_val;

	}//end create()


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


	/**
	* Returns true if at least one of the passed assetids has read access to this asset
	*
	* @param array $assetids An array of user_group and/or user ids to check for read access
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		return true;
	}//end readAccess()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if ($name == 'name' && !$this->_in_create) {
			trigger_error('You are not able to alter the name for the root folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}//end setAttrValue()

}//end class
?>
