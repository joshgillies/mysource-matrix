<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: root_folder.inc,v 1.36 2003/10/14 05:28:46 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Root_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Root_Folder extends Folder
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Root_Folder($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$this->_tmp[__CLASS__.'_in_create'] = true;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT COUNT(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Root Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$this->setAttrValue('name', 'Root Folder');
		$ret_val = parent::create($link);
		if ($ret_val) {
			if ($this->id != 1) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('The new Root Folder Asset was not given assetid #1. This needs to be fixed by YOU, consider this system corrupted', E_USER_ERROR);
				return false;
			}

			$linkid = $db->nextId('sq_sequence_asset_link');
			if (DB::isError($linkid)) {
				trigger_error($linkid->getMessage().'<br/>'.$linkid->getUserInfo(), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			if ($linkid != 1) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error('The new Root Folder Asset was not given linkid #1. This needs to be fixed by YOU, consider this system corrupted', E_USER_ERROR);
				return false;
			}

			// Right we need to do some manipulation for the link table to put in some initial values
			// so that the root folder, like everything else, has a link where it is a minor party
			$values = Array('linkid'         => 1,
							'majorid'        => 0,
							'minorid'        => 1,
							'link_type'      => SQ_LINK_TYPE_1,
							'value'          => '',
							'sort_order'     => 1,
							'dependant'      => 1,
							'exclusive'      => 1,
							'updated'        => ts_iso8601(time()),
							'updated_userid' => 0
							);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$values = Array('treeid'             => '',
							'linkid'             => 1,
							'num_immediate_kids' => 0
						);
			if (!$GLOBALS['SQ_SYSTEM']->rollbackInsert('asset_link_tree', $values)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		// create failed
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

		}// end if ret_val

		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

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
		return ($short_name) ? '/' : 'Root Folder';

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
		if (empty($assetids)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) return true;
			return false;
		} else {
			return parent::_checkPermissionAccess($perm, $assetids);
		}

	}//end checkAccess()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if ($name == 'name' && empty($this->_tmp[__CLASS__.'_in_create'])) {
			trigger_error('You are not able to alter the name for the root folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

}//end class

?>