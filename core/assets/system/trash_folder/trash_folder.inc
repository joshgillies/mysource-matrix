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


	/*
	* For a normal Asset returns all links to this asset, but for trash returns all the assets who are not minor's
	* in exclusive or unite links to any other asset
	*
	* @param int    $link_types  will only ever be SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE or bitmask that includes these
	* @param string $asset_type  the type of asset that is linked (eg 'Page', 'File', etc);
	* @param string $ret_type    flag to the structure of the return array - only relevant when $asset_type is blank
	*                            'T' = sorted/structured by type 
	*                                  eg Array('folder' => array(link1, link2, ..), 'user' => Array(link3, link4, ..))
	*                            'S' = sorted by name, single array 
	*                                  eg Array(link1, link2, ..)
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $asset_type='', $ret_type='T') {

		// because we are only collecting assets with no SQ_LINK_EXCLUSIVE or SQ_LINK_UNITE links
		// if the requested type doesn't contain these piss off
		if (($link_types & (SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE)) == 0) return Array();

		include_once SQ_LIB_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		// get all assets that have no EXCLUSIVE or UNITE links to any parents
		$sql = 'SELECT a.assetid as minorid, a.type_code
				FROM sq_asset a 
				WHERE a.assetid != 1
				  AND a.assetid NOT IN (~SQ0~) '
				.(empty($asset_type) ? '' : ' AND a.type_code = '.$db->quote($asset_type))
				.' ORDER BY a.name';
		$subs = Array( 'SELECT l.minorid
						FROM sq_asset_link l
						WHERE (l.link_type & '.$db->quote(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE).') > 0');

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) 
			trigger_error($sql->getMessage().'<br>'.$sql->getUserInfo(), E_USER_ERROR);

#		pre_echo($sql);
		error_log("\n".$sql);

		$result = $db->query($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);


		$ret_arr = Array();
		if ($ret_type == 'T') {
			while (NULL !== ($row = $result->fetchRow())) {
				if (!isset($ret_arr[$row['type_code']])) $ret_arr[$row['type_code']] = Array();
				$ret_arr[$row['type_code']][] = $row;
			}//end while
		} else {
			while (NULL !== ($row = $result->fetchRow())) {
				$ret_arr[] = $row;
			}//end while
		}

		$result->free();
		return $ret_arr;

	}//end getLinks()


	/*
	* Returns the number of links from this asset, see getLinks() for more info about why 
	* Trash_Folder needs to override default
	*
	* @param int    $link_types  integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string $asset_type  the type of asset that is linked (eg 'Page', 'File', etc);
	*
	* @return int
	* @access public
	*/
	function countLinks($link_types, $asset_type='') 
	{

		// because we are only collecting assets with no SQ_LINK_EXCLUSIVE or SQ_LINK_UNITE links
		// if the requested type doesn't contain these piss off
		if (($link_types & (SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE)) == 0) return Array();

		include_once SQ_LIB_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_RESOLVE']->getDb();

		// get all assets that have no EXCLUSIVE or UNITE links to any parents
		$sql = 'SELECT COUNT(*)
				FROM sq_asset a 
				WHERE a.assetid != 1
				  AND a.assetid NOT IN (~SQ0~) '
				.(empty($asset_type) ? '' : ' AND a.type_code = '.$db->quote($asset_type));
		$subs = Array( 'SELECT l.minorid
						FROM sq_asset_link l
						WHERE (l.link_type & '.$db->quote(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE).') > 0');

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) 
			trigger_error($sql->getMessage().'<br>'.$sql->getUserInfo(), E_USER_ERROR);

		error_log($sql);

		$result = $db->getOne($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br>'.$result->getUserInfo(), E_USER_ERROR);
		
		return $result;

	}//end countLinks()	

	/*
	* In normal asset's this creates a link between this asset and the passed asset
	* In the trash no links are created, but the minor asset is checked to see if it has any
	* unite or exclusive links
	*
	* @param object Asset $minor  the minor asset that we are linking to
	* @param string $link_type    NOT USED
	* @param string $value        NOT USED
	* @param string $sort_order   NOT USED
	*
	* @see Asset::createLink()
	*
	* @return boolean
	* @access public
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1) {

		if (!is_a($minor, 'Asset')) {
			trigger_error('Minor is not an asset, unable to create link', E_USER_WARNING);
			return false;
		}

		// does the minor still have links ?
		if ($minor->countLinks(SQ_LINK_EXCLUSIVE | SQ_LINK_UNITE) > 0) {
			return false;

		} else {
			return true;

		}// end if

	}// end createLink();


}#end class
?>
