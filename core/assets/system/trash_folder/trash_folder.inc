<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Trash_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Trash_Folder extends Folder
{

	/**
	* Indicates whether we are currently in create() or not
	*
	* @see setAttribute()
	* @var boolean
	*/
	var $_in_create = false;

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Trash_Folder($assetid=0) 
	{
		$this->Folder($assetid);
	}


	/*
	* Create this asset
	*
	* @return boolean
	* @access public
	*/
	function create()
	{

		$this->_in_create = true;

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote(get_class($this));
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Trash Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}
		
		$ret_val = parent::create('Trash');
		$this->_in_create = false;
		return $ret_val;

	}#end create()

	/*
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttribute($name, $value) {

		if ($name == 'name' && !$this->_in_create) {
			trigger_error('You are not able to alter the name for the trash folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttribute($name, $value);

	}#end setAttribute()


}#end class
?>
