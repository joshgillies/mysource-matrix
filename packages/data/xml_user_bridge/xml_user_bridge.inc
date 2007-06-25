<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: xml_user_bridge.inc,v 1.2 2007/06/25 01:58:25 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**XML User Bridge
*
* Purpose
* 	Exposes shadow user assets representing the data in an XML file
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage data
*/

register_implementation('xml_user_bridge', 'bridge');

class XML_User_Bridge extends Bridge
{

	var $groups = NULL;
	var $group_members = NULL;
	var $user_memberships = NULL;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	* @return object
	* @access public
	*/
	function XML_User_Bridge($assetid=0)
	{
		$res = $this->Bridge($assetid);
		$this->loadFromFile();
		return $res;

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	If TRUE, the short name should be returned.
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Load the user data from the XML file, refreshing the file if it's too old
	*
	* @return void
	* @access public
	*/
	function loadFromFile()
	{
		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$filename = $this->data_path.'/contents.xml';

		if (!file_exists($filename) || (filemtime($filename) < strtotime('-'.$this->attr('refresh_time').' seconds'))) {
			@unlink($filename);
			if ($this->attr('data_url')) {
				$xml = @file_get_contents($this->attr('data_url'));
				if ($xml) {
					if (!is_dir($this->data_path)) {
						create_directory($this->data_path);
					}
					string_to_file($xml, $filename);
				}
			}
		}
		if (!file_exists($filename)) return;

		$input =& new XML_Tree($filename);
		$root  =& $input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Could not parse XML', E_USER_WARNING);
		}

		$this->groups = Array();
		$this->group_members = Array();
		$this->user_memberships = Array();
		foreach ($root->children as $group_node) {
			if ($group_node->name != 'group') continue;
			$groupid = $group_node->attributes['id'];
			$this->groups[$groupid] = $group_node->attributes['name'];
			foreach ($group_node->children as $member_node) {
				$this->group_members[$groupid][$member_node->attributes['id']] = $member_node->attributes;
				$this->user_memberships[$member_node->attributes['id']][] = $groupid;
			}
		}

	}//end loadFromFile()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($assetid, $type_code='', $mute_errors=FALSE)
	{
		$bits = explode(':', $assetid);
		switch (count($bits)) {
			case 1:
				return $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code, $mute_errors);
			case 2:
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('user_group');
				$shadowid = $bits[1];
				$ug =& new User_Group();
				$ug->id = $assetid;
				$ug->name = $this->groups[$shadowid];
				$ug->short_name = $this->groups[$shadowid];
				$ug->status = $this->status;
				$ug->setAttrValue('name', $this->groups[$shadowid]);
				return $ug;
			case 3:
				$groupid = $bits[1];
				$personid = $bits[2];
				$user_type = $this->attr('user_type');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
				$user =& new $user_type();
				$user->id = $assetid;
				$user->status = $this->status;
				if (isset($this->group_members[$groupid][$personid])) {
					foreach ($this->group_members[$groupid][$personid] as $field => $val) {
						if (isset($user->vars[$field])) {
							$user->setAttrValue($field, $val);
						}
					}
				}
				$user->name = $user->_getName();
				$user->short_name = $user->_getName(TRUE);
				return $user;
		}//end switch

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* The bulk user system only deals with significant, non-dependant and non-exclusive links.  Sorting is ignored.
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	* 										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	* 										(eg 'Page', 'File', etc)
	* 										if an array returns link if matches any
	* 										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	* 										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	* 										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	* 										sorted by (eg. name, short_name etc)
	* @param boolean	$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean	$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		// We don't have any dependant or exclusive links
		if ($dependant || $exclusive) {
			return Array();
		}

		// We only have type 1 links
		if (! $link_types && SQ_LINK_TYPE_1) {
			return Array();
		}

