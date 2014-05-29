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
* $Id: file_bridge.inc,v 1.6 2012/08/30 00:58:36 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* File_Bridge
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class File_Bridge extends Asset implements Bridge
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function File_Bridge($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Pages add a default web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

	}//end _createAdditional()


	/**
	 * Returns the list of files and directories under a particular path
	 *
	 * @param string	$path	The full path to the file
	 *
	 * @returns array
	 * @access private
	 */
	private function _listFiles($path)
	{

		$list = Array();

		$dirs  = list_dirs($path, TRUE, Array('CVS', '.svn'));
		$files = list_files($path, TRUE);

		sort($dirs);
		sort($files);

		$temp = array_merge($dirs, $files);
		$list = Array();

		// Cleanup by removing hidden files
		foreach ($temp as $element) {
			$filename = basename($element);
			if (strpos($filename, '.') !== 0) {
				$list[] = $element;
			}//end if
		}//end foreach
		
		return $list;

	}//end _listFiles()


	/**
	 * Returns if this file is a valid file or not
	 *
	 * @param string	$path	The full path to the file
	 *
	 * @returns boolean
	 * @access private
	 */
	private function _isValidFile($path)
	{
		if (file_exists($path) && is_readable($path)) {
			return TRUE;
		}//end if

		return FALSE;

	}//end _isValidFile()


