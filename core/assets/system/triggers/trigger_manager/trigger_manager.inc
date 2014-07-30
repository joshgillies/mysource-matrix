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
* $Id: trigger_manager.inc,v 1.76 2013/10/11 06:52:45 cupreti Exp $
*
*/

// determines if tree_location refers to all assets under itself, or itself only
define('SQ_TRIG_TREE_PROPAGATE',   '01');
define('SQ_TRIG_TREE_NO_PROPAGATE','00');

// trigger status
define('SQ_TRIG_STATUS_ACTIVE',    '1');
define('SQ_TRIG_STATUS_INACTIVE',  '0');
define('SQ_TRIG_STATUS_DEFAULT',   SQ_TRIG_STATUS_INACTIVE);

// used to determine if a trigger has failed, and if so, how
define('SQ_TRIG_RESULT_SUCCESS',  1);
define('SQ_TRIG_RESULT_FALSE',   -1);
define('SQ_TRIG_RESULT_INVALID', -2);
define('SQ_TRIG_RESULT_FAILURE',  0);

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once 'hash.inc';

register_implementation('trigger_manager', 'bridge');

/**
* Trigger_Manager
*
* Purpose
*    Manages the triggering system
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.76 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Manager extends Asset implements Bridge
{

	// Triggers will be stored in a array structure as category hierarchy
	// e.g $_trigger_tree['category1']['category2'][0] represents first trigger in category1/category2
	var $_trigger_tree;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @access public
	*
	*/
	function Trigger_Manager($assetid=0)
	{
		$this->Asset($assetid);

		// include base classes for conditions, actions, events
		$this->_loadComponent('trigger_condition');
		$this->_loadComponent('trigger_action');
		$this->_loadComponent('trigger_event');

	}//end constructor
	

	/**
	* Wakeup.
	*
	* It's important to load components when restored from a serialised
	* state by Deja Vu.
	*
	* @access public
	*/
	function __wakeup() 
	{
		// include base classes for conditions, actions, events
		$this->_loadComponent('trigger_condition');
		$this->_loadComponent('trigger_action');
		$this->_loadComponent('trigger_event');
		
	}//end __wakeup()


	/**
	* Create this asset
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
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Return an array of all the permitted link types, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$tm_links = Array();

		$tm_links[SQ_LINK_TYPE_2]['trigger'] = Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   );

		return $tm_links;

	}//end _getAllowedLinks()


	/**
	* Return FALSE always because we don't want to be able to clone a trigger manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Return a name to use to describe this instance of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Returns the name of a component (an action or a condition)
	*
	* @param string	$type	component type_code
	*
	* @return string
	* @access private
	*/
	function _getComponentName($type)
	{
		$name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'name');
		return $name;

	}//end _getComponentName()


	/**
	* Returns the name of a component (an action or a condition)
	*
	* @param string	$type	component type_code
	*
	* @return string
	* @access private
	*/
	function _getComponentDescription($type)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type, 'description');

	}//end _getComponentDescription()


