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
* @package Resolve_Packages
* @subpackage __core__
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
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$this->setAttrValue('name', '/');
		$ret_val = parent::create($link);
		if ($ret_val) {
			if ($this->id != 1) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('The new Root Folder Asset was not given assetid #1. This needs to be fixed by YOU, consider this system corrupted', E_USER_ERROR);
			}

			$linkid = $db->nextId('sq_sequence_asset_link');
			if (DB::isError($linkid)) {
				trigger_error($linkid->getMessage().'<br/>'.$linkid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return 0;
			}

			if ($linkid != 1) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('The new Root Folder Asset was not given linkid #1. This needs to be fixed by YOU, consider this system corrupted', E_USER_ERROR);
			}

			// Right we need to do some manipulation for the link table to put in some initial values 
			// so that the root folder, like everything else, has a link where it is a minor party
			$sql = 'INSERT INTO sq_asset_link 
					(linkid, majorid, minorid, link_type, value, sort_order, dependant, exclusive, last_updated, last_userid) 
					VALUES 
					(1, 0, 1, 
					'.$db->quote(SQ_LINK_TYPE_1).', '.$db->quote('').', 1, '.$db->quote('1').', 
					'.$db->quote('1').', '.$db->quote(ts_iso8601(time())).', 0);';
			$result = $db->query($sql);
			if (DB::isError($result)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			}

			$sql = 'INSERT INTO sq_asset_link_tree 
					(treeid, linkid) 
					VALUES ('.$db->quote('').', 1);';
			$result = $db->query($sql);
			if (DB::isError($result))  {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// create failed
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

		}// end if ret_val

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
	* Returns true if at least one of the passed assetids has the passed permission
	* Does not check current status of lock
	* Only root or a system admin have write/admin access to the root folder
	*
	* @param int	$perm		the permission to check access for
	* @param array	$assetids	An array of user_group and/or user ids to check for permission
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function _checkPermissionAccess($perm, $assetids=Array())
	{
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;
		if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
		return false;
	}//end checkAccess()


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