		$bits = explode(':', $assetid);
		switch (count($bits)) {
			case 1:
				if (($assetid == $this->id) && ($side_of_link == 'major')) {
					// Get the groups under this asset
					$sample = Array(
								'majorid'			=> $this->id,
								'link_type'			=> SQ_LINK_TYPE_1,
								'major_type_code'	=> 'xml_user_bridge',
								'minor_type_code'	=> 'user_group',
								'sort_order'		=> -1,
							  );
					$res = Array();
					foreach ($this->groups as $id => $name) {
						$sample['minorid'] = $this->id.':'.$id;
						$res[$this->id.':'.$id] = $sample;
					}
					return $res;
				} else {
					return $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, $link_types, $type_code, $strict_type_code, $side_of_link, $sort_by, $dependant, $exclusive);
				}
			break;
			case 2:
				if ($side_of_link == 'major') {
					// Get the users under this group
					$sample = Array(
								'majorid'			=> $assetid,
								'link_type'			=> SQ_LINK_TYPE_1,
								'major_type_code'	=> 'user_group',
								'minor_type_code'	=> $this->attr('user_type'),
								'sort_order'		=> -1,
							  );
					$res = Array();
					foreach ($this->group_members[$bits[1]] as $userid => $details) {
						$sample['minorid'] = $assetid.':'.$userid;
						$res[$assetid.':'.$userid] = $sample;
					}
					return $res;

				} else {
					// Just this
					return Array(
							'minorid'			=> $assetid,
							'majorid'			=> $this->id,
							'link_type'			=> SQ_LINK_TYPE_1,
							'major_type_code'	=> 'xml_user_bridge',
							'minor_type_code'	=> 'user_group',
							'sort_order'		=> -1,
						   );
				}
			break;
			case 3:
				if ($side_of_link == 'major') {
					return Array();
				} else {
					// Get the groups this user belongs to
					$sample = Array(
								'minorid'			=> $assetid,
								'link_type'			=> SQ_LINK_TYPE_1,
								'major_type_code'	=> 'user_group',
								'minor_type_code'	=> $this->attr('user_type'),
								'sort_order'		=> -1,
							  );
					$res = Array();
					foreach ($this->user_memberships[$bits[2]] as $groupid) {
						$sample['majorid'] = $this->id.':'.$groupid;
						$res[$this->id.':'.$groupid.':'.$bits[2]] = $sample;
					}

				}
			break;
		}//end switch


	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'bulkmail_user', 'User_Group', etc)
	*										if an array returns link if matches any of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		$bits = explode(':', $assetid);
		$res = Array();
		switch (count($bits)) {
			case 1:
				$res = $GLOBALS['SQ_SYSTEM']->am->getParents($assetid, $type_code, $strict_type_code);
			break;
			case 2:
				if (empty($type_code) || ($type_code == 'xml_user_bridge') || (!$strict_type_code && ($type_code == 'bridge'))) {
					$res = Array($this->id => 'xml_user_bridge');
				}
			break;
			case 3:
				if (empty($type_code) || ($type_code == 'user_group') || (!$strict_type_code && ($type_code == 'folder'))) {
					foreach ($this->user_memberships[$bits[2]] as $groupid) {
						$res[$groupid] = 'user_group';
					}
				}
			break;
		}
		return $res;

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		$bits = explode(':', $assetid);
		switch (count($bits)) {
			case 1:
				if ($assetid == $this->id) {
					foreach ($this->group_members as $groupid => $members) {
						$res[$this->id.':'.$groupid] = 'user_group';
					}
					return $res;
				} else {
					return $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL);
				}
			break;
			case 2:
				$res = Array();
				foreach (array_get_index($this->group_members, $bits[1], Array()) as $memberid => $details) {
					$res[$assetid.':'.$memberid] = 'bulkmail_user';
				}
				return $res;
			break;
			case 3:
				return Array();
			break;
		}

	}//end getChildren()


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
		// There are no individual links of note in this sytem
		return NULL;

	}//end getLink()


	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param object	&$major		the major asset in the proposed link
	* @param object	&$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @protected
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		return FALSE;

	}//end canCreateLink()


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
		assert_type($assetids, 'array');
		if (empty($assetids)) return Array();
		if (!is_array($type_code)) {
			$type_code = Array($type_code);
		}
		$template = reset($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($this->id));

		$res = Array();
		$available_types = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($this->attr('user_type'));
		$available_types[] = $this->attr('user_type');
		if (empty($type_code) || array_intersect($available_types, $type_code)) {
			// Look through the users
			foreach ($assetids as $assetid) {
				$asset =& $this->getAsset($assetid, $this->attr('user_type'));
				if (empty($field)) {
					$entry = $template;
					foreach ($template as $i => $v) {
						if ($i == 'type_code') continue;
						$entry[$i] = $asset->$i;
					}
				} else {
					$entry = $asset->$field;
				}
				$res[$assetid] = $entry;
			}
		}

		if (empty($type_code) || in_array('user_group', $type_code)) {
			// Look through the groups
			foreach ($assetids as $id) {
				$bits = explode(':', $id);
				if (count($bits) == 2) {
					$details = array_get_index($this->groups, $bits[1]);
					if ($details) {
						$entry = $template;
						$entry['name'] = $details['name'];
						$entry['short_name'] = $details['name'];
						$entry['type_code'] = 'user_group';
						if (empty($field)) {
							$res[$id] = $entry;
						} else {
							$res[$id] = array_get_index($entry, $field);
						}
					}
				}
			}
		}

		return $res;

	}//end getAssetInfo()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* @param mixed	$assetids	the ID of the asset we are going to the check for existence
	*							or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{
		if (!is_array($assetids)) {
			$my_assetids = Array($assetids);
		} else {
			$my_assetids = $assetids;
		}
		foreach ($my_assetids as $assetid) {
			$bits = explode(':', $assetid);
			switch (count($bits)) {
				case 1:
					$res[$assetid] = ($assetid == $this->id);
				break;
				case 2:
					$res[$assetid] = isset($this->groups[$bits[1]]);
				break;
				case 3:
					$res[$assetid] = isset($this->groups[$bits[1]]) && isset($this->group_members[$bits[1]][$bits[2]]);
				break;
			}
		}
		return (is_array($assetids)) ? $res : $res[$assetids];

	}//end assetExists()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of userids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are false)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $permission, $granted, $and_greater, $expand_groups, $all_info);

	}//end getPermission()


}//end class
?>