//--        BRIDGE FUNCTIONS         --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param mixed		$shadowid		(int | string) the asset id to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	*/
	function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{

		$asset = NULL;

		// we can only handle triggers
		if (empty($type_code)) $type_code = 'trigger';

		$id_parts = explode(':', $shadowid);
		$id_part = $id_parts[count($id_parts)-1];

		// create either a trigger or a folder
		if(is_numeric($id_part)){
			if (!empty($id_part) && $type_code == 'trigger') {
				$id = $id_part;

				// attempt to get trigger asset
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('trigger');
				$asset = new Trigger($this->id, $id);
				//set up status and display name
				$asset->short_name = ($asset->vars['active']['value'] == '1') ? $asset->vars['name']['value'] : $asset->vars['name']['value'].' (disabled)';
				$asset->name = ($asset->vars['active']['value'] == '1') ? $asset->vars['name']['value'] : $asset->vars['name']['value'].' (disabled)';
				$asset->status = ($asset->vars['active']['value'] == '1') ? SQ_STATUS_LIVE : SQ_STATUS_UNDER_CONSTRUCTION;

				// if no id, this trigger is bodge
				if (empty($asset->id)) $asset = NULL;
			}
		}
		else {
			// create a shadow folder
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('trigger_folder');
			$asset = new Trigger_Folder($shadowid);
			$asset->status = SQ_STATUS_UNDER_CONSTRUCTION;
			$asset->short_name = $id_part;

		}
		return $asset;

	}//end getAsset()


	/**
	* Creates a trigger tree from a list of triggers
	* Trigger tree is used to display category hierarchy in asset map
	* e.g $_trigger_tree['category1']['category2'][0] represents first trigger in category1/category2
	*
	* @param Array	$triggers	List of triggers to be processed
	*
	*
	* @return mixed Array|NULL
	* @access private
	*/
	function _createCategoryTriggerTree($triggers)
	{
		$results = Array();
		foreach ($triggers as $trigger) {

			$my_result = &$results;
			//category name must have right format, otherwise, it will be linked under trigger manager
			if(empty($trigger['category']) || !preg_match('/^[a-zA-Z][a-zA-Z0-9_\- ]*(\/[a-zA-Z][a-zA-Z0-9_\- ]*)*$/',$trigger['category'])){
				$categories = NULL;
			}
			else {
				$categories = explode('/',$trigger['category']);
				foreach ($categories as $i => $cate) {
					$my_result = &$my_result[$cate];
				}
			}

			$my_result [] = $trigger;
		}

		return $results;
	}// end _createCategoryTriggerTree ()


	/**
	* Locates the  branch in trigger tree for the specified category.
	* It will recursively look for the category in the tree.
	*
	* @param String	$assetid	Asset id which contains the category to be searched.
	* @param Array	$trigger_tree	Trigger tree array.
	*
	*
	* @return mixed Array|NULL
	* @access private
	*/
	function _findCategoryTriggerTree($assetid,$trigger_tree)
	{
		$id_parts = explode(':', $assetid);
		array_shift($id_parts);

		// For  every recursion, go deeper.
		$new_assetid = implode(':', $id_parts);
		if(count($id_parts) == 0 || !array_key_exists($id_parts[0],$trigger_tree)){
			return NULL;
		}
		else if(count($id_parts)==1) {
				return $trigger_tree[$id_parts[0]];
		}
		return $this->_findCategoryTriggerTree($new_assetid,$trigger_tree[$id_parts[0]]);
	}// end _findCategoryTriggerTree()



	/**
	* Returns all links in current level of trigger tree.
	* Links will be created, sorted and passed to links array.
	*
	* @param String	$parentid	Parent asset id which has all links in this level
	* @param Array	$trigger_tree	Trigger tree array.
	*
	*
	* @return mixed Array|NULL
	* @access private
	*/
	function _getTopTriggerAssetMapLink($parentid, $trigger_tree, &$links)
	{
		// temp array for links before sorting
		$links_unsorted = Array();
		// temp array for sorting
		$sort_index = Array();

		if (!isset($trigger_tree) || !is_array($trigger_tree)) return NULL;
			foreach ($trigger_tree as $cate => $trigger) {
				$link = Array();

				//if it's a trigger, create a trigger link
				if(is_numeric($cate)){

						$link['url'] = '';
						$link['path'] = '';
						$link['num_kids'] = 0;
						$link['accessible'] = 1;

						$link['majorid'] = $this->id;
						// link all triggers under trigger manager, it will help asset map to refresh properly
						$link['minorid'] = implode(':', Array($this->id, $trigger['id']));
						$link['assetid'] = implode(':', Array($this->id, $trigger['id']));
						$link['minor_type_code'] = 'trigger';
						$link['type_code'] = 'trigger';
						$link['linkid'] = implode(':', Array($this->id, $trigger['id']));

						// make name and short name the same
						$link['name'] = $trigger['name'];
						$link['short_name'] = $trigger['active'] ? $trigger['name'] : $trigger['name'].' (disabled)';
						$link['status'] = $trigger['active'] ? SQ_STATUS_LIVE : SQ_STATUS_UNDER_CONSTRUCTION;
						$link['sort_order'] = NULL;
						$link['link_type'] = SQ_LINK_TYPE_1;
						$link['locked'] = 0;
						$link['is_dependant'] = 0;

						$sort_index ['trigger'][] = $link['name'];
						$links_unsorted['trigger'][] = $link;
				}
				// if it's category, create a folder link
				else if(isset ($cate) && !empty($cate)) {

						$link['url'] = '';
						$link['path'] = '';
						$link['num_kids'] = -1;
						$link['accessible'] = 1;

						$link['majorid'] = $parentid;
						$link['minorid'] = implode(':', Array($parentid, $cate));
						$link['assetid'] = implode(':', Array($parentid, $cate));
						$link['minor_type_code'] = 'trigger_folder';
						$link['type_code'] = 'trigger_folder';

						$link['linkid'] = implode(':', Array($parentid, $cate));

						// make name and short name the same
						$link['name'] = $cate;
						$link['short_name'] = $cate;
						$link['status'] = SQ_STATUS_UNDER_CONSTRUCTION;
						$link['sort_order'] = NULL;
						$link['link_type'] = SQ_LINK_TYPE_1;
						$link['locked'] = 0;
						$link['is_dependant'] = 0;

						$sort_index ['folder'][] = $link['name'];
						$links_unsorted ['folder'][] = $link;

				}//end else if
			}// end foreach

			/*
			 * Sort links, folders come first, and then order by alphabet
			 */

			// make sure folder comes first
			if(!empty($sort_index)){
				ksort($sort_index);
			}



			// make sure everything is with alphabetic order
			foreach(array_keys($sort_index) as $type){
				asort($sort_index[$type]);
				foreach ($sort_index[$type] as $index => $name) {
					// now, insert it into links array
					$links[] = $links_unsorted[$type][$index];
				}
			}

	}// end _getTopTriggerAssetMapLink()




	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array|NULL
	* @access public
	*/
	function getAssetMapLinks()
	{
		// use existing trigger manager functions to get our triggers
		$linked_triggers = $this->getTriggerList();

		$links = Array();
		if(!isset($linked_triggers)) return NULL;

		//create trigger tree
		$this->_trigger_tree = $this->_createCategoryTriggerTree($linked_triggers);

		//get top level links from trigger tree
		$this->_getTopTriggerAssetMapLink($this->id, $this->_trigger_tree, $links);

		return $links;

	}//end getAssetMapLinks()


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
		$asset_info = Array();

		$id_parts = explode(':', $assetid);
		$id_part = $id_parts[count($id_parts)-1];
		if (empty($id_part)) return $asset_info;

		// if it's trigger, return info of this trigger
		if(is_numeric($id_part)){
			$trigger = $this->_loadTrigger($id_part);
			if (empty($trigger)) return $asset_info;

			$asset_info['assetid']		= $assetid;
			$asset_info['name']			= $trigger['active'] ? $trigger['name'] : $trigger['name'].' (disabled)';
			$asset_info['short_name']	= $trigger['active'] ? $trigger['name'] : $trigger['name'].' (disabled)';
			$asset_info['version']		= $this->version;
			$asset_info['status']		= $trigger['active'] ? SQ_STATUS_LIVE : SQ_STATUS_UNDER_CONSTRUCTION;
			$asset_info['type_code']	= 'trigger';
			$asset_info['num_kids']		= 0;
			$asset_info['accessible']	= 1;
			$asset_info['url']			= '';
			$asset_info['web_path']		= '';
		}
		// return info of a folder
		else {
			$asset_info['assetid']		= $id_part;
			$asset_info['name']			= $id_part;
			$asset_info['short_name']	= $id_part;

			$asset_info['status']		=  SQ_STATUS_UNDER_CONSTRUCTION;
			$asset_info['type_code']	= 'folder';
			//folders should always have at least one child
			$asset_info['num_kids']		= -1;
			$asset_info['accessible']	= 1;
			$asset_info['url']			= '';
			$asset_info['web_path']		= '';
		}
		return $asset_info;

	}//end getAssetMapAssetInfo()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	* @param boolean	$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean	$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		$id_parts = explode(':', $assetid);
		//get last part of id
		$id_part = $id_parts[count($id_parts)-1];
		// only folders can have children, not for triggers
		if (empty($id_part) || is_numeric($id_part)) return Array();

		$links = Array();

		// create trigger tree
		$linked_triggers = $this->getTriggerList();
		$this->_trigger_tree = $this->_createCategoryTriggerTree($linked_triggers);

		//get links in current level  in trigger tree
		$this->_getTopTriggerAssetMapLink($assetid, $this->_findCategoryTriggerTree($assetid,$this->_trigger_tree), $links);

		return $links;

	}//end getLinks()




	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>user_group</i>
	*
	* @param int		$assetid			the id of the asset to get its parents for
	* @param mixed		$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
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
		return Array();

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
		return Array();

	}//end getChildren()


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

		return Array();

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
		$info_array = Array();

		foreach ($assetids as $id) {

			$GLOBALS['SQ_SYSTEM']->am->setPermission($id, $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user'), SQ_PERMISSION_READ, 0, TRUE, TRUE);
			$id_parts = explode(':', $id);

			$id_part = $id_parts[count($id_parts)-1];
			if (is_null($id_part)) continue;
			// if asset is a trigger
			if(is_numeric($id_part)){

				// use existing load trigger function to get trigger info
				$trigger_info = $this->_loadTrigger($id_part);
				if (empty($trigger_info)) continue;

				$info_array[$id]['assetid']		= $id;
				$info_array[$id]['name']		= $trigger_info['name'];
				$info_array[$id]['short_name']	= $trigger_info['active'] ? $trigger_info['name'] : $trigger_info['name'].' (disabled)';
				$info_array[$id]['version']		= $this->version;
				$info_array[$id]['status']		= $this->status;
				$info_array[$id]['type_code']	= 'trigger';
				$info_array[$id]['num_kids']	= 0;
				$info_array[$id]['accessible']	= 1;
				$info_array[$id]['url']			= '';
				$info_array[$id]['web_path']	= '';

				if (!empty($field)) {
					$info_array[$id] = array_get_index($info_array[$id], $field);
				}
			}
			// or a folder
			else {
				$info_array[$id]['assetid']		= $id;
				$info_array[$id]['name']		= $id_part;
				$info_array[$id]['short_name']	= $id_part;
				$info_array[$id]['version']		= $this->version;
				$info_array[$id]['status']		= $this->status;
				$info_array[$id]['type_code']	= 'trigger_folder';
				$info_array[$id]['num_kids']	= -1;
				$info_array[$id]['force_secure']	=$this->force_secure;
				$info_array[$id]['accessible']	= 1;
				$info_array[$id]['url']			= '';
				$info_array[$id]['web_path']	= '';

			}//end else
		}//end foreach
		return $info_array;

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

		$link_parts = explode(':', $linkid);
		$trigger_id = $link_parts[count($link_parts)-1];
		if (is_null($trigger_id)) return NULL;

		//if it's a trigger, load trigger detail, return link info
		if(is_numeric($trigger_id)){
				$trigger_info = $this->_loadTrigger($trigger_id);

				// construct template
				$link = Array(
					'majorid'			=> $this->id,
					'minorid'			=> $trigger_id,
					'link_type'			=> SQ_LINK_TYPE_1,
					'major_type_code'	=> 'trigger_manager',
					'minor_type_code'	=> 'trigger',
					'value'				=> '',
					'linkid'			=> $this->id.':'.$trigger_id,
					'sort_order'		=> $trigger_info['id'],
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
					'locked'			=> 0,
				);

		}
		else {
				// get parent id
				foreach($link_parts as $node){
					$parent_parts [] = $node;
				}
				array_pop($parent_parts);
				$parent_id = implode(':',$parent_parts);


				$link = Array(
					'majorid'			=> $parent_id,
					'minorid'			=> $trigger_id,
					'link_type'			=> count($parent_parts) == 1 ? 'trigger_manager' : 'trigger_folder',
					'minor_type_code'	=> 'trigger_folder',
					'value'				=> '',
					'linkid'			=> $trigger_id,
					'sort_order'		=> NULL,
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
					'locked'			=> 0,
				);


		}

		return $link;

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
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
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
		// not concerned about the link id, just the trigger id
		// as triggers only have one link (with the manager)
		$link_parts = explode(':', $linkid);

		$trigger_id = $link_parts[count($link_parts)-1];
		if (is_null($trigger_id)) return FALSE;

		// use trigger manager function to delete the trigger
		if (is_numeric($trigger_id)) return $this->_deleteTrigger($trigger_id);



		// delete folder, that is deleting all triggers under this folder, not permitted
		trigger_localised_error('SYS0069', E_USER_WARNING, $trigger_id);

		return FALSE;

	}//end deleteAssetLink()





	/**
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param Asset	$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(Asset $minor, $link_type, $exclusive)
	{
		return FALSE;

	}//end canCreateLink()


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


//--        TRIGGER EXECUTION        --//


	/**
	* Broadcasts an event, and run all the triggers associated with it
	*
	* @param string	$event_name		the name of the event to broadcast
	* @param object	&$broadcaster	The firing asset
	* @param array	$parameters		extra parameters
	*
	* @return boolean
	* @access public
	*/
	function broadcastEvent($event_name, &$broadcaster, $parameters=NULL)
	{
		if (is_null($broadcaster)) {
			return FALSE;
		}

		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return TRUE;
		if (!$this->attr('enabled')) return TRUE;
		// We've been told not to do this right now
		if ($broadcaster->shouldFastTrack($event_name)) return FALSE;
		
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// include the event asset
		if (!$this->_loadComponent($event_name)) {
			trigger_localised_error('CORE0219', E_USER_WARNING, $event_name);
			return FALSE;
		}

		// check whether any triggers listen for this event
		if (!$this->_isEventListenedFor($event_name)) {
			return TRUE;
		}

		// calculate state, prepare state hash
		$state = $this->_calculateState($event_name, $broadcaster, $parameters);
		$state_hash = $this->_calculateStateHash($state);

		$broadcast_status = TRUE;

		// lookup triggers
		$candidate_trigger_ids = $this->_lookupCandidateTriggers($state_hash);
		
		// Enhancement #5488: Trigger Batching Specify Trigger Ids
		$specified_trigger_ids = !empty($parameters['specified_trigger_ids']) ? $parameters['specified_trigger_ids'] : $candidate_trigger_ids;
		$candidate_trigger_ids = array_intersect($candidate_trigger_ids, $specified_trigger_ids);

		// execute each trigger
		foreach ($candidate_trigger_ids as $trigger_id) {

			$status_message = NULL;
			$msg_type = NULL;
			// re-create a trigger
			
			
			$trigger = $this->_loadTrigger($trigger_id);
			// Timer!
			$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, $trigger['name']);
			$status  = $this->_executeTrigger($trigger['data'], $state);
			// End timer
			$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, $trigger['name']);
			
			
			// get settings
			$trigger_settings =& $trigger['data']['settings'];
			$is_blocking      = array_get_index($trigger['data']['settings'], 'blocking', FALSE);

			// prepare message data
			$msg_reps = Array(
							'trigger_name'	=> $trigger['name'],
							'triggerid'		=> $trigger['id'],
							'event'			=> $event_name,
							'asset_name'	=> $broadcaster->name,
							'assetid'		=> $broadcaster->id,
						);

			switch ($status) {
				case SQ_TRIG_RESULT_FAILURE:
					if ($is_blocking) {
						// if the trigger is set to block, then drop out of the
						// loop to prevent the other triggers from running
						// only allowing blocking triggers to affect the broadcast status
						$broadcast_status = FALSE;
						break 2;
					} else {
						trigger_localised_error('CORE0127', E_USER_WARNING, $trigger['id'], $trigger['name'], $event_name, $broadcaster->id, $broadcaster->name);
					}

				break;

				case SQ_TRIG_RESULT_SUCCESS:
					$msg_type = 'trigger.success';
				break;

				case SQ_TRIG_RESULT_INVALID:
					$msg_type = 'trigger.invalid';
				break;

				default:
			}

			if (!empty($msg_type)) {
				// log message using messaging service
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();

				$message = $ms->newMessage(Array(), $msg_type, $msg_reps);
				$message->send();
			}

		}//end foreach

		// clear out the record of running triggers
		$this->_tmp['running_triggers'] = Array();

		return $broadcast_status;

	}//end broadcastEvent()


	/**
	* Returns a state calculated from the event.
	*
	* @param string	$event_type			the name of the event to calculate the state from
	* @param object	&$event_broadcaster	the firing asset
	* @param array	$parameters			extra parameters
	*
	* @return array
	* @access private
	*/
	function _calculateState($event_type, &$event_broadcaster, $parameters=NULL)
	{
		// calculate the environment state
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// for use by the trigger
		$state['event']['name'] = $event_type;
		$state['event']['data'] = $parameters;

		$state['asset']         =& $event_broadcaster;
		$state['assetid']       = $event_broadcaster->id;
		$state['asset_type']    = $event_broadcaster->type();

		//// parent links
		$parent_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, NULL, TRUE, 'minor');
		$new_parent_links = Array();
		$parent_assets    = Array();
		$parent_link_map  = Array();
		foreach ($parent_links as $link) {
			$new_parent_links[$link['linkid']] = $link;
			$parent_assets[$link['majorid']] = $link['major_type_code'];
			$parent_link_map[$link['majorid']][] = $link['linkid'];
		}
		$state['immediate_parent_link_map'] = $parent_link_map;
		$state['immediate_parent_links']    = $new_parent_links;
		$state['immediate_parents']         = $parent_assets;


		//// child links
		$child_links = $am->getLinks($state['assetid'], SQ_SC_LINK_ALL, NULL, TRUE, 'major');
		$new_child_links = Array();
		$child_assets = Array();
		$child_link_map  = Array();
		foreach ($child_links as $link) {
			$new_child_links[$link['linkid']] = $link;
			$child_assets[$link['minorid']] = $link['minor_type_code'];
			$child_link_map[$link['minorid']][] = $link['linkid'];
		}
		$state['immediate_child_link_map'] = $child_link_map;
		$state['immediate_child_links'] = $new_child_links;
		$state['immediate_children'] = $child_assets;

		// uses type 1,2 and 3 only
		$tmp_treeid = $am->getAssetTreeids($state['assetid']);

		if ($tmp_treeid) {
			$state['treeid'] = $tmp_treeid;
		} else {
			$state['treeid'] = Array();
		}

		return $state;

	}//end _calculateState()


	/**
	* Returns the trigger "state" for the trigger action actioning on the given asset id
	* This evalutes only the necessary state data required by the trigger action
	*
	* @param string	$assetid
	* @param array	$state
	*
	* @return array|boolean
	* @access private
	*/
	private function _calculatePseudoState($assetid, $state)
	{
		$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) {
			return FALSE;
		}

		$pseudo_state = Array();

		$pseudo_state['event'] = Array();
		$pseudo_state['event']['name'] = $state['event']['name'];
		$pseudo_state['event']['data'] = $state['event']['data'];

		$pseudo_state['asset'] = $asset;
		$pseudo_state['assetid'] = $asset->id;
		$pseudo_state['type'] = $asset->type();
	
		$pseudo_state['target_assetids'] = $state['target_assetids'];
		
		// Below trigger state data is not used or required by trigger actions
		// and hence can be left to empty
		$pseudo_state['immediate_parent_link_map'] = Array();
		$pseudo_state['immediate_parent_links'] = Array();
		$pseudo_state['immediate_parents'] = Array();
		$pseudo_state['immediate_child_link_map'] = Array();
		$pseudo_state['immediate_child_links'] = Array();
		$pseudo_state['immediate_children'] = Array();
		$pseudo_state['treeid'] = Array();

		return $pseudo_state;

	}//end _calculatePseudoState()

	/**
	* Returns a state_hash calculated from the state
	*
	* @param array	&$state	the state to calculate the hash from
	*
	* @return array
	* @access private
	*/
	function _calculateStateHash(&$state)
	{
		$state_hash['event']        = $state['event']['name'];
		$state_hash['assetid']      = $state['assetid'];
		$state_hash['asset_type']   = $state['asset_type'];
		$state_hash['treeid']       = $state['treeid'];
		$state_hash['parents']      = $state['immediate_parents'];
		$state_hash['children']     = $state['immediate_children'];

		return $state_hash;

	}//end _calculateStateHash()


	/**
	* Lookup the State hash table for all triggers that might need executing
	*
	* @param array	$state_hash	The hash used to check the database with
	*
	* @return array
	* @access private
	*/
	function _lookupCandidateTriggers($state_hash=NULL)
	{// TODO TOF TEST THIS
		// if no state_hash, then fail
		if (empty($state_hash)) return Array();
		$db = MatrixDAL::getDb();
		$bind_vars = Array();

		// compose the link_tree SQL block
		$treeid_sql_array[] = 'treeid = :null_treeid';
		$bind_vars['null_treeid'] = (string)'-1';
		if (empty($state_hash['treeid'])) {
			$state_hash['treeid'] = Array();
		}

		$treeid_count = 0;
		$propagate_flag_count = 0;
		$no_propagate_flag_count = 0;
		$current_tree_length_count = 0;
		foreach ($state_hash['treeid'] as $current_treeid) {
			if (empty($current_treeid)) continue;

			$treeid_sql_array[] = '
						(
							(treeid != :current_treeid'.'_'.++$treeid_count.' AND SUBSTR(:current_treeid'.'_'.++$treeid_count.', 1, LENGTH(treeid)) = treeid)
							AND (
								tree_propagate = :propagate_flag'.'_'.++$propagate_flag_count.'
								OR (tree_propagate = :no_propagate_flag'.'_'.++$no_propagate_flag_count.' AND LENGTH(treeid) = :current_tree_length'.'_'.++$current_tree_length_count.')
							)
						)';

			for ($i=$treeid_count-1; $i<=$treeid_count; $i++) {
				$bind_vars['current_treeid_'.$i] = $current_treeid;
			}
			$bind_vars['propagate_flag_'.$propagate_flag_count]				= SQ_TRIG_TREE_PROPAGATE;
			$bind_vars['no_propagate_flag_'.$no_propagate_flag_count]		= SQ_TRIG_TREE_NO_PROPAGATE;
			$bind_vars['current_tree_length_'.$current_tree_length_count]	= strlen($current_treeid) - SQ_CONF_ASSET_TREE_SIZE;
		}

		$treeid_sql = implode($treeid_sql_array, ' OR ');

		//// PARENTS
		$parent_id_array = Array();
		$parent_type_array = Array();

		$parents = $state_hash['parents'];
		foreach ($parents as $id => $type) {
			$parent_id_array[]   = $id;
			$parent_type_array[$type] = '\''.$type.'\'';
		}

		// parent IDs
		$parent_id_sql = 'parentid = :null_parentid';
		$bind_vars['null_parentid'] = (string)'-1';
		if (!empty($parent_id_array)) {
			for (reset($parent_id_array); NULL !== ($k = key($parent_id_array)); next($parent_id_array)) {
				$parent_id_array[$k] = '\''.(string) $parent_id_array[$k].'\'';
			}

			foreach (array_chunk($parent_id_array, 200) as $chunk) {
				$parent_id_sql .= ' OR parentid IN ('.implode(',', $chunk).')';
			}
		}

		// parent types
		$parent_type_sql = 'parent_type = :null_parent_type';
		$bind_vars['null_parent_type'] = (string)'-1';
		if (!empty($parent_type_array)) {
			$parent_type_sql .= ' OR parent_type IN ('.implode(',', $parent_type_array).')';
		}


		//// CHILDREN
		$child_id_sql   = 'childid = :null_childid';
		$child_type_sql = 'child_type = :null_child_type';
		$bind_vars['null_childid']    = (string)'-1';
		$bind_vars['null_child_type'] = (string)'-1';

		if (!empty($state_hash['treeid'])) {
			// children will always be the same, regardless which treeid is used, so just choose the first one off the top
			$treeid        = reset($state_hash['treeid']);

			// TODO: This gets included twice, so needs to be modified to
			// change the bind variables in the second query.
			// get all the immediate children
			$child_fromwhere_sql = '
				FROM
					(sq_ast_lnk_tree t INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid)
					INNER JOIN sq_ast a ON l.minorid = a.assetid
				WHERE
					t.treeid LIKE :childsql_treeid_wildcard
					AND t.treeid > :childsql_treeid
					AND LENGTH(t.treeid) <= :childsql_treeid_length
			';
			$bind_vars['childsql_treeid_wildcard'] = $treeid.'%';
			$bind_vars['childsql_treeid']          = $treeid;
			$bind_vars['childsql_treeid_length']   = strlen($treeid) + SQ_CONF_ASSET_TREE_SIZE;

			$child_id_sql .= '
				OR childid IN
				(
				SELECT DISTINCT l.minorid
				'.$child_fromwhere_sql.'
				)
			';

			$child_type_sql .= '
				OR child_type IN
				(
				SELECT DISTINCT a.type_code
				'.$child_fromwhere_sql.'
				)
			';
		}//end if

		// event
		$event_sql = 'event = :null_event_id';
		$bind_vars['null_event_id'] = (string)'-1';
		if (!is_null($state_hash['event'])) {
			$event_sql .= ' OR event = :event_id';
			$bind_vars['event_id'] = $state_hash['event'];
		}

		// broadcaster's asset ID
		$assetid_sql = 'assetid = :null_bc_assetid';
		$bind_vars['null_bc_assetid'] = (string)'-1';
		if (!is_null($state_hash['assetid'])) {
			$assetid_sql .= ' OR assetid = :bc_assetid';
			$bind_vars['bc_assetid'] = $state_hash['assetid'];
		}

		// broadcaster's asset Type
		$asset_type_sql = 'asset_type = :null_bc_asset_type';
		$bind_vars['null_bc_asset_type'] = (string)'-1';
		if (!is_null($state_hash['asset_type'])) {
			$asset_type_sql .= ' OR asset_type = :bc_asset_type';
			$bind_vars['bc_asset_type'] = $state_hash['asset_type'];
		}

		// compose query
		$sql = 'SELECT DISTINCT triggerid
				FROM sq_trig_hash
				WHERE   ('.$event_sql.')
					AND ('.$assetid_sql.')
					AND ('.$asset_type_sql.')
					AND ('.$treeid_sql.')
					AND ('.$parent_id_sql.')
					AND ('.$parent_type_sql.')
					AND ('.$child_id_sql.')
					AND ('.$child_type_sql.')';

		$result = NULL;
		 try {
			$query = MatrixDAL::preparePdoQuery($sql);
			foreach ($bind_vars as $bind_var => $bind_value) {
				MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
			}
			$result = MatrixDAL::executePdoAssoc($query, 0);
		 } catch (Exception $e) {
			 throw new Exception('unable to get the candidate triggers: '.$e->getMessage());
		 }

		return $result;

	}//end _lookupCandidateTriggers()


	/**
	* Execute one trigger, using the passed trigger data and state info
	*
	* Returns the status of the executed trigger
	*
	* @param array	$trigger_data	details (id, name, etc.) and data (actions, conditions) associated with the trigger
	* @param array	&$state_data	data relating to the asset the trigger is running on
	*
	* @return boolean
	* @access private
	*/
	function _executeTrigger($trigger_data, &$state_data)
	{
		$status = SQ_TRIG_RESULT_SUCCESS;


		$conditions  = $trigger_data['conditions'];
		$actions     = $trigger_data['actions'];

		if (isset($trigger_data['settings'])) {
			$is_blocking = array_get_index($trigger_data['settings'], 'blocking', FALSE);
		} else {
			$is_blocking = FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// check each condition
		foreach ($conditions as $condition) {
			// include condition code
			if (!$this->_loadComponent($condition['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}

			// check condition
			// dynamic substitution of static condition class
			$condition_success = call_user_func_array(Array($condition['type'], 'evaluate'), Array($condition['data'], &$state_data));

			// if condition fails return with Success
			// (trigger does not need execution, false alarm but trigger did not fail)
			if ($condition_success == $condition['inverse_condition']) {
				return SQ_TRIG_RESULT_FALSE;
			}
		}


		// all conditions pass. now perform the actions.
		// NOTE: this is not thread-safe
		// for thread safety all the conditions and actions need to be a part of one transaction
		// we rely on the fact that this function will be wrappend in a transaction

		// this relies on the state remaining constant throughout trigger execution
		// this assumption will be the first to blame for unpredictable behaviour

		// include action code in one go, in case one of the middle actions are missing
		foreach ($actions as $action) {
			if (!$this->_loadComponent($action['type'])) {
				return SQ_TRIG_RESULT_INVALID;
			}
		}

		// init locks - this will be used to store locks to release after executing all the actions
		$locks_held = Array();

		// perform actions
		foreach ($actions as $action) {
			$not_required = array_get_index($action, 'not_required', FALSE);
			$ignore_permissions = array_get_index($action, 'ignore_permissions', FALSE);

			// Trigger action's "other" asset ids to action on
			$target_assetids = $this->_getTargetAssetIds($action, $state_data);
			if ($target_assetids === FALSE) {
				// If "Action On Other Assets" setting is empty, use the current asset
				$target_assetids = Array($state_data['assetid']);
			}
			$state_data['target_assetids'] = $target_assetids;

			// If there are no target assets to action on, its as good
			$action_success = TRUE;

			// If the target assets are to actioned in a single go, in which case the foreach loop below will break in the very first iteration
			// It is the responsibility of the specific trigger action to process all the target assets in the a single action call
			$action_at_once = method_exists($action['type'], 'actionAssetsAtOnce') && is_callable(Array($action['type'], 'actionAssetsAtOnce')) && call_user_func(Array($action['type'], 'actionAssetsAtOnce'));

			foreach($target_assetids as $target_assetid) {

				// Whether actioning on broadcaster or the "other" assets, we need to set the state data accourdingly
				if ($target_assetid == $state_data['assetid'] || $action_at_once) {
					$action_state_data = $state_data;
				} else {
					$action_state_data = $this->_calculatePseudoState($target_assetid, $state_data);
				}

				if ($ignore_permissions || !$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_LOCKING)) {

					// execute action
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$action_success = call_user_func_array(Array($action['type'], 'execute'), Array($action['data'], &$action_state_data));
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

				} else {
					$lock_success = TRUE;
					// acquire locks
					// dynamic substitution of static action class
					$action_locks = call_user_func_array(Array($action['type'], 'getLocks'), Array($action['data'], &$action_state_data));

					foreach ($action_locks as $lock_assetid => $lock_types) {
						foreach ($lock_types as $lock_type) {
							$lock_success = $am->acquireLock($lock_assetid, $lock_type);
							// add this lock to the list of locks to release if we actually acquired, not if we only updated
							if ($lock_success==1) {
								$locks_held[$lock_assetid][$lock_type] = TRUE;
							} else if ($lock_success==0) {
								// bail if we can't get all of the locks
								break 2;
							}
						}
					}

					if (!$lock_success) {
						if ($not_required) {
							// next action, please
							continue;
						} else {
							$status = SQ_TRIG_RESULT_FAILURE;
							break;
						}
					}

					// execute action
					$action_success = call_user_func_array(Array($action['type'], 'execute'), Array($action['data'], &$action_state_data));
				}//end else

				if (!$action_success && !$not_required) {
					break;
				}

				if ($action_at_once) {
					// The trigger action is supposed to action all target assets in a single go, so we are done here
					break;
				}

			}//end foreach target assetids

			if ($action_success === FALSE) {
				if ($not_required) {
					if (!$is_blocking) {
						trigger_localised_error('CORE0134', E_USER_NOTICE, $action['type']);
					}
					continue;
				} else {
					$status = SQ_TRIG_RESULT_FAILURE;
					if (!$is_blocking) {
						trigger_localised_error('CORE0135', E_USER_NOTICE, $action['type']);
					}
					break;
				}
			}

			$status = SQ_TRIG_RESULT_SUCCESS;

		}//end foreach

		// release the locks
		foreach ($locks_held as $lock_assetid => $lock_types) {
			foreach ($lock_types as $lock_type => $lock_true) {
				// can't do anything if locks aren't released, so don't take result
				$am->releaseLock($lock_assetid, $lock_type);
			}
		}

		return $status;

	}//end _executeTrigger()


	/**
	* Return the list of asset ids to be actioned on by the Trigger Action
	* as per action's "Action On Other Assets" settings
	*
	* @param array	$action	Trigger action settings
	* @param array	$state	Trigger action state data
	*
	* @return array|boolean
	* @access private
	*/
	private function _getTargetAssetIds($action, $state)
	{
		// "Action On Other Assets" settings
		$settings = array_get_index($action, 'other_assets', FALSE);
		if (!$settings || empty($settings['enabled'])) {
			return FALSE;
		}

		$assetids = Array();
		// Include current asset
		if (array_get_index($settings, 'current_asset', FALSE)) {
			$assetids[] = $state['assetid'];
		}

		// Include linked parents of the current asset
		$parnets_linked_types = array_get_index($settings, 'parent_link_types', Array());
		if (!empty($parnets_linked_types)) {
			$link_type = 0;
			foreach($parnets_linked_types as $parnets_linked_type) {
				$link_type |= $parnets_linked_type;
			}
			$link_value = array_get_index($settings, 'parent_link_value', '');
			if (!$link_value) {
				$link_value = NULL;
			}
			$parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($state['assetid'], $link_type, '', TRUE, 'minor', $link_value);
			if (!empty($parent_links)) {
				foreach($parent_links as $parent_link) {
					if (isset($parent_link['majorid'])) {
						$assetids[] = $parent_link['majorid'];

						// Note: This is as an exception - when the noticed linked parent is bodycopy container/content type,
						// we include its dependant parent in the list as well ("./?=xxx", nesting asset)
						if ($parent_link['link_type'] == SQ_LINK_NOTICE) {
							$linked_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);
							if ($linked_parent instanceof Content_Type || $linked_parent instanceof Bodycopy_Container) {
								$dependant_parents = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($parent_link['majorid']);
								$assetids = array_merge($assetids, $dependant_parents);
							}
						}
					}//end if
				}//end foreach
			}//end if
		}//end if

		// Include the selected assets
		if (!empty($settings['assets'])) {
			$assetids = array_merge($assetids, $settings['assets']);
		}

		// Include dynamic source assets
		if (!empty($settings['dynamic_source'])) {
			$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $settings['dynamic_source']);
			$value_map_value = $atr_parameter_map->getParameterValue('assetids');
			foreach(explode(',', $value_map_value) as $ds_assetid) {
				$ds_assetid = trim($ds_assetid);
				if (!empty($ds_assetid)) {
					$assetids[] = $ds_assetid;
				}
			}
			if (is_null($old_current_asset)) {
				$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
			} else {
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
	 		}
		}

		if (!empty($assetids)) {
			$assetids = array_unique($assetids);
		}

		// Selected asset types filter
		$selected_asset_types = array_get_index($settings, 'asset_types', '');

		// Include the children of the selected assetids if required
		$child_assetids = Array();
		if (!empty($settings['asset_level']) && $settings['asset_level'] != 'single') {
			// Validate the selected assetids before getting their children
			$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids));

			foreach($assetids as $assetid) {
				if ($settings['asset_level'] == 'children') {
					$child_assetids += $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $selected_asset_types);
				} else {
					$child_assetids += $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid, $selected_asset_types);
				}
			}//end foreach

		}//end if
		
		// Filter the selected assetids as per asset types filter setting
		$assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $selected_asset_types));

		if (!empty($child_assetids)) {
			$assetids = array_unique(array_merge($assetids, array_keys($child_assetids)));
			unset($child_assetids);
		}

		return $assetids;

	}//end _getTargetAssetIds()


