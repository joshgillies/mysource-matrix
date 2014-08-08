<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_bridge.inc,v 1.21 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_PACKAGES_PATH.'/ipb/ipb.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';


/**
* IPB_Bridge
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/

class IPB_Bridge extends Asset implements Bridge
{

	/**
	* Reference to IPB connector object
	* @var object
	*/
	var $ipb = NULL;

	/**
	* An array of objectclass values that represent groups in LDAP
	* All values must be lowercase
	*
	* @var array(string => string)
	*/
	var $top_categories = Array(
							'g_0'	=> Array(
										'id'				=> 'g_0',
										'name'				=> 'All Users',
										'description'		=> 'IPB User Groups',
										'type_code'			=> 'ipb_user_group',
										'list_function'		=> 'getListOfGroups',
										'minor_prefix'		=> 'g_',
										'minor_type_code'	=> 'ipb_user_group',
									   ),
							'f_0'	=> Array(
										'id'				=> 'f_0',
										'name'				=> 'All Forums',
										'description'		=> 'IPB Forums',
										'type_code'			=> 'ipb_forum',
										'list_function'		=> 'getListOfRootForums',
										'minor_prefix'		=> 'r_',
										'minor_type_code'	=> 'ipb_root_forum',
									   ),
							'o_0'	=> Array(
										'id'				=> 'o_0',
										'poll_question'		=> 'All Polls',
										'description'		=> 'IPB Polls',
										'type_code'			=> 'ipb_poll',
										'list_function'		=> 'getListOfPolls',
										'minor_prefix'		=> 'o_',
										'minor_type_code'	=> 'ipb_poll',
									   ),
						  );

	/**
	* The link type of assets linked underneath an LDAP_Bridge
	*
	* @var int
	*/
	var $ipb_shdw_link_type = SQ_LINK_TYPE_1;


	/**
	* Weather the asset is deprecated or not.
	* this will be used in asset_map.inc:_cloneAsset() to see if the asset type
	* can be created (cloned) on the system
	*/
	public $deprecated = TRUE;


	/**
	* Marking the asset deprecated since so that we can
	* use it when warning the users on backend
	*/
	public $deprecated_since = '5.1';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function IPB_Bridge($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Returns the reference to IPB connector object
	*
	* @return object
	* @access private
	*/
	function &connectToIpb()
	{
		$location = $this->attr('location');
		$set_location = empty($location);
		if ($set_location) return NULL;

		if (is_null($this->ipb)) {
			$this->ipb = new Ipb($this->attr('location'), $this->attr('version'));
		}

		return $this->ipb;

	}//end connectToIpb()


	/**
	* Returns the reference to IPB connector object
	*
	* @return object
	* @access private
	*/
	function &getIpbConn()
	{
		$location = $this->attr('location');
		$set_location = empty($location);
		if ($set_location) {
			$null = NULL;
			return $null;
		}

		if (is_null($this->ipb)) {
			$this->ipb = new Ipb($this->attr('location'), $this->attr('version'));
		}

		return $this->ipb;

	}//end getIpbConn()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
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
	function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;

		$id_parts = explode(':', $shadowid);
		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		$ipb = $this->getIpbConn();
		if (is_null($ipb)) return $asset;
		$prefix = substr($shadowid, 0, 1);

		if (strlen($shadowid) == 3 && substr($shadowid, -1) == '0') {
			// top categories
			if ($prefix == 'g') {
				$type_code = 'ipb_user_group';
				$class_name = 'Ipb_User_Group';
			} else if ($prefix == 'f') {
				$type_code = 'ipb_forum';
				$class_name = 'Ipb_Forum';
			} else if ($prefix == 'o') {
				$type_code = 'ipb_poll';
				$class_name = 'Ipb_Poll';
			}

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$asset = new $class_name($this->id, $this->top_categories[$shadowid]);

		} else {
			// all the other shadow assets from IPB
			if ($prefix == 'u') {
				$type_code	= 'ipb_user';
				$method		= 'getMemberInfo';
				$class_name	= 'Ipb_User';

			} else if ($prefix == 'g') {
				$type_code	= 'ipb_user_group';
				$method		= 'getGroupInfo';
				$class_name	= 'Ipb_User_Group';

			} else if ($prefix == 'r') {
				$type_code	= 'ipb_root_forum';
				$method		= 'getRootForumInfo';
				$class_name	= 'Ipb_Root_Forum';

			} else if ($prefix == 'f') {
				$type_code	= 'ipb_forum';
				$method		= 'getForumInfo';
				$class_name	= 'Ipb_Forum';

			} else if ($prefix == 't') {
				$type_code	= 'ipb_topic';
				$method		= 'getTopicInfo';
				$class_name	= 'Ipb_Topic';

			} else if ($prefix == 'p') {
				$type_code	= 'ipb_post';
				$method		= 'getPostInfo';
				$class_name	= 'Ipb_Post';

			} else if ($prefix == 'o') {
				$type_code	= 'ipb_poll';
				$method		= 'getPollInfo';
				$class_name	= 'Ipb_Poll';
			}
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$ipb = $this->getIpbConn();
				eval('$data = $ipb->'.$method.'($this->id.\':\'.$shadowid);');

			$data['id'] = $shadowid;

			$asset = new $class_name($this->id, $data);

		}//end else
		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* NOTE that for historical reasons this function's arguments are arranged differently to those in
	* Asset_Manager::getLinks()
	*
	* @param int			$assetid			id of the the $side_of_link asset
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
	* @param boolean		$dependant			Whether we want dependant links, non-dependent or (null) don't care
	* @param boolean		$exclusive			Whether we want exclusive links, non-exclusive or (null) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		$links = Array();

		// if the wanted link type is not the type of links
		// that this bridge allows, lets return now
		if (!($this->ipb_shdw_link_type & $link_types)) {
			return Array();
		}

		if ($dependant || $exclusive) {
			return Array();
		}

		$no_id_parts = FALSE;
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$assetid = $id_parts[1];
		} else {
			$no_id_parts = TRUE;
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		// is not connected yet...
		$ipb =& $this->getIpbConn();
		if (is_null($ipb) || !$ipb->isConnected()) {
			return $links;
		}

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link])) {

			if ($no_id_parts) {

				/////////////////////////////////
				// Links from Ipb_Bridge
				/////////////////////////////////
				if ($side_of_link == 'major') {

					// major
					foreach ($this->top_categories as $top_key => $top_data) {
						$links[] = Array(
									'linkid'			=> 0,
									'majorid'			=> $this->id,
									'minorid'			=> $this->id.':'.$top_key,
									'minor_type_code'	=> $top_data['type_code'],
									'value'				=> '',
									'link_type'			=> $this->ipb_shdw_link_type,
									'is_dependant'		=> FALSE,
									'is_exclusive'		=> FALSE,
									'sort_order'		=> 0,
								   );
					}

				} else {
					// minor
					$links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, 'minor');
				}

			} else if (array_key_exists($assetid, $this->top_categories)) {

				/////////////////////////////////
				// Links from Top Categories
				/////////////////////////////////
				if ($side_of_link == 'major') {

					// major
					$ipb =& $this->getIpbConn();
					if (!$ipb->isConnected()) return $links;

					$list_function = $this->top_categories[$assetid]['list_function'];
					$prefix = $this->top_categories[$assetid]['minor_prefix'];
					$type_code = $this->top_categories[$assetid]['minor_type_code'];

					$list = $ipb->$list_function();
					foreach ($list as $sub_group) {
						$links[] = Array(
									'linkid'			=> 0,
									'majorid'			=> $this->id.':'.$assetid,
									'minorid'			=> $this->id.':'.$prefix.$sub_group['id'],
									'minor_type_code'	=> $type_code,
									'value'				=> '',
									'link_type'			=> $this->ipb_shdw_link_type,
									'is_dependant'		=> FALSE,
									'is_exclusive'		=> FALSE,
									'sort_order'		=> 0,
								   );
					}

				} else {

					// minor, link to Ipb_Bridge
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'major_type_code'	=> $this->type(),
								'minorid'			=> $this->id.':'.$assetid,
								'value'				=> '',
								'link_type'			=> $this->ipb_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
							   );
				}

			} else {
				$prefix = substr($assetid, 0, 1);

				/////////////////////////////////
				// Links from shadow assets
				/////////////////////////////////
				if ($side_of_link == 'major') {

					if ($prefix == 'g') {

						// ipb_user_group => ipb_user
						$ipb =& $this->getIpbConn();
							$tmp = explode('_', $assetid);
							$group_id = $tmp[1];
							$members = $ipb->getMembers(0, $group_id);
							foreach ($members as $key => $data) {
								$links[] = Array(
											'linkid'			=> 0,
											'majorid'			=> $this->id.':'.$assetid,
											'minorid'			=> $this->id.':'.$key,
											'minor_type_code'	=> 'ipb_user',
											'value'				=> '',
											'link_type'			=> $this->ipb_shdw_link_type,
											'is_dependant'		=> FALSE,
											'is_exclusive'		=> FALSE,
											'sort_order'		=> 0,
										   );
							}

					} else if ($prefix == 'u') {

						// ipb_user => NONE
						$links = Array();

					} else if ($prefix == 'r') {

						// ipb_root_forum => ipb_forum
						$ipb =& $this->getIpbConn();
							$list_of_topics = $ipb->getListOfForums($assetid);
						foreach ($list_of_topics as $topic) {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id.':'.$assetid,
										'minorid'			=> $this->id.':f_'.$topic['id'],
										'minor_type_code'	=> 'ipb_forum',
										'value'				=> '',
										'link_type'			=> $this->ipb_shdw_link_type,
										'is_dependant'		=> FALSE,
										'is_exclusive'		=> FALSE,
										'sort_order'		=> 0,
									   );
						}

					} else if ($prefix == 'f') {

						// ipb_forum => ipb_topic
						$ipb =& $this->getIpbConn();
							$list_of_topics = $ipb->getListOfTopics($assetid);
						foreach ($list_of_topics as $topic) {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id.':'.$assetid,
										'minorid'			=> $this->id.':t_'.$topic['id'],
										'minor_type_code'	=> 'ipb_topic',
										'value'				=> '',
										'link_type'			=> $this->ipb_shdw_link_type,
										'is_dependant'		=> FALSE,
										'is_exclusive'		=> FALSE,
										'sort_order'		=> 0,
									   );

						}

					} else if ($prefix == 't') {

						// ipb_forum => ipb_topic
						$ipb =& $this->getIpbConn();
							$list_of_posts = $ipb->getListOfPosts($assetid);
						foreach ($list_of_posts as $post) {
							$links[] = Array(
										'linkid'			=> 0,
										'majorid'			=> $this->id.':'.$assetid,
										'minorid'			=> $this->id.':p_'.$post['id'],
										'minor_type_code'	=> 'ipb_post',
										'value'				=> '',
										'link_type'			=> $this->ipb_shdw_link_type,
										'is_dependant'		=> FALSE,
										'is_exclusive'		=> FALSE,
										'sort_order'		=> 0,
									   );
						}

					}

				} else {

					// minor
					$links = Array(
								'linkid'		=> 0,
								'value'			=> '',
								'link_type'		=> $this->ipb_shdw_link_type,
								'is_dependant'	=> FALSE,
								'is_exclusive'	=> FALSE,
								'sort_order'	=> 0,
							 );

					if ($prefix == 'g') {

						// ipb_user_group => ipb_all_users
						$links['majorid']			= $this->id.':'.'g_0';
						$links['major_type_code']	= 'ipb_all_users';
						$links['minorid']			= $this->id.':'.$assetid;

					} else if ($prefix == 'u') {

						// ipb_user => ipb_user_group
						$ipb =& $this->getIpbConn();
						$tmp = explode('_', $assetid);
						$user_id = $tmp[1];
						$group = $ipb->getMembers($user_id);
						$links['majorid']			= $this->id.':g_'.$group['mgroup'];
						$links['major_type_code']	= 'ipb_user_group';
						$links['minorid']			= $this->id.':'.$assetid;

					} else if ($prefix == 'r') {

						// ipb_root_forum => ipb_forum (Category)
						$links['majorid']			= $this->id.':'.'f_0';
						$links['major_type_code']	= 'ipb_forum';
						$links['minorid']			= $this->id.':'.$assetid;

					} else if ($prefix == 'f') {

						// ipb_forum => ipb_root_forum
						$ipb =& $this->getIpbConn();
						$forum_info = $ipb->getForumInfo($this->id.':'.$assetid);
						$links['majorid']			= $this->id.':'.'r_'.$forum_info['parent_id'];
						$links['major_type_code']	= 'ipb_root_forum';
						$links['minorid']			=  $this->id.':'.$assetid;

					} else if ($prefix == 't') {

						// ipb_topic => ipb_forum
						$ipb =& $this->getIpbConn();
						$forum_info = $ipb->getTopicInfo($this->id.':'.$assetid);
						$links['majorid']			= $this->id.':'.'f_'.$forum_info['forum_id'];
						$links['major_type_code']	= 'ipb_forum';
						$links['minorid']			= $this->id.':'.$assetid;

					} else if ($prefix == 'p') {
						// ipb_topic => ipb_forum
						$ipb =& $this->getIpbConn();
						$forum_info = $ipb->getPostInfo($this->id.':'.$assetid);
						$links['majorid']			= $this->id.':'.'t_'.$forum_info['tid'];
						$links['major_type_code']	= 'ipb_topic';
						$links['minorid']			= $this->id.':'.$assetid;

					} else {
						$links = Array();
					}

				}//end else

			}//end else

			// put in $_tmp
			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;

		}//end if

		return $this->_tmp['getLinks'][$assetid][$side_of_link];

	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
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
		$no_id_parts = FALSE;
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$assetid = $id_parts[1];
		} else {
			$no_id_parts = TRUE;
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		if (!isset($this->_tmp['getParents'][$assetid])) {

			$prefix = substr($assetid, 0, 1);
			if (array_key_exists($assetid, $this->top_categories)) {
				// nothing. top level
				$parents = Array();
			} else if ($prefix == 'g' || $prefix == 'r' || $prefix == 'o' ) {

				/////////////////////////////////
				// Right Under Top Categories
				/////////////////////////////////
				switch ($prefix) {
					case 'g' :
						$parents[$id_parts[0].':'.$prefix.'_0'] = 'ipb_user_group';
					break;
					case 'r' :
						$parents[$id_parts[0].':'.'f_0'] = 'ipb_forum';
					break;
					case 'o' :
						$parents[$id_parts[0].':'.$prefix.'_0'] = 'ipb_poll';
					break;
				}

			} else {

				$ipb =& $this->getIpbConn();
				if ($prefix == 'u') {
					// IPB USER
					$parents[$id_parts[0].':'.'g_0'] = 'ipb_user_group';
					$result = $ipb->getMemberParent($id_parts[0].':'.$assetid);
					$parents[$id_parts[0].':'.'g_'.$result['mgroup']] = 'ipb_user_group';

				} else if ($prefix == 'f') {
					// IPB FORUM
					$parents[$id_parts[0].':'.'f_0'] = 'ipb_forum';
					$result = $ipb->getForumParent($id_parts[0].':'.$assetid);
					$parents[$id_parts[0].':'.'r_'.$result['parent_id']] = 'ipb_root_forum';

				} else if ($prefix == 't') {
					// IPB TOPIC
					$parents[$id_parts[0].':'.'f_0'] = 'ipb_forum';
					$result = $ipb->getTopicParent($id_parts[0].':'.$assetid);
					$parents[$id_parts[0].':'.'r_'.$result['parent_id']] = 'ipb_root_forum';
					$parents[$id_parts[0].':'.'f_'.$result['forum_id']] = 'ipb_forum';

				} else if ($prefix == 'p') {
					// IPB POST
					$parents[$id_parts[0].':'.'f_0'] = 'ipb_forum';
					$result = $ipb->getPostParent($id_parts[0].':'.$assetid);
					$parents[$id_parts[0].':'.'r_'.$result['parent_id']] = 'ipb_root_forum';
					$parents[$id_parts[0].':'.'f_'.$result['forum_id']] = 'ipb_forum';
					$parents[$id_parts[0].':'.'t_'.$result['topic_id']] = 'ipb_topic';
				}

			}//end else
			$this->_tmp['getParents'][$assetid] = $parents;

		}//end if

		return $this->_tmp['getParents'][$assetid];

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param string			$assetid			the id of the asset to get its children for
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
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		$children = Array();
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) {
			$assetid = $id_parts[1];
		} else {
			$links = $this->getLinks($assetid, SQ_LINK_TYPE_1, '', TRUE, 'major');
			foreach ($links as $link) {
				if (empty($type_code) || (!empty($type_code) && (array_search($link['minor_type_code'], $type_code) !== FALSE))) {
					$children[$link['minorid']] = $link['minor_type_code'];
				}
				$children = array_merge($children, $this->getChildren($link['minorid'], $type_code, $strict_type_code, $dependant, $sort_by));
			}
			return $children;
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}
		$no_type_code = empty($type_code);

		if (!isset($this->_tmp['getChildren'][$assetid])) {
			// ipb_user, no children
			$prefix = substr($assetid, 0, 1);
			$suffix = substr($assetid, -1);
			$is_top_category = strlen($assetid) == 3 && $suffix == '0';

			$ipb =& $this->getIpbConn();
			if ($prefix == 'g') {
				if ($is_top_category) {
					// all groups + all users
					$all_groups = Array();
					$all_members = Array();
					$add_user_group	= (array_search('ipb_user_group', $type_code) !== FALSE);
					$add_user		= (array_search('ipb_user', $type_code) !== FALSE);
					if ($no_type_code || (!$no_type_code && $add_user_group)) {
						$all_groups = $ipb->getListOfGroups();
					}
					if ($no_type_code || (!$no_type_code && $add_user)) {
						$all_members = $ipb->getListOfMembers();
					}

					foreach ($all_groups as $group) {
						$children[$this->id.':g_'.$group['g_id']] = 'ipb_user_group';
					}
					foreach ($all_members as $member) {
						$children[$this->id.':u_'.$member['id']] = 'ipb_user';
					}
				} else {
					// all users of the one group
					$tmp = explode('_', $assetid);
					$group_members = Array();
					$add_user = (array_search('ipb_user', $type_code) !== FALSE);
					if ($no_type_code || (!$no_type_code && $add_user)) {
						$group_members = $ipb->getListOfMembers($this->id.':'.$assetid);
					}
					foreach ($group_members as $member) {
						$children[$this->id.':u_'.$member['id']] = 'ipb_user';
					}
				}

			} else if ($prefix == 'f') {
				if ($is_top_category) {
					if ($no_type_code) {
						// no type code specified
						$all_root_forums = $ipb->getListOfRootForums();
						foreach ($all_root_forums  as $root_forum) {
							$children[$this->id.':r_'.$root_forum['id']] = 'ipb_root_forum';

							$all_forums = $ipb->getListOfForums('f_'.$root_forum['id']);
							foreach ($all_forums as $forum) {
								$children[$this->id.':f_'.$forum['id']] = 'ipb_forum';

								$all_topics = $ipb->getListOfTopics('f_'.$forum['id']);
								foreach ($all_topics as $topic) {
									$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';

									$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
									foreach ($all_posts as $post) {
										$children[$this->id.':p_'.$post['id']] = 'ipb_post';
									}
								}
							}
						}
					} else {
						// type code specified
						$add_ipb_root_forum	= (array_search('ipb_root_forum', $type_code) !== FALSE);
						$add_ipb_forum		= (array_search('ipb_forum', $type_code) !== FALSE);
						$add_ipb_topic		= (array_search('ipb_topic', $type_code) !== FALSE);
						$add_ipb_post		= (array_search('ipb_post', $type_code) !== FALSE);

						$all_root_forums = $ipb->getListOfRootForums();
						foreach ($all_root_forums  as $root_forum) {
							if ($add_ipb_root_forum) {
								$children[$this->id.':r_'.$root_forum['id']] = 'ipb_root_forum';
							}
							if ($strict_type_code && !$add_ipb_forum && !$add_ipb_topic && !$add_ipb_post) {
								continue;
							} else {
								$all_forums = $ipb->getListOfForums('f_'.$root_forum['id']);
								foreach ($all_forums as $forum) {
									if ($add_ipb_forum) {
										$children[$this->id.':f_'.$forum['id']] = 'ipb_forum';
									}
									if ($strict_type_code && !$add_ipb_topic && !$add_ipb_post) {
										continue;
									} else {
										$all_topics = $ipb->getListOfTopics('f_'.$forum['id']);
										foreach ($all_topics as $topic) {
											if ($add_ipb_topic) {
												$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';
											}
											if ($strict_type_code && !$add_ipb_post) {
												continue;
											} else {
												$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
												foreach ($all_posts as $post) {
													if ($add_ipb_post) {
														$children[$this->id.':p_'.$post['id']] = 'ipb_post';
													}
												}
											}
										}
									}
								}
							}
						}//end foreach
					}//end else
				} else {
					if ($no_type_code) {
						// topics + posts
						$all_topics = $ipb->getListOfTopics($assetid);
						foreach ($all_topics as $topic) {
							$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';

							$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
							foreach ($all_posts as $post) {
								$children[$this->id.':p_'.$post['id']] = 'ipb_post';
							}
						}
					} else {
						$add_ipb_topic		= (array_search('ipb_topic', $type_code) !== FALSE);
						$add_ipb_post		= (array_search('ipb_post', $type_code) !== FALSE);
						if ($strict_type_code && !$add_ipb_topic && !$add_ipb_post) {
							;
						} else {
							$all_topics = $ipb->getListOfTopics($assetid);
							foreach ($all_topics as $topic) {
								if ($add_ipb_topic) {
									$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';
								}
								if ($strict_type_code && !$add_ipb_post) {
									;
								} else {
									$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
									foreach ($all_posts as $post) {
										if ($add_ipb_post) {
											$children[$this->id.':p_'.$post['id']] = 'ipb_post';
										}
									}
								}
							}
						}
					}
				}//end else

			} else if ($prefix == 'r') {
				if ($no_type_code) {
					// forums + topics + posts
					$all_forums = $ipb->getListOfForums($assetid);
					foreach ($all_forums as $forum) {
						$children[$this->id.':f_'.$forum['id']] = 'ipb_forum';

						$all_topics = $ipb->getListOfTopics('f_'.$forum['id']);
						foreach ($all_topics as $topic) {
							$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';

							$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
							foreach ($all_posts as $post) {
								$children[$this->id.':p_'.$post['id']] = 'ipb_post';
							}
						}
					}
				} else {
					// forums + topics + posts
					// type code specified
					$add_ipb_forum		= (array_search('ipb_forum', $type_code) !== FALSE);
					$add_ipb_topic		= (array_search('ipb_topic', $type_code) !== FALSE);
					$add_ipb_post		= (array_search('ipb_post', $type_code) !== FALSE);
					if ($strict_type_code && !$add_ipb_forum && !$add_ipb_topic && !$add_ipb_post) {
						;
					} else {
						$all_forums = $ipb->getListOfForums($assetid);
						foreach ($all_forums as $forum) {
							if ($add_ipb_forum) {
								$children[$this->id.':f_'.$forum['id']] = 'ipb_forum';
							}
							if ($strict_type_code && !$add_ipb_topic && !$add_ipb_post) {
								continue;
							} else {
								$all_topics = $ipb->getListOfTopics('f_'.$forum['id']);
								foreach ($all_topics as $topic) {
									if ($add_ipb_topic) {
										$children[$this->id.':t_'.$topic['id']] = 'ipb_topic';
									}
									if ($strict_type_code && !$add_ipb_post) {
										continue;
									} else {
										$all_posts = $ipb->getListOfPosts('t_'.$topic['id']);
										foreach ($all_posts as $post) {
											if ($add_ipb_post) {
												$children[$this->id.':p_'.$post['id']] = 'ipb_post';
											}
										}
									}
								}
							}
						}
					}
				}//end else

			} else if ($prefix == 't') {
				// posts
				if ($no_type_code) {
					$all_posts = $ipb->getListOfPosts($assetid);
					foreach ($all_posts as $post) {
						$children[$this->id.':p_'.$post['id']] = 'ipb_post';
					}
				} else {
					$add_ipb_post		= (array_search('ipb_post', $type_code) !== FALSE);
					if ($strict_type_code && !$add_ipb_post) {
						;
					} else {
						$all_posts = $ipb->getListOfPosts($assetid);
						foreach ($all_posts as $post) {
							if ($add_ipb_post) {
								$children[$this->id.':p_'.$post['id']] = 'ipb_post';
							}
						}
					}
				}

			} else if ($prefix == 'o') {
				// polls
				if ($is_top_category) {
					if ($no_type_code) {
						$all_polls = $ipb->getListOfPolls();
						foreach ($all_polls as $poll) {
							$children[$this->id.':o_'.$poll['id']] = 'ipb_poll';
						}
					} else {
						$add_ipb_poll = (array_search('ipb_poll', $type_code) !== FALSE);
						if ($strict_type_code && !$add_ipb_poll) {
							;
						} else {
							$all_polls = $ipb->getListOfPolls();
							foreach ($all_polls as $poll) {
								if ($add_ipb_poll) {
									$children[$this->id.':o_'.$poll['id']] = 'ipb_poll';
								}
							}
						}
					}
				}
			}

			$this->_tmp['getChildren'][$assetid] = $children;

		}//end if

		return $this->_tmp['getChildren'][$assetid];

	}//end getChildren()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> NULL means it is ignored
	* @param string	$url		the urls to check for -> NULL defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		$lineage = Array();

		$shadow_ids = explode(',', $assetid);
		foreach ($shadow_ids as $id) {
			$asset =& $this->getAsset($this->id.':'.$id);
			$url = substr($asset->getURL(), strlen(current_protocol()) +3);
			$lineage[] = Array (
							'url'			=> $url,
							'assetid'		=> $this->id.':'.$id,
							'name'			=> $asset->name,
							'short_name'	=> $asset->short_name,
							'type_code'		=> $asset->type(),
							'link_type'		=> SQ_LINK_TYPE_1,
							'protocol'		=> current_protocol(),
						 );

		}
		return array_reverse($lineage);

	}//end getLineageFromURL()


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
			$asset =& $this->getAsset($assetid);

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
									'languages'				=> $this->languages,
									'charset'				=> $this->charset,
									'force_secure'			=> $this->force_secure,
									'created'				=> $this->created,
									'created_userid'		=> $this->created_userid,
									'updated'				=> $this->updated,
									'updated_userid'		=> $this->updated_userid,
									'published'				=> $this->published,
									'published_userid'		=> $this->published_userid,
									'status_changed'		=> $this->status_changed,
									'status_changed_userid'	=> $this->status_changed_userid,
								  );
			}

		}//end foreach

		return $info;

	}//end getAssetInfo()


//--        EMPTY FUNCTIONS        --//


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
	* Returns TRUE if the supplied assets can be linked in the specified way
	*
	* @param object	&$major		the major asset in the proposed link
	* @param object	&$minor		the minor asset in the proposed link
	* @param string	$link_type	the type of link that is proposed
	* @param int	$exclusive	(0|1) whether this will be the only significant link
	*							with $minor as the minor asset
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$major, &$minor, $link_type, $exclusive)
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
