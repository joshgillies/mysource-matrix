<?php

require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Designs_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Designs_Folder extends Folder
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
	function Designs_Folder($assetid=0) 
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
			trigger_error('Unable to create another System Designs Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', 'System Designs');

		$ret_val = parent::create($link);
		$this->_in_create = false;
		return $ret_val;

	}//end create()


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory		whether or not to duplicate thia assets data directory
	*
	* @return object Asset | NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate Systems Designs Folder, you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;
	}//end duplicate()


	/**
	* Morph's this asset to either one of it's parent type or one of it's children types
	* Only succeeds if $type_code asset's is allowed to link to all children that this asset has
	*
	* @param string	$new_type_code	the new type_code to attempt to cast this asset to
	*
	* @return bool
	* @access public
	* @see Asset::morph()
	*/
	function morph($new_type_code)
	{
		trigger_error('Unable to morph Designs Folder', E_USER_WARNING);
		return false;
	}//end morph()


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
			trigger_error('You are not able to alter the name for the designs folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value, $dont_run_updated);

	}//end setAttrValue()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param string			$value		the value that is to be associated with this link
	* @param string			$sort_order	the position in the links list that this link should take, 
	*									if null or less than zero places at end of list
	* @param string			$dependant	'0' / '1' on whether the this asset is dependant on the 
	*									asset that will be linked by this new link
	* @param string			$exclusive	'0' / '1' on whether the this asset is linked exclusivly 
	*									to the asset that will be linked by this new link
	*
	* @return int
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0')
	{
		if (empty($GLOBALS['SQ_INSTALL'])) {
			trigger_error('You cannot create a new link to the designs folder', E_USER_WARNING);
			return 0;
		
		} else {
			return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);
		}

	}//end createLink()


	/**
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid)
	{
		trigger_error('You cannot delete anything from the designs folder', E_USER_WARNING);
		return false;
	}// end deleteLink()


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_TYPE_1 => Array('design' => Array('card' => 'M', 'exclusive' => true)));

	}//end _getAllowedLinks()


	/**
	* Returns true if the passed assetid has write access to this asset
	*
	* @param array $assetids An array of user_group and/or user ids to check for write access
	*						 If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array())
	{
		// only root can write to this folder
		return $GLOBALS['SQ_SYSTEM']->userRoot();
	}//end writeAccess()


}//end class
?>