//--        TRIGGER CREATION        --//


	/**
	* Create and return an instance of hash object
	*
	* @return object
	* @access private
	*/
	function _getHashObject()
	{
		$hash = new Hash();
		return $hash;

	}//end _getHashObject()


	/**
	* Return a blank trigger in preparation for editing
	*
	* @return array
	* @access private
	*/
	function _getEmptyTrigger()
	{
		$trigger['name'] = '';
		$trigger['description'] = '';
		$trigger['category'] = '';
		$trigger['data'] = NULL;
		$trigger['active'] = SQ_TRIG_STATUS_INACTIVE;
		$trigger['state_hash'] = $this->_getHashObject();

		return $trigger;

	}//end _getEmptyTrigger()


	/**
	* Prepare the trigger and state hash for saving, and ship them off to be saved
	*
	* Returns trigger ID on success or FALSE on failure
	*
	* @param array	$trigger	the trigger to be saved
	*
	* @return int
	* @access private
	*/
	function _saveTrigger($trigger)
	{//TODO TOF: write unit tests
		// triggers do not work in ROLLBACK mode
		if (SQ_ROLLBACK_VIEW) return TRUE;
		if (empty($trigger)) return FALSE;

		try {
			// start transaction
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$db = MatrixDAL::getDb();
			$status = TRUE;
			$new    = FALSE;

			// lack of id means it is a new trigger
			// this bit gets all ids to come from the sequence, manual entries will make db inconsistent
			if (!isset($trigger['id'])) {
				$new = TRUE;
				$tr_id = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_trig_id_seq'));
				$trigger['id'] = $tr_id;
			}

			if ($new) {

				$sql = 'INSERT INTO
							sq_trig
							(
								id,
								data,
								active,
								description,
								category,
								name
							)
							VALUES
							(
								:id,
								:data,
								:active,
								:description,
								:category,
								:name
							)';

				$prepared = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($prepared, 'id', $trigger['id']);
				MatrixDAL::bindValueToPdo($prepared, 'data', serialize($trigger['data']));
				MatrixDAL::bindValueToPdo($prepared, 'active', array_get_index($trigger,'active', SQ_TRIG_STATUS_DEFAULT));
				MatrixDAL::bindValueToPdo($prepared, 'description', $trigger['description']);
				MatrixDAL::bindValueToPdo($prepared, 'category', $trigger['category']);
				MatrixDAL::bindValueToPdo($prepared, 'name', $trigger['name']);
				$result = MatrixDAL::execPdoQuery($prepared);

			} else {

				$sql = 'UPDATE
							sq_trig
						SET
							data		= :data,
							active		= :active,
							description	= :description,
							category	= :category,
							name		= :name
						WHERE
						id = :id';

				$prepared = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($prepared, 'id', $trigger['id']);
				MatrixDAL::bindValueToPdo($prepared, 'data', serialize($trigger['data']));
				MatrixDAL::bindValueToPdo($prepared, 'active', array_get_index($trigger,'active', SQ_TRIG_STATUS_DEFAULT));
				MatrixDAL::bindValueToPdo($prepared, 'description', $trigger['description']);
				MatrixDAL::bindValueToPdo($prepared, 'category', $trigger['category']);
				MatrixDAL::bindValueToPdo($prepared, 'name', $trigger['name']);
				$result = MatrixDAL::execPdoQuery($prepared);
			}

			$this->_deleteTriggerHash($trigger['id']);

			// prepare and save the hash
			// the edit interface can set the optional 'save_hash' index to false, which will cause
			// the hash not to be saved for this trigger, for whatever reason
			if ($status && array_get_index($trigger, 'save_hash', TRUE) && $trigger['active']) {

				$hash = $this->_getHashObject();
				$hash->setTriggerId($trigger['id']);

				// update the hash
				if (isset($trigger['data'])) {
					if (isset($trigger['data']['events'])) {
						foreach ($trigger['data']['events'] as $event) {
							$hash->setEvent($event);
						}
					}
					if (isset($trigger['data']['conditions'])) {
						foreach ($trigger['data']['conditions'] as $condition) {
							if ($GLOBALS['SQ_SYSTEM']->am->installed(array_get_index($condition, 'type', ''))) {
								if ($this->_loadComponent($condition['type'])) {
									if (isset($condition['data'])) {
										if (!$condition['inverse_condition']) {
											$settings =& $condition['data'];
										} else {
											$settings = Array();
										}
										call_user_func_array(Array($condition['type'], 'setHash'), Array(&$settings, &$hash));
									}
								}
							}
						}
					}
				}

				$status = $status && $this->_saveTriggerHash($hash);
			}

			// now try to save the trigger itself
			if ($status) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return $trigger['id'];
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

		} catch (Exception $e)
		{
			throw new Exception('Unable to save the trigger due to database error: '.$e->getMessage());
		}

	}//end _saveTrigger()


	/**
	* Saves the trigger hash in the database
	*
	* @param object	$hash	the Hash object
	*
	* @return boolean
	* @access private
	*/
	function _saveTriggerHash($hash=NULL)
	{// TODO TOF TEST
		if (empty($hash)) return FALSE;

		$hash_combinations = $hash->getHashCombinations();
		if (empty($hash_combinations)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($hash_combinations as $combination) {

			$field_array = Array();
			$value_array = Array();

			foreach ($combination as $hash_id => $hash_data) {
				$field_array[] = $hash_id;
				$value_array[] = '\''.($hash_data).'\'';
			}

			$sql = 'INSERT INTO
						sq_trig_hash
						(
							'.implode($field_array, ', ').'
						)
						VALUES
						(
							'.implode($value_array, ', ').'
						)';

			$result = NULL;
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				$result = MatrixDAL::execPdoQuery($query);
			} catch (Exception $e) {
				throw new Exception('could not create the trigger hash due to database error: '.$e->getMessage());
			}

			// check result
			if ($result === FALSE) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _saveTriggerHash()


	/**
	* Format and return the state_hash
	*
	* @param array	$state_hash	the state_hash to check
	*
	* @return array
	* @access private
	*/
	function _prepareRawStateHash($state_hash)
	{
		// if the state_hash is empty or not an array, make it an array so we
		// can set defaults later
		if (empty($state_hash) || !is_array($state_hash)) {
			$state_hash = Array();
		}

		// define the defaults
		$state_hash_defaults = Array(
								'event'				=> '',
								'assetid'			=> '',
								'asset_type'		=> '',
								'treeid'			=> '',
								'tree_propagate'	=> SQ_TRIG_TREE_NO_PROPAGATE,
								'triggerid'			=> '',
							   );

		// make sure every field has data, and that no fields are null
		foreach ($state_hash_defaults as $defaults_id => $defaults_data) {
			if (empty($state_hash[$defaults_id])) {
				$state_hash[$defaults_id] = $defaults_data;
			}
		}

		return $state_hash;

	}//end _prepareRawStateHash()


//--        TRIGGER DELETION        --//


	/**
	* Deletes the trigger from trigger table and clears the hash entries
	*
	* @param int	$trigger_id	id of trigger to be deleted
	*
	* @return boolean
	* @access private
	*/
	function _deleteTrigger($trigger_id)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$result  = NULL;
		try {
			$bind_vars	= Array (
									'trigger_id'			=> $trigger_id,
								);
			$result  = MatrixDAL::executeQuery('core', 'deleteTriggerId', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable delete the Trigger'.$trigger_id.':  due to database error: '.$e->getMessage());
		}

		$success = $this->_deleteTriggerHash($trigger_id);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $success;

	}//end _deleteTrigger()


	/**
	* Removes all hash entries for a particular trigger ID
	*
	* @param int	$trigger_id	id of trigger whose hash is be deleted
	*
	* @return boolean
	* @access private
	*/
	function _deleteTriggerHash($trigger_id)
	{// TODO TOF TEST THIS FUNCTION
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		try {
			$db = MatrixDAL::getDb();
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$sql = 'DELETE FROM
						sq_trig_hash
					WHERE
					triggerid = :triggerid';

			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'triggerid', $trigger_id);
			$result = MatrixDAL::execPdoQuery($query);

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} catch(Exception $e) {
			throw new Exception("Failed to delete the hash (#$trigger_id): ".$e->getMessage());
		}
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _deleteTriggerHash()


//--        GETTERS/SETTERS, LOAD FUNCTIONS        --//


	/**
	* Returns a list of all the triggers, including data
	* See Trigger_Manager::getTrigger() for a complete description of the return format
	*
	*
	* @return array
	* @access public
	* @see Trigger_Manager::getTrigger()
	*/
	function &getTriggerList()
	{// TODO TOF TEST THIS FUNCTION

			$trigger_db = MatrixDAL::executeAll('core', 'getTriggerList');

		if (empty($trigger_db)) {
			foreach ($trigger_db as $trigger_data) {
				$trigger_data['data'] = (isset($trigger_data['data']) && !empty($trigger_data['data'])) ? unserialize($trigger_data['data']) : '';
			}
		}

		return $trigger_db;

	}//end getTriggerList()


	/**
	* Returns a trigger with the passed ID
	*
	* <pre>
	* Array(
	*	'id'			=> integer,			// the triggerid
	*	'name'			=> string,			// the name of the trigger
	*	'description'	=> string,			// a description of the trigger
	*	'active'		=> boolean,			// on/off switch for each trigger
	*	'data'			=> Array(
	*						'events'		=> Array(
	*											'event_name'	=> 'event_name',
	*										   ),
	*						'conditions'	=> Array(
	*											int	=> Array(
	*													'type'	=> string							// the type_code of the condition
	*													'data'	=> Array(mixed)						// condition-specific data
	*												   ),
	*										   ),
	*						'actions'		=> Array(
	*											int	=> Array(
	*													'type'					=> string			// the type_code of the condition
	*													'data'					=> Array(mixed)		// condition-specific data
	*													'not_required'			=> boolean			// if true, if the action fails, the trigger will continue to run
	*													'ignore_permissions'	=> boolean			// whether to run with elevated privileges
	*												   ),
	*										   ),
	*						'settings'		=> Array(
	*											'blocking'	=> boolean,		// if true, then if this trigger fails it will halt the
	*																		// execution of any further triggers scheduled to run
	*										   ),
	*					   ),
	* )
	* </pre>
	*
	* @param int	$triggerid	The id of the trigger to retrieve
	*
	* @return array
	* @access public
	*/
	function getTrigger($triggerid)
	{
		if (is_numeric($triggerid)) {
			return $this->_loadTrigger($triggerid);
		} else {
			return NULL;
		}

	}//end getTrigger()


	/**
	* Creates a trigger from data passed, and returns the id of the new trigger
	* See Trigger_Manager::getTrigger() for a complete description of $trigger,
	* except that new triggers do not require $trigger['id']
	*
	* @param int	&$trigger	the id of the trigger to use to create the hash, null for all
	*
	* @return int
	* @access public
	* @see Trigger_Manager::getTrigger()
	*/
	function createTrigger(&$trigger)
	{
		// check the structure of trigger
		// $s contains the status - if any part of $trigger fails the checking,
		// $s will stop further checks from happening
		$s  = TRUE;
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// basic elements
		$s = $s && isset($trigger['name']);
		$s = $s && isset($trigger['description']);
		$s = $s && isset($trigger['active']);
		$s = $s && isset($trigger['data']);

		// events
		$s = $s && isset($trigger['data']['events']);

		// conditions
		$s = $s && isset($trigger['data']['conditions']);
		if ($s && !empty($trigger['data']['conditions'])) {
			foreach ($trigger['data']['conditions'] as $condition) {
				$s = $s && isset($condition['type']) && $am->installed($condition['type']);
				$s = $s && isset($condition['data']);
				$s = $s && isset($condition['inverse_condition']);
			}
		}

		// actions
		$s = $s && isset($trigger['data']['actions']);
		if ($s && !empty($trigger['data']['actions'])) {
			foreach ($trigger['data']['actions'] as $action) {
				$s = $s && isset($action['type']) && $am->installed($action['type']);
				$s = $s && isset($action['data']);
				$s = $s && isset($action['not_required']);
				$s = $s && isset($action['ignore_permissions']);
			}
		}

		if ($s) {
			return $this->_saveTrigger($trigger);
		} else {
			trigger_localised_error('CORE0232', E_USER_WARNING);
			return FALSE;
		}

	}//end createTrigger()


	/**
	* Load Trigger Data
	*
	* @param int	$trigger_id	the ID of the trigger to load
	*
	* @return array
	* @access private
	*/
	function _loadTrigger($trigger_id=NULL)
	{// TODO TOF: test this function
		if (!isset($trigger_id)) return NULL;
		$trigger_db = NULL;
		try {
			$db = MatrixDAL::getDb();

			$select = 'SELECT
						id,
						name,
						data,
						description,
						active,
						category
					FROM
						'.SQ_TABLE_RUNNING_PREFIX.'trig';

			$where  = 'id = :id';
			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$query = MatrixDAL::preparePdoQuery($select.' '.$where);
			MatrixDAL::bindValueToPdo($query, 'id', $trigger_id);
			$trigger_db = MatrixDAL::executePdoAll($query);
			if (!empty($trigger_db)) {
				$trigger_db = $trigger_db[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable get the information from the trigger table due to database error: '.$e->getMessage());
		}

		if (empty($trigger_db)) {
			// trigger_localised_error('CORE0133', E_USER_WARNING, $trigger_id);
			return NULL;
		}

		$trigger_db['data'] = (isset($trigger_db['data']) && !empty($trigger_db['data'])) ? unserialize($trigger_db['data']) : '';

		// set defaults if not present
		if (!isset($trigger_db['data']['events'])) {
			$trigger_db['data']['events'] = Array();
		}
		if (!isset($trigger_db['data']['conditions'])) {
			$trigger_db['data']['conditions'] = Array();
		}
		if (!isset($trigger_db['data']['actions'])) {
			$trigger_db['data']['actions'] = Array();
		}
		if (!isset($trigger_db['data']['settings'])) {
			$trigger_db['data']['settings'] = Array();
		}

		return $trigger_db;

	}//end _loadTrigger()


	/**
	* Obtain a list of trigger info
	*
	* @return array
	* @access private
	*/
	function _getTriggerInfoList()
	{
		$db = MatrixDAL::getDb();

		try {
			$select = 'SELECT
							id,
							name,
							description,
							category,
							active
						FROM
							'.SQ_TABLE_RUNNING_PREFIX.'trig';

			$where  = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause();
			$order  = 'ORDER BY active DESC , name ASC';

			$query	= MatrixDAL::preparePdoQuery($select.' '.$where.' '.$order);
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get trigger info list due to the following database error:'.$e->getMessage());
		}//end try catch

		return $result;

	}//end _getTriggerInfoList()


	/**
	* Returns a list of descendants of a certain type, along with type info
	*
	* @param string	$type	the type_code to get the descendants of
	*
	* @return array
	* @access private
	*/
	function _getComponentList($type=NULL)
	{
		if (empty($type)) return Array();

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		$component_types = $am->getTypeDescendants($type);
		$component_info  = $am->getTypeInfo($component_types, 'name');

		return $component_info;

	}//end _getComponentList()


	/**
	* Get A list of all installed events
	*
	* @return array
	* @access private
	*/
	function _getEventList()
	{
		$event_list = $this->_getComponentList('trigger_event');
		asort($event_list);

		return $event_list;

	}//end _getEventList()


	/**
	* Get a list of installed conditions
	*
	* @return array
	* @access private
	*/
	function _getConditionList()
	{
		return $this->_getComponentList('trigger_condition');

	}//end _getConditionList()


	/**
	* Get a list of installed actions
	*
	* @return array
	* @access private
	*/
	function _getActionList()
	{
		return $this->_getComponentList('trigger_action');

	}//end _getActionList()


	/**
	* Load a component
	*
	* NOTE: logic of this function can be pushed onto asset manager
	* includeAsset could be modified to not throw an exception and return status
	* if that change is ever made, conversion should be simple. Currently asset
	* manager makes an assertion, which kills the system when trying to include
	* something that is not installed
	*
	* @param string	$type_code	the type_code of the component to load
	*
	* @return boolean
	* @access private
	*/
	function _loadComponent($type_code)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if ($am->installed($type_code)) {
			$am->includeAsset($type_code);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end _loadComponent()


	/**
	* Returns a string containing the buffered output of a component's edit interface
	*
	* @param string		$type_code		the type_code of the component to get the interface of
	* @param array		$data			the components settings data
	* @param string		$prefix			the temporary unique ID for the component
	* @param boolean	$write_access	the write access status of the component
	*
	* @return string
	* @access private
	*/
	function _getComponentInterface($type_code, $data, $prefix, $write_access=FALSE, Trigger $trigger=NULL, $action_id)
	{
		if (!$this->_loadComponent($type_code)) return '';

		$interface = call_user_func_array(Array($type_code, 'getInterface'), Array($data, $prefix, $write_access, &$trigger, $action_id));

		return $interface;

	}//end _getComponentInterface()


	/**
	* Determines if more than one instance of a condition is allowed in the one trigger
	*
	* @param string	$type_code	the type_code of the condition to check
	*
	* @return boolean
	* @access private
	*/
	function _isMultipleConditionAllowed($type_code)
	{
		$this->_loadComponent($type_code);
		$status = call_user_func(Array($type_code, 'allowMultiple'));
		return $status;

	}//end _isMultipleConditionAllowed()


	/**
	* Returns a list of triggerids for triggers that listen for a given event
	*
	* @param string	$event	the name of the event
	*
	* @return boolean
	* @access private
	*/
	function _isEventListenedFor($event)
	{// TODO TOF TEST THIS
		// Check the trigger hash table to see whether any active triggers are listening for this event
		$trigger_count  = NULL;
		try {
			$bind_vars	= Array (
									'event'			=> $event,
								);
			$trigger_count  = MatrixDAL::executeOne('core', 'getNbOfTriggerHash', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable get the number of trigger hashes:  due to database error: '.$e->getMessage());
		}


		return !empty($trigger_count);

	}//end _isEventListenedFor()


}//end class


?>
