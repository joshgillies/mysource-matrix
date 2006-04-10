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
* $Id: ldap_bridge.inc,v 1.30.2.2 2006/04/10 22:42:46 sdanis Exp $
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
* @version $Revision: 1.30.2.2 $
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
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function LDAP_Bridge($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @return object Ldap
	* @access public
	*/
	function connectToLdap()
	{
		$ldap = new Ldap();
		if (!$ldap->connect($this->attr('host'), $this->attr('port'))) {
			return null;
		}

		if ($this->attr('system_type') == 'openldap3') {
			// set the protocol version for openLDAP (protocol v3)
			ldap_set_option($ldap->ptr, LDAP_OPT_PROTOCOL_VERSION, 3);
		}

		if (!$ldap->bind($this->attr('bind_dn'), $this->attr('password'))) {
			return null;
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
	* @return object Asset or null on error
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;
		if (empty($shadowid)) return $asset;

		$ldap = @$this->connectToLdap();
			if (is_null($ldap->ptr)) return $asset;

			$result = $ldap->search($shadowid,'(objectClass=*)', null, false, false);
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
				$asset = new Ldap_User_Group($this->id, $data);
			} else if (isset($this->_user_types[$class])) {
				// an LDAP user
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user');
				$asset = new LDAP_User($this->id, $data);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int			$assetid			id of the the $side_of_link asset
	* @param int			$link_types			integer that can be the product of bitwise operations
	*											on the SQ_LINK_* constants
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$side_of_link		Which side of the link this (the current) asset
	*											is on ('major' or 'minor')
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $sort_by=null)
	{
		if (!is_array($type_code)) {
			if (empty($type_code)) $type_code = Array();
			else $type_code = Array($type_code);
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
					$links[] = Array('linkid'          => 0,
									 'majorid'         => $this->id,
									 'major_type_code' => $this->type(),
									 'minorid'         => $this->id.':'.$assetid,
									 'value'           => '',
									 'link_type'       => SQ_LINK_TYPE_1,
									 'is_dependant'    => false,
									 'is_exclusive'    => false,
									 'sort_order'      => 0,
									);
				} else {
					$asset = &$this->getAsset($minorid);

					if (!empty($type_code)) {
						if ($asset->type() == 'ldap_user_group') {
							if (!in_array('user_group', $type_code) && !in_array('ldap_user_group', $type_code)) return Array();
						} else {
							if (!in_array('user', $type_code) && !in_array('ldap_user', $type_code)) return Array();
						}
					}

					$links[] = Array('linkid'          => 0,
									 'majorid'         => $asset->id,
									 'major_type_code' => $asset->type(),
									 'minorid'         => $this->id.':'.$assetid,
									 'value'           => '',
									 'link_type'       => SQ_LINK_TYPE_1,
									 'is_dependant'    => false,
									 'is_exclusive'    => false,
									 'sort_order'      => 0,
									);
				}

			} else {

				$entries = null;

				if ($assetid != $this->id) {
					// if we are using Active Directory, we have to get the members of a group from the member
					// attribute of the group instead of using tree searches
					if ($this->attr('system_type') == 'activedirectory') {
						$asset = &$this->getAsset($assetid);
						if ($asset->type() == 'ldap_user_group' && in_array('group', $asset->vars['objectclass']) && isset($asset->vars['member'])) {
							$members = $asset->vars['member'];
							$entries = Array();
							foreach ($members as $dn) {
								$user = &$this->getAsset($dn);
								$entries[$dn] = $user->vars;
							}
						}
					}
				}

				if (is_null($entries)) {
					$ldap = @$this->connectToLdap();
					if (is_null($ldap->ptr)) return Array();

					if ($assetid == $this->id) $base_dn = $this->attr('root_dn');
					else $base_dn = $assetid;

					$filter = '';
					foreach ($this->_group_types as $key => $value) {
						if (empty($filter)) $filter = '(objectClass='.$key.')';
						else $filter = '(|'.$filter.'(objectClass='.$key.'))';
					}
					foreach ($this->_user_types as $key => $value) {
						if (empty($filter)) $filter = '(objectClass='.$key.')';
						else $filter = '(|'.$filter.'(objectClass='.$key.'))';
					}

					$user_attrs = $this->attr('user_attributes');
					$sort_by = (isset($user_attrs['sort_by'])) ? $user_attrs['sort_by'] : '';

					$result = $ldap->search($base_dn, $filter, $sort_by, false);
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
				}

				$links = Array();

				foreach ($entries as $data) {

					$type = '';
					if (isset($data['objectclass']['count'])) unset($data['objectclass']['count']);
					foreach ($data['objectclass'] as $class) {
						$class = strtolower($class);
						foreach ($this->_group_types as $key => $value) {
							if ($class == $key) {
								if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) continue(3);
								$name = $data[$attrs[$value]][0];
								$type = 'ldap_user_group';
							}
						}

						// if this container is not a group, check to see if it is a user
						if (empty($type)) {
							foreach ($this->_user_types as $key => $value) {
								if ($class == $key) {
									if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) continue(2);
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
					if (isset($data['objectclass']['count'])) unset($data['objectclass']['count']);
					foreach ($data['objectclass'] as $class) {
						$class = strtolower($class);
						foreach ($this->_group_types as $key => $value) {
							if ($class == $key) {
								if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) continue(3);
								$name = $data[$attrs[$value]][0];
								$type = 'ldap_user_group';
							}
						}

						// if this container is not a group, check to see if it is a user
						if (empty($type)) {
							foreach ($this->_user_types as $key => $value) {
								if ($class == $key) {
									if (!isset($attrs[$value]) || !isset($data[$attrs[$value]][0])) continue(2);
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
							if (!in_array('user_group', $type_code) && !in_array('ldap_user_group', $type_code)) continue;
						} else {
							if (!in_array('user', $type_code) && !in_array('ldap_user', $type_code)) continue;
						}
					}

					// find the DN for the asset
					if (is_array($data['dn'])) $dn = array_shift($data['dn']);
					else $dn = $data['dn'];

					$links[] = Array('linkid'          => 0,
									 'majorid'         => $this->id,
									 'minorid'         => $this->id.':'.$dn,
									 'minor_type_code' => $type,
									 'value'           => '',
									 'link_type'       => SQ_LINK_TYPE_1,
									 'is_dependant'    => false,
									 'is_exclusive'    => false,
									 'sort_order'      => $i,
									);
					$i++;

				}//end foreach entries

			}//end if side_of_link

			$this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key] = $links;
		}

		return $this->_tmp['getLinks'][$assetid][$side_of_link][$type_code_key];

	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* Can only find parents of type <i>user_group</i>
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array(int)
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=true)
	{
		$parents = Array();
		if (!is_array($type_code)) $type_code = Array($type_code);
		if (!in_array('user_group', $type_code)) return Array();

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
				if (!$mo_groups['count']) {
					$mo_groups = Array();
				}
				unset($mo_groups['count']);
			}

			// also add in groups based on our DN
			$ou_groups = Array();
			$matches = Array();
			$reg_expression = '|ou=([^,]+),(.*)|i';
			$reg_string = $assetid;
			while ($reg_string != '') {
				preg_match_all($reg_expression, $reg_string, $matches);
				if (empty($matches[1])) break;
				$reg_string = array_shift($matches[2]);
				$ou_groups[array_shift($matches[1])] = $reg_string;
			}

			foreach ($mo_groups as $group_name) {
				$result = $ldap->search($group_name, '(objectClass=*)');
				if (!$result) continue;
				$group_data = $ldap->getEntries($result);
				if ($group_data['count'] > 0) $parents[$this->id.':'.$group_data[0]['dn']] = 'ldap_user_group';
			}

			foreach ($ou_groups as $group_name => $group_dn) {
				$result = $ldap->search($attrs['ou'].'='.$group_name.','.$group_dn,'(&(objectClass=organizationalUnit)('.$attrs['ou'].'='.$group_name.'))');
				if (!$result) continue;
				$group_data = $ldap->getEntries($result);
				if ($group_data['count'] > 0) $parents[$this->id.':'.$group_data[0]['dn']] = 'ldap_user_group';
			}

			$ldap->disconnect();
			$this->_tmp['getParents'][$assetid] = $parents;

		}//end if not cached

		return $this->_tmp['getParents'][$assetid];

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param bool			$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array(int => string)	returns an array of assetids and their type code
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
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
			if (empty($type_code)) $type_code = Array();
			else $type_code = Array($type_code);
		}

		$ldap = @$this->connectToLdap();
		if (is_null($ldap->ptr)) return Array();

		if ($assetid == $this->id) $base_dn = $this->attr('root_dn');
		else $base_dn = $assetid;

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

				$sorted_array[strtolower($data[$attrs[$attr]][0])] = Array('dn' => $data['dn'],
																		   'objectclass' => $data['objectclass']
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
					if (!empty($type_code) && !in_array('user_group', $type_code)) continue(2);
					$children[$this->id.':'.$data['dn']] = 'ldap_user_group';
				} else if (isset($this->_user_types[$class])) {
					if (!empty($type_code) && !in_array('user', $type_code)) continue(2);
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
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the url to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		$lineage = Array();

		$groups = split(',', $assetid);
		for (reset($groups); null !== ($k = key($groups));) {
			$dn = implode(',', $groups);
			if ($dn == $this->attr('root_dn')) break;

			$asset = &$this->getAsset($dn);
			$url = substr($asset->getURL(), strlen(current_protocol()) +3);

			$lineage[] = Array (
							'url'        => $url,
							'assetid'    => $this->id.':'.$dn,
							'name'       => $asset->name,
							'short_name' => $asset->short_name,
							'type_code'  => $asset->type(),
							'link_type'  => SQ_LINK_TYPE_1,
							'protocol'   => current_protocol(),
							);

			unset($groups[$k]);
		}
		return array_reverse($lineage);

	}//end getLineageFromURL()


}//end class

?>
