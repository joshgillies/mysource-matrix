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
* @package Resolve::packages::__core__
*/
class Trash_Folder extends Folder
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

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another Trash Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}
		
		$ret_val = parent::create('Trash');
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
		trigger_error('Unable to duplicate Trash Folder, you are only allowed one', E_USER_WARNING);
		$null = NULL; // 'cause we need to return by reference :)
		return $null;
	}

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
	}

	/*
	* Set the value for the passed variable to the passed value
	*
	* @access public
	*/
	function setAttrValue($name, $value) {

		if ($name == 'name' && !$this->_in_create) {
			trigger_error('You are not able to alter the name for the trash folder', E_USER_WARNING);
			return false;
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

	/*
	* Returns the link information for a specific link that this asset is the major asset in
	* 
	* @param int	linkid
	*
	* @see Asset::getLink()
	*
	* @return Array
	* @access public
	*/
	function getLinkById($linkid)
	{

		$linkid = (int) $linkid;

		if ($linkid >= 0) return Array();

		$assetid = abs($linkid);
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return Array();

		return Array(
					'linkid'     => $linkid, 
					'majorid'    => $this->id, 
					'minorid'    => $asset->id, 
					'link_type'  => SQ_LINK_UNITE,
					'value'      => '',
					'sort_order' => 0					
					);

			
	}//end getLink()

	/*
	* For a normal Asset returns all links to this asset, but for trash returns all the assets who are not minor's
	* in significant links to any other asset
	*
	* @param int    $link_types			will only ever be a bitmask that matches SQ_LINK_SIGNIFICANT
	* @param string $type_code			the type of asset that is linked (eg 'Page', 'File', etc);
	* @param bool	$strict_type_code	whether we are finding assets that are just a $type_code or $type_code and any of it's sub-classes
	* @param string $side_of_link		Which side of the link this asset is on ('major' or 'minor')
	*
	* @see Asset::getLinks()
	*
	* @return Array()
	* @access public
	*/
	function getLinks($link_types, $type_code='', $strict_type_code=true, $side_of_link='major') 
	{
		if ($side_of_link != 'major' && $side_of_link != 'minor') 
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		// if we are after the side that we are the minor party in, then call Asset::getLinks() like normal
		if ($side_of_link == 'minor') {
			return parent::getLinks($link_types, $type_code, $strict_type_code, $side_of_link);
		}

		// because we are only collecting assets with no EXCLUSIVE, UNITE or USES links
		// if the requested type doesn't contain these piss off
		if (($link_types & SQ_LINK_SIGNIFICANT) == 0) return Array();

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all assets that have no EXCLUSIVE or UNITE links to any parents
		$sql = 'SELECT a.assetid as minorid, a.type_code
				FROM sq_asset a 
				WHERE a.assetid != 1
				  AND a.assetid NOT IN (~SQ0~) ';

		$subs = Array( 'SELECT l.minorid
						FROM sq_asset_link l
						WHERE (l.link_type & '.$db->quote(SQ_LINK_SIGNIFICANT).') > 0');

		if ($type_code) {
			if ($strict_type_code) {
				$sql .= ' AND a.type_code = '.$db->quote($type_code);
			} else {
				$sql .= ' AND (a.type_code = '.$db->quote($type_code).' OR a.type_code IN (~SQ1~))';
				$subs[] = 'SELECT type_code
							FROM sq_asset_type_inherited
							WHERE inherited_type_code = '.$db->quote($type_code);
			}
		}// end if

		$sql .= ' ORDER BY a.name';

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) 
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);

#		pre_echo($sql);
#		error_log("\n".$sql);

		$result = $db->getAll($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		// create the extra fields
		for($i = 0, $count = count($result); $i < $count; $i++) {
			// make the link id the negative of the assetid as this will be unique in the trash anyway
			$result[$i]['linkid']     =  -1 * (int) $result[$i]['minorid']; 
			$result[$i]['link_type']  = SQ_LINK_UNITE;
			$result[$i]['value']      = '';
			$result[$i]['sort_order'] = $i;
		}//end while

		return $result;

	}//end getLinks()


	/*
	* Returns the number of links from this asset, see getLinks() for more info about why 
	* Trash_Folder needs to override default
	*
	* @param string $side_of_link	Which side of the link this (the current) asset is on ('major' or 'minor')
	* @param int	$link_types		integer that can be the product of bitwise operations on the SQ_LINK_* constants
	* @param string	$type_code		the type of asset that is linked (eg 'Page', 'File', etc);
	*
	* @return int
	* @access public
	*/
	function countLinks($side_of_link, $link_types, $type_code='') 
	{

		if ($side_of_link != 'major' && $side_of_link != 'minor') 
			trigger_error('Unknown Side of Link "'.$side_of_link.'"', E_USER_ERROR);

		// if we are after the side that we are the minor party in, then call Asset::getLinks() like normal
		if ($side_of_link == 'minor') {
			return parent::countLinks($side_of_link, $link_types, $type_code);
		}

		// because we are only collecting assets with no SQ_LINK_EXCLUSIVE or SQ_LINK_UNITE links
		// if the requested type doesn't contain these piss off
		if (($link_types & (SQ_LINK_NAV)) == 0) return Array();

		require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get all assets that have no EXCLUSIVE or UNITE links to any parents
		$sql = 'SELECT COUNT(*)
				FROM sq_asset a 
				WHERE a.assetid != 1
				  AND a.assetid NOT IN (~SQ0~) '
				.(empty($type_code) ? '' : ' AND a.type_code = '.$db->quote($type_code));
		$subs = Array( 'SELECT l.minorid
						FROM sq_asset_link l
						WHERE (l.link_type & '.$db->quote(SQ_LINK_NAV).') > 0');

		$sql = db_extras_subquery($db, $sql, $subs);
		if (DB::isError($sql)) 
			trigger_error($sql->getMessage().'<br/>'.$sql->getUserInfo(), E_USER_ERROR);

#		error_log($sql);

		$result = $db->getOne($sql);
		if (DB::isError($result)) 
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
		
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

		return true;

	}// end createLink();

	/*
	* Checks whether a link of a certain type can be made to this asset
	* Returns TRUE if link allowed and a string containing the error if it is not
	*
	* @see Asset::canLinkToType()
	*
	* @return boolean
	* @access public
	*/
	function canLinkToType($type_code, $link_type) 
	{
		return 'The Trash Folder cannot have assets linked to it. The Trash holds all assets that are not linked to any other asset';
	}// end canLinkToType()


	/*
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string $sort_order	the position in the links list that this link should take, if less than zero places at end of list
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


	/*
	* In normal asset's this deletes a link between this asset on the passed linkid
	* In the trash no links are created, so none can be deleted, 
	* the linkid is checked to see if it is negative (ie from getLinks() above)
	* and if the asset now has a link, true is returned
	*
	* Removes a link where this asset is the major player
	*
	* @param int	$linkid		the link id of the link to remove
	*
	* @see Asset::getLinks()
	* @see Asset::deleteLink()
	*
	* @return boolean
	* @access public
	*/
	function deleteLink($linkid) {

		$linkid = (int) $linkid;

		if ($linkid >= 0) {
			trigger_error('Existing Link #'.$linkid.' not found, unable to delete link', E_USER_WARNING);
			return false;
		}

		$assetid = abs($linkid);
		$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			trigger_error('Linked asset could not be found, unable to delete link', E_USER_WARNING);
			return false;
		}


		// is the asset linked to any thing
		return ($asset->countLinks('minor', SQ_LINK_NAV) > 0);

	}// end deleteLink();

}//end class
?>
