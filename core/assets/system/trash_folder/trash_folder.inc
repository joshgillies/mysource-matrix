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
* @package Resolve_Packages
* @subpackage __core__
*/
class Trash_Folder extends Folder
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Trash_Folder($assetid=0) 
	{
		$this->Folder($assetid);
	}


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

		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Trash Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('name', 'Trash');

		$ret_val = parent::create($link);
		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

	}//end create()


	/**
	* Creates a duplicate of this asset. It DOES NOT duplicate the links that this asset has either to it or from it
	* Returns a reference to the new object or if an error occured FALSE
	*
	* @param bool	$dupe_directory	whether or not to duplicate thia assets data directory
	*
	* @return mixed object Asset or NULL
	* @access public
	* @see Asset::duplicate()
	*/
	function &duplicate($dupe_directory=true)
	{
		trigger_error('Unable to duplicate Trash Folder, you are only allowed one', E_USER_WARNING);
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
		trigger_error('Unable to morph Trash Folder', E_USER_WARNING);
		return false;
	}//end morph()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		if ($name == 'name' && empty($this->_tmp[__CLASS__.'_in_create'])) {
			trigger_error('You are not able to alter the name for the trash folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value, $dont_run_updated);
	}//end setAttrValue()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string	$sort_order	the position in the links list that this link should take, 
	*							if less than zero places at end of list
	*
	* @see Asset::moveLinkPos()
	*
	* @return boolean
	* @access public
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_error('You cannot move assets\'s around in the trash', E_USER_WARNING);
		return false;
	}//end moveLinkPos()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param int			$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		if (!$this->id) return false;
		if (!is_a($minor, 'Asset')) return 'Minor is not an asset';
		return true;

	}// end canCreateLink()


	/**
	* Prints out the backend interface for the asset
	* it's up to the kids to override
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
		$ei = &$this->getEI();
		$prefix = $ei->getPrefix($this);

		if (isset($_REQUEST[$prefix.'_purge_complete']) && (int)$_REQUEST[$prefix.'_purge_complete'] === 1) {
			$o->addOnLoad('	if (top.sidenav && top.sidenav.reload_asset) {
								top.sidenav.reload_asset("2");
							}');
			parent::paintBackend($o);
		} else if (isset($_REQUEST[$prefix.'_purge_trash']) && (int)$_REQUEST[$prefix.'_purge_trash'] === 1) {
			$o->addFormActionGetVar($prefix.'_purge_trash', 1);
			$ef = $this->getEditFns();
			$ef->_purgeTrash($this, $o, $prefix);
		} else {
			parent::paintBackend($o);
		}

	}// end paintBackend()


	/**
	* Returns true if the passed assetid has read access to this asset
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
	* Returns true if the passed assetid has write access to this asset
	*
	* @param array		$assetids	An array of user_group and/or user ids to check for write access
	*								If the array is empty, the current user will be checked
	* @param boolean	$effective	effective access based on locking and status
	* @param boolean	$editing	effective access based on live edit access
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($assetids=Array(), $effective=true, $editing=true)
	{
		return true;
	}//end writeAccess()


	/**
	* Returns true if the current user can purge the trash
	*
	* @return boolean
	* @access public
	*/
	function purgeAccess()
	{
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;
		if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
		return false;
	}//end purgeAccess()

}//end class
?>
