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
* $Id: root_folder.inc,v 1.54 2012/08/30 01:09:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Root_Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.54 $
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
	* Return linkid (which should always be link *1* as this is the root folder)
	* or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		$this->_tmp[__CLASS__.'_in_create'] = TRUE;

		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$ret_val = parent::create($link);
		if ($ret_val) {
			if ($this->id != 1) {
				throw new Exception('Unable to create root folder, as it did not receive asset ID #1. Fragments of another system may exist in the database; please ensure the database is clear of previous installations before installing.');
				trigger_localised_error('CORE0077', E_USER_ERROR);
				return FALSE;
			}

			$db = DAL::getDb();

			$linkid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_ast_lnk_seq'));

			if ($linkid != 1) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				throw new Exception('Unable to create root folder, as it did not receive link ID #1. Fragments of another system may exist in the database; please ensure the database is clear of previous installations before installing.');
				trigger_localised_error('CORE0078', E_USER_ERROR);
				return FALSE;
			}

			require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
			$db = DAL::getDb();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			// right, we need to do some manipulation for the link table to put in some initial values
			// so that the root folder, like everything else, has a link where it is a minor party
			$sql = 'INSERT INTO
						sq_ast_lnk
						(
							linkid,
							majorid,
							minorid,
							link_type,
							value,
							sort_order,
							is_dependant,
							is_exclusive,
							updated,
							updated_userid
						)
						VALUES
						(
							:linkid,
							:majorid,
							:minorid,
							:link_type,
							:value,
							:sort_order,
							:is_dependant,
							:is_exclusive,
							:updated,
							:updated_userid
						)';


			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'linkid',                1,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'majorid',               0,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'minorid',               1,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'link_type',             SQ_LINK_TYPE_1, PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'value',                 '', PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'sort_order',            0,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'is_dependant',          1,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'is_exclusive',          1,  PDO::PARAM_INT);
				MatrixDAL::bindValueToPdo($query, 'updated',               ts_iso8601(time()), PDO::PARAM_STR);
				MatrixDAL::bindValueToPdo($query, 'updated_userid',        0,  PDO::PARAM_STR);
				// TODO: turn this into a PDO query
				MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('Unable to create root folder\'s link #1 entry due to database error: '.$e->getMessage());
			}


			$sql = 'INSERT INTO
						sq_ast_lnk_tree
						(
							treeid,
							linkid,
							num_kids
						)
						VALUES
						(
							\'-\',
							1,
							0
						)';

			try {
				// TODO: turn this into a PDO query
				MatrixDAL::executeSql($sql);
			} catch (Exception $e) {
				throw new Exception('Unable to create root folder\'s link tree entry due to database error: '.$e->getMessage());
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

			if (!system_asset_fns_create_cleanup($this)) {
				return FALSE;
			}
		}//end if

		unset($this->_tmp[__CLASS__.'_in_create']);
		return $ret_val;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		return ($short_name) ? '/' : translate('Root Folder');


	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a root folder
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess(Array $assetids=Array())
	{
		return TRUE;

	}//end readAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has the passed permission
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
	function _checkPermissionAccess($perm, $assetids=Array(), $only_workflow=TRUE)
	{
		if (empty($assetids)) {
			if ($GLOBALS['SQ_SYSTEM']->userRoot()) return TRUE;
			if ($GLOBALS['SQ_SYSTEM']->userSystemAdmin()) {
				return TRUE;
			}
			return FALSE;
		} else {
			return parent::_checkPermissionAccess($perm, $assetids, $only_workflow);
		}

	}//end _checkPermissionAccess()


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $filter=NULL)
	{
		if ($name == 'name' && empty($this->_tmp[__CLASS__.'_in_create'])) {
			trigger_localised_error('CORE0112', E_USER_WARNING);
			return FALSE;
		}

		return parent::setAttrValue($name, $value, $filter);

	}//end setAttrValue()


}//end class

?>
