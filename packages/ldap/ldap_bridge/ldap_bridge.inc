<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ldap_bridge.inc,v 1.38.2.1 2006/04/10 22:40:48 sdanis Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';

/**
* LDAP_Bridge
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.38.2.1 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
register_implementation('ldap_bridge', 'bridge');
class LDAP_Bridge extends Asset
{

	/**
	* An array of objectclass values that represent groups in LDAP
	* All values must be lowercase
	*
	* @var array(string => string)
	*/
	var $_group_types = Array(
							'organizationalunit'	=> 'ou',
							'group'					=> 'cn',
							'groupofnames'			=> 'cn',
							'domain'				=> 'dc',
							'container'				=> 'cn',
							'organization'			=> 'o',
						);

	/**
	* An array of objectclass values that represent users in LDAP
	* All values must be lowercase
	*
	* @var array(string)
	*/
	var $_user_types = Array(
						'organizationalperson'	=> 'cn',
						'inetorgperson'			=> 'cn',
					   );

	/**
	* The link type of assets linked underneath an LDAP_Bridge
	*
	* @var int
	*/
	var $ldap_shdw_link_type = SQ_LINK_TYPE_1;


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function LDAP_Bridge($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
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
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @return object
	* @access public
	*/
	function connectToLdap()
	{
		$ldap =& new Ldap();
		if (!$ldap->connect($this->attr('host'), $this->attr('port'))) {
			return NULL;
		}

		if ($this->attr('system_type') == 'openldap3') {
			// set the protocol version for openLDAP (protocol v3)
			ldap_set_option($ldap->ptr, LDAP_OPT_PROTOCOL_VERSION, 3);
		}

		if (!$ldap->bind($this->attr('bind_dn'), $this->attr('password'))) {
			return NULL;
		}
		return $ldap;

	}//end connectToLdap()


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
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $shadowid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		$ldap = @$this->connectToLdap();
			if (is_null($ldap->ptr)) return $asset;

			$result = $ldap->search($shadowid,'(objectClass=*)', NULL, FALSE, FALSE);
			if (!$result) return $asset;

			$data = $ldap->getEntries($result);
			if (!$data['count']) return $asset;

			$data = $data[0];
		$ldap->disconnect();

		unset($data['objectclass']['count']);
		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			if (isset($this->_group_types[$class])) {
				// an LDAP user group
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user_group');
				$asset =& new Ldap_User_Group($this->id, $data);
			} else if (isset($this->_user_types[$class])) {
				// an LDAP user
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user');
				$asset =& new LDAP_User($this->id, $data);
			}
		}

		return $asset;

	}//end getAsset()


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

		// if the wanted link type is not the type of links
		// that this bridge allows, lets return now
		if (!($this->ldap_shdw_link_type & $link_types)) {
			return Array();
		}

		if ($dependant || $exclusive) {
			return Array();
		}

		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) $assetid = $id_parts[1];

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}
		$type_code_key = implode('-', $type_code);

		// the attribute mappings to the LDAP schemas
		$attrs = $this->attr('attributes');

		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key])) {

			if ($side_of_link == 'minor') {

				// we'll get the direct parent above us using our DN
				$minorid = substr($assetid, strpos($assetid, ',') +1);
				if ($minorid == $this->attr('root_dn')) {
					// the parent is the bridge
					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'major_type_code'	=> $this->type(),
								'minorid'			=> $this->id.':'.$assetid,
								'value'				=> '',
								'link_type'			=> $this->ldap_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
							   );
				} else {
					$asset =& $this->getAsset($this->id.':'.$minorid);

					if (!empty($type_code)) {
						if ($asset->type() == 'ldap_user_group') {
							if (!in_array('user_group', $type_code) && !in_array('ldap_user_group', $type_code)) {
								return Array();
							}
						} else {
							if (!in_array('user', $type_code) && !in_array('ldap_user', $type_code)) {
								return Array();
							}
						}
					}

					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $asset->id,
								'major_type_code'	=> $asset->type(),
								'minorid'			=> $this->id.':'.$assetid,
								'value'				=> '',
								'link_type'			=> $this->ldap_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
							   );
				}

			} else {

				$entries = NULL;

				if ($assetid != $this->id) {
					// if we are using Active Directory, we have to get the members of a group from the member
					// attribute of the group instead of using tree searches
					if ($this->attr('system_type') == 'activedirectory') {
						$asset =& $this->getAsset($this->id.':'.$assetid);
						if ($asset->type() == 'ldap_user_group' && in_array('group', $asset->vars['objectclass']) && isset($asset->vars['member'])) {
							$members = $asset->vars['member'];
							$entries = Array();
							foreach ($members as $dn) {
								$user =& $this->getAsset($this->id.':'.$dn);
								$entries[$dn] = $user->vars;
							}
						}
					}
				}

				if (is_null($entries)) {
					$ldap = @$this->connectToLdap();
					if (is_null($ldap->ptr)) return Array();

					if ($assetid == $this->id) {
						$base_dn = $this->attr('root_dn');
					} else {
						$base_dn = $assetid;
					}

					$filter = '';
					foreach ($this->_group_types as $key => $value) {
						if (empty($filter)) {
							$filter = '(objectClass='.$key.')';
						} else {
							$filter = '(|'.$filter.'(objectClass='.$key.'))';
						}
					}
					foreach ($this->_user_types as $key => $value) {
						if (empty($filter)) {
							$filter = '(objectClass='.$key.')';
						} else {
							$filter = '(|'.$filter.'(objectClass='.$key.'))';
						}
					}

					$user_attrs = $this->attr('user_attributes');
					$sort_by = (isset($user_attrs['sort_by'])) ? $user_attrs['sort_by'] : '';

					$result = $ldap->search($base_dn, $filter, $sort_by, FALSE);
					if (!$result) {
						$ldap->disconnect();
						return Array();
					}

					$entries = $ldap->getEntries($result);
					if (!$entries['count']) {
						$ldap->disconnect();
						return Array();
					}
					unset($entries['count']);

					$ldap->disconnect();
				}//end if is_null

				$links = Array();

				foreach ($entries as $data) {

					$type = '';
					if (isset($data['objectclass']['count'])) {
						unset($data['objectclass']['count']);
					}

					foreach ($data['objectclass'] as $class) {
						$class = strtolower($class);
						foreach ($this->_group_types as $key => $value) {
							if ($class == $key) {
								if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) {
									continue(3);
								}

								$name = $data[$attrs[$value]][0];
								$type = 'ldap_user_group';
							}
						}

						// if this container is not a group, check to see if it is a user
						if (empty($type)) {
							foreach ($this->_user_types as $key => $value) {
								if ($class == $key) {
									if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) {
										continue(2);
									}

									$name = $data[$attrs[$value]][0];
									$type = 'ldap_user';
								}
							}
						}
						if (!empty($type)) break;
					}

				}//end foreach entries

				$i = 0;
				foreach ($entries as $data) {

					$type = '';
					if (isset($data['objectclass']['count'])) {
						unset($data['objectclass']['count']);
					}

					foreach ($data['objectclass'] as $class) {
						$class = strtolower($class);
						foreach ($this->_group_types as $key => $value) {
							if ($class == $key) {
								if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) {
									continue(3);
								}

								$name = $data[$attrs[$value]][0];
								$type = 'ldap_user_group';
							}
						}

						// if this container is not a group, check to $resultsee if it is a user
						if (empty($type)) {
							foreach ($this->_user_types as $key => $value) {
								if ($class == $key) {
									if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) {
										continue(2);
									}

									$name = $data[$attrs[$value]][0];
									$type = 'ldap_user';
								}
							}
						}
						if (!empty($type)) break;
					}

					if (empty($type)) continue;
					if (!empty($type_code)) {
						if ($type == 'ldap_user_group') {
							if (!in_array('user_group', $type_code) && !in_array('ldap_user_group', $type_code)) {
								continue;
							}
						} else {
							if (!in_array('user', $type_code) && !in_array('ldap_user', $type_code)) {
								continue;
							}
						}
					}

					// find the DN for the asset
					if (is_array($data['dn'])) {
						$dn = array_shift($data['dn']);
					} else {
						$dn = $data['dn'];
					}

					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$dn,
								'minor_type_code'	=> $type,
								'value'				=> '',
								'link_type'			=> $this->ldap_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $i,
							   );
					$i++;

				}//end foreach entries

			}//end else side_of_link

			$this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key] = $links;
		}//end if isset

		return $this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key];

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
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) $assetid = $id_parts[1];

		$parents = Array();
		if (!is_array($type_code)) {
			$type_code = Array($type_code);
		}

		if (!in_array('user_group', $type_code)) {
			return Array();
		}

		if (!isset($this->_tmp['getParents'][$assetid])) {
			$ldap = @$this->connectToLdap();
			if (is_null($ldap->ptr)) return Array();

			$result = $ldap->search($assetid,'(objectClass=*)');
			if (!$result) return Array();

			$data = $ldap->getEntries($result);
			if (!$data['count']) {
				$ldap->disconnect();
				return Array();
			}
			$data = $data[0];

			// the attribute mappings to the LDAP schemas
			$attrs = $this->attr('attributes');

			$mo_groups = Array();
			if (isset($data[$attrs['mo']])) {
				$mo_groups = $data[$attrs['mo']];
				if (!$mo_groups['count']) $mo_groups = Array();
				unset($mo_groups['count']);
			}

			// also add in groups based on our DN
			$ou_groups = Array();
			$matches = Array();
			$reg_expression = '|ou=([^,]+),(.*)|i';
			$reg_string = substr($assetid, strpos($assetid, ',') + 1);

			preg_match_all($reg_expression, $reg_string, $matches);
			if (!empty($matches[1])) {
				$reg_string = array_shift($matches[2]);
				$ou_groups[array_shift($matches[1])] = $reg_string;
			}

			if (empty($ou_groups)) return Array();

			foreach ($mo_groups as $group_name) {
				$result = $ldap->search($group_name, '(objectClass=*)');
				if (!$result) continue;
				$group_data = $ldap->getEntries($result);
				if ($group_data['count'] > 0) {
					$parents[$this->id.':'.$group_data[0]['dn']] = 'ldap_user_group';
				}
			}

			foreach ($ou_groups as $group_name => $group_dn) {
				$result = $ldap->search($attrs['ou'].'='.$group_name.','.$group_dn,'(&(objectClass=organizationalUnit)('.$attrs['ou'].'='.$group_name.'))');
				if (!$result) continue;
				$group_data = $ldap->getEntries($result);

				if ($group_data['count'] > 0) {
					$parents[$this->id.':'.$group_data[0]['dn']] = 'ldap_user_group';
				}
			}

			foreach (array_keys($parents) as $parentid) {
				$real_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($parentid, $type_code, $strict_type_code);
				$parents = $real_parents + $parents;
			}

			$ldap->disconnect();
			$this->_tmp['getParents'][$assetid] = $parents;

		}//end if not cached


		return $this->_tmp['getParents'][$assetid];

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
		$id_parts = explode(':', $assetid);
		if (isset($id_parts[1])) $assetid = $id_parts[1];

		if ($assetid != $this->id) {
			// if we are using Active Directory, the tree structure does not quite work the same
			// as openLDAP, so we can only do one level for now
			if ($this->attr('system_type') == 'activedirectory') {
				$links = $this->getLinks($assetid, SQ_LINK_TYPE_1 & SQ_LINK_TYPE_2, $type_code, $strict_type_code, 'major', $sort_by);

				$children = Array();
				foreach ($links as $link_data) {
					$children[$link_data['minorid']] = $link_data['minor_type_code'];
				}
				return $children;
			}
		}

		if (!is_array($type_code)) {
			if (empty($type_code)) {
				$type_code = Array();
			} else {
				$type_code = Array($type_code);
			}
		}

		$ldap = @$this->connectToLdap();
		if (is_null($ldap->ptr)) return Array();

		if ($assetid == $this->id) {
			$base_dn = $this->attr('root_dn');
		} else {
			$base_dn = $assetid;
		}

		// the attribute mappings to the LDAP schemas
		$attrs = $this->attr('attributes');

		$result = $ldap->search($base_dn,'(objectClass=*)');
		if (!$result) {
			$ldap->disconnect();
			return Array();
		}
		$entries = $ldap->getEntries($result);
		if (!$entries['count']) {
			$ldap->disconnect();
			return Array();
		}
		unset($entries['count']);

		if ($sort_by == 'name') {
			// we need to sort the entries by name
			$sorted_array = Array();
			foreach ($entries as $data) {

				$attr = '';

				unset($data['objectclass']['count']);
				foreach ($data['objectclass'] as $class) {
					$class = strtolower($class);
					foreach ($this->_group_types as $key => $value) {
						if ($class == $key) $attr = $value;
					}
					foreach ($this->_user_types as $key => $value) {
						if ($class == $key) $attr = $value;
					}
				}
				if (empty($attr)) continue;

				$sorted_array[strtolower($data[$attrs[$attr]][0])] = Array(
																		'dn'			=> $data['dn'],
																		'objectclass'	=> $data['objectclass'],
																	 );
			}//end foreach

			ksort($sorted_array, SORT_STRING);
			$entries = $sorted_array;
		}


		// now that we have an array in the format we wanted (sorted or not)
		// we can go through and construct our children array
		$children = Array();

		foreach ($entries as $data) {
			if ($data['dn'] == $assetid) continue;
			unset($data['objectclass']['count']);
			foreach ($data['objectclass'] as $class) {
				$class = strtolower($class);
				if (isset($this->_group_types[$class])) {
					if (!empty($type_code) && !in_array('user_group', $type_code)) {
						continue(2);
					}
					$children[$this->id.':'.$data['dn']] = 'ldap_user_group';
				} else if (isset($this->_user_types[$class])) {
					if (!empty($type_code) && !in_array('user', $type_code)) {
						continue(2);
					}
					$children[$this->id.':'.$data['dn']] = 'ldap_user';
				}
			}
		}

		$ldap->disconnect();

		return $children;

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
		$lineage = Array();

		$groups = split(',', $assetid);
		for (reset($groups); NULL !== ($k = key($groups));) {
			$dn = implode(',', $groups);
			if ($dn == $this->attr('root_dn')) break;

			$asset =& $this->getAsset($this->id.':'.$dn);
			$url = substr($asset->getURL(), strlen(current_protocol()) +3);

			$lineage[] = Array (
							'url'			=> $url,
							'assetid'		=> $this->id.':'.$dn,
							'name'			=> $asset->name,
							'short_name'	=> $asset->short_name,
							'type_code'		=> $asset->type(),
							'link_type'		=> SQ_LINK_TYPE_1,
							'protocol'		=> current_protocol(),
						 );

			unset($groups[$k]);
		}
		return array_reverse($lineage);

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
		}//end foreach assetids

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


}//end class

?>