//--        BRIDGE INTERFACE        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	* (Asset or NULL on error)
	*
	* @param int		$assetid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// The shadow id is the path from the location of the bridge
		require_once (SQ_DATA_PATH.'/private/conf/file_bridge.inc');
		$location = FILE_BRIDGE_PATH;
		$location = rtrim($location, '/');
		if ($this->_isValidFile($location.'/'.$shadowid)) {
			if (is_dir($location.'/'.$shadowid)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('physical_folder');
				$asset = new Physical_Folder($this->id.':'.$shadowid);
			} else {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('physical_file');
				$asset = new Physical_File($this->id.':'.$shadowid);
			}//end if
		}//end if

		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			the shadow part asset id to be loaded (i.e part after the ':') of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
	{
		// only TYPE_2 links are ever returned
		if (!($link_types & SQ_LINK_TYPE_2)) {
			return Array();
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link])) {

			$links = Array();
			// if we are looking from this asset and not a shadow asset
			if ($this->id == $assetid) {
				if ($side_of_link == 'minor') {
					// should never happen...but anyway
					trigger_localised_error('CORE0089', E_USER_ERROR);

				} else {
					require_once (SQ_DATA_PATH.'/private/conf/file_bridge.inc');
					$location = FILE_BRIDGE_PATH;
					$location = rtrim($location, '/');
					$files = $this->_listFiles($location);
					$i = 0;
					foreach ($files as $file) {
						$varid = str_replace($location.'/', '', $file);
						$type_code = (is_dir($file)) ? 'physical_folder' : 'physical_file';
						$links[] = Array(
									'linkid'			=> 0,
									'minorid'			=> $this->id.':'.$varid,
									'minor_type_code'	=> $type_code,
									'majorid'			=> $this->id,
									'value'				=> '',
									'link_type'			=> SQ_LINK_TYPE_2,
									'is_dependant'		=> FALSE,
									'is_exclusive'		=> FALSE,
									'sort_order'		=> $i,
								   );
						$i++;
					}// end foreach

				}// endif


			// looking from shadow asset
			} else {
				list($bridgeid,$shadowid) = explode(':', $assetid);
				require_once (SQ_DATA_PATH.'/private/conf/file_bridge.inc');
				$location = FILE_BRIDGE_PATH;
				$location = rtrim($location, '/');

				// The file is invalid why proceed?
				if (!$this->_isValidFile($location.'/'.$shadowid)) {
					return Array();
				}//end if

				// because the varieties are only ever going to be one level down
				// if we are looking up the tree all we can see is this bridge asset
				if ($side_of_link == 'minor') {
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'major_type_code'	=> $this->type(),
								'value'				=> '',
								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> TRUE,
								'sort_order'		=> 0,
							   );
				} else {
					// Looking down the tree
					if (is_dir($location.'/'.$shadowid)) {
						$files = $this->_listFiles($location.'/'.$shadowid);
						$i = 0;
						foreach ($files as $file) {
							$varid = str_replace($location.'/', '', $file);
							$type_code = (is_dir($file)) ? 'physical_folder' : 'physical_file';
							$links[] = Array(
										'linkid'			=> 0,
										'minorid'			=> $this->id.':'.$varid,
										'minor_type_code'	=> $type_code,
										'majorid'			=> $this->id,
										'value'				=> '',
										'link_type'			=> SQ_LINK_TYPE_2,
										'is_dependant'		=> TRUE,
										'is_exclusive'		=> TRUE,
										'sort_order'		=> $i,
									   );
							$i++;
						}// end foreach
					} else {
						// Just a file, nothing underneath
						return Array();
					}//end if
				}//endif

			}//end else - looking from shadow asset

			$sort_by = str_replace("'", '', $sort_by);
			if (!is_null($sort_by)) {
				uasort($links, create_function('$a,$b', 'return $a[\''.$sort_by.'\'] < $b[\''.$sort_by.'\'];'));
			}

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}//end if

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	* Returns an array of assetids and their type code
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	public function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		// now that we have an array in the format we wanted (sorted or not)
		// we can go through and construct our children array
		$children = Array();
		$links = $this->getLinks($assetid, SQ_LINK_TYPE_2, $type_code, $strict_type_code, 'major', NULL);
		if (!empty($links)) {
			foreach ($links as $link) {
				$children[$link['minorid']][0]['type_code'] = $link['minor_type_code'];
			}
		}
		return $children;

	}//end getChildren()

	
	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>form_section</i> or <i>form</i>
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Form_Section', 'Form_Question', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		$id_parts = explode(':', $assetid);
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($id_parts[0]);
		// after getting its own parent, the bridge adds itself as a parent into the array as the shadow asset is its child
		$parents[$this->id] = $this->type();
		return $parents;

	}//end getParents()


	/**
	* Prints the body of the asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{

	}//end printBody()


	/**
	* Return the keywords for this asset type.
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		return parent::getKeywordReplacement($keyword);

	}//end getKeywordReplacement()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the url to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : NULL = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
	{
		// All users underneath this bridge inherit the permissions of the bridge
		return $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $permission, $granted, $and_greater, $expand_groups, $all_info);

	}//end getPermission()


	/**
	* Sets a permission for the passed user or user_group
	*
	* @param int		$assetid	the assetid for asset whose permissions to set
	* @param int		$userid		the id of the user or user_group to add the permission for
	* @param string		$permission	the permission code you are adding
	* @param boolean	$granted	whether this permission grants access or not
	*
	* @return boolean
	* @access public
	*/
	function setPermission($assetid, $userid, $permission, $granted)
	{
		return FALSE;

	}//end setPermission()


	/**
	* Deletes a permission with the passed user or user_group
	*
	* @param int	$assetid	the assetid for asset whose permissions to delete
	* @param int	$userid		the id of the user or user_group to remove the permission from
	* @param string	$permission	the permission code you are deleting
	*
	* @return boolean
	* @access public
	*/
	function deletePermission($assetid, $userid, $permission)
	{
		return FALSE;

	}//end deletePermission()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
	{
		$info = Array();
		foreach ($assetids as $assetid) {
			$asset = $this->getAsset($assetid);

			if (!empty($field)) {
				if ($field == 'assetid') {
					$info[$assetid] = $assetid;
				} else if ($field == 'type_code') {
					$info[$assetid] = $asset->type();
				} else if (in_array($field , Array('version', 'name', 'short_name', 'status'))) {
					$info[$assetid] = $asset->$field;
				} else {
					$info[$assetid] = $this->$field;
				}
			} else {
				$info[$assetid] = Array(
									'assetid'				=> $assetid,
									'type_code'				=> $asset->type(),
									'version'				=> $asset->version,
									'name'					=> $asset->name,
									'short_name'			=> $asset->short_name,
									'status'				=> $asset->status,
									'languages'				=> $asset->languages,
									'charset'				=> $asset->charset,
									'force_secure'			=> $this->force_secure,
									'created'				=> $asset->created,
									'created_userid'		=> $asset->created_userid,
									'updated'				=> $asset->updated,
									'updated_userid'		=> $asset->updated_userid,
									'published'				=> $asset->published,
									'published_userid'		=> $asset->published_userid,
									'status_changed'		=> $asset->status_changed,
									'status_changed_userid'	=> $asset->status_changed_userid,
								  );
			}

		}//end foreach

		return $info;

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* Note that this will always return FALSE if you pass it a shadow asset
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		return FALSE;

	}//end assetExists()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		return Array();

	}//end getLink()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the link were returning
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		return Array();

	}//end getLinkById()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
	{
		return 0;

	}//end countLinks()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not NULL)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		return Array();

	}//end getLinkByAsset()


	/**
	* Get details of the links from the specified asset to all its children
	*
	* @param string	$assetid	the assetid of the asset to find the child links for
	* @param int	$link_type	the type of links to get
	*
	* @return array
	* @access public
	*/
	function getAllChildLinks($assetid, $link_type=0)
	{
		return Array();

	}//end getAllChildLinks()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid		the link id of the link to update
	* @param int	$link_type	the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value		the value to place on the link
	* @param int	$sort_order	the position in the links list that this link should take,
	*							if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=NULL, $value=NULL, $sort_order=NULL)
	{
		return FALSE;

	}//end updateLink()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		return FALSE;

	}//end deleteAssetLink()


	/**
	* Create a link between the two supplied assets
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if NULL or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return int
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		return 0;

	}//end createAssetLink()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


}//end class

?>
