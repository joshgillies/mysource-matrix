<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: ldap_bridge.inc,v 1.8 2004/04/02 05:04:21 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/bridge/bridge.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';

/**
* LDAP_Bridge
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge extends Bridge
{

	var $_group_types = Array('organizationalunit', 'group');
	var $_user_types = Array('organizationalperson');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function LDAP_Bridge($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Bridge($assetid);

	}//end constructor


	/**
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @return object Ldap
	* @access public
	*/
	function connectToLdap()
	{
		return new Ldap($this->attr('host'), $this->attr('port'), $this->attr('password'), $this->attr('bind_dn'), $this->attr('root_dn'));

	}//end connectToLdap()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the asset id to be loaded
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
		
		$ldap = $this->connectToLdap();
		$result = $ldap->search($shadowid,'(objectClass=*)');
		if (!$result) return $asset;
		$data = $ldap->getEntries($result);
		if (!$data['count']) return $asset;
		$data = $data[0];
		$ldap->disconnect();
	
		unset($data['objectclass']['count']);	
		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			if (in_array($class, $this->_group_types)) {
				// an LDAP user group
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user_group');
				$asset = new Ldap_User_Group($this->id, $data);
			} else if (in_array($class, $this->_user_types)) {
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
		if (!isset($this->_tmp['getLinks'][$assetid][$side_of_link])) {

			$ldap = $this->connectToLdap();

			if ($assetid == $this->id) $base_dn = $this->attr('root_dn');
			else $base_dn = $assetid;
			
			$result = $ldap->search($base_dn,'(objectClass=*)', false);
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

			$links = Array();

			// the attribute mappings to the LDAP schemas
			$attrs = $this->attr('attributes');

			$i = 0;
			foreach ($entries as $data) {

				$type = '';	
				unset($data['objectclass']['count']);
				foreach ($data['objectclass'] as $class) {
					$class = strtolower($class);
					switch ($class) {
						case 'organizationalunit' :
							if (!isset($data[$attrs['ou']][0])) continue(2);
							$name = $data[$attrs['ou']][0];
							$type = 'ldap_user_group';
						break;
						case 'group' :
							if (!isset($data[$attrs['cn']][0])) continue(2);
							$name = $data[$attrs['cn']][0];
							$type = 'ldap_user_group';
						break;
						case 'organizationalperson' :
							if (!isset($data[$attrs['cn']][0])) continue(2);
							$name = $data[$attrs['cn']][0];
							$type = 'ldap_user';
						break;
					}
					if (!empty($type)) break;
				}
				if (empty($type)) continue;

				$links[] = Array('linkid' => 0,
								 'majorid' => $this->id,
								 'minorid' => $this->id.':'.$data['dn'],
								 'minor_type_code' => $type,
								 'value' => '',
								 'link_type' => SQ_LINK_TYPE_1,
								 'dependant' => false,
								 'exclusive' => false,
								 'sort_order' => $i,
								);
				$i++;
			}

			$ldap->disconnect();

			$this->_tmp['getLinks'][$assetid][$side_of_link] = $links;
		}
		
		return $this->_tmp['getLinks'][$assetid][$side_of_link];

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
			$ldap = $this->connectToLdap();
			$result = $ldap->search($assetid,'(objectClass=*)');
			if (!$result) return $asset;
			
			$data = $ldap->getEntries($result);
			if (!$data['count']) {
				$ldap->disconnect();
				return Array();
			}
			$data = $data[0];

			// the attribute mappings to the LDAP schemas
			$attrs = $this->attr('attributes');

			$groups = $data[$attrs['ou']];
			if (!$groups['count']) {
				$ldap->disconnect();
				return Array();
			}
			unset($groups['count']);

			foreach ($groups as $group_name) {
				switch ($this->attr('system_type')) {
					case 'openldap' :
						$result = $ldap->search($this->attr('root_dn'),'(&(objectClass=organizationalUnit)('.$attrs['ou'].'='.$group_name.'))');
					break;
					case 'activedirectory' :
						$result = $ldap->search($group_name, '(objectClass=*)');
					break;
				}
				$group_data = $ldap->getEntries($result);
				if ($group_data['count'] > 0) $parents[] = $this->id.':'.$group_data[0]['dn'];
			}

			$ldap->disconnect();

			$this->_tmp['getParents'][$assetid] = $parents;
		}
		
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
		if (!is_array($type_code)) {
			if (empty($type_code)) $type_code = Array();
			else $type_code = Array($type_code);
		}
		$ldap = $this->connectToLdap();

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
					if ($class == 'organizationalunit') $attr = 'ou';
					else if ($class == 'group') $attr = 'cn';
					else if ($class == 'organizationalperson') $attr = 'cn';
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
				if (in_array($class, $this->_group_types)) {
					if (!empty($ype_code) && !in_array('user_group', $type_code)) continue(2);
					$children[$this->id.':'.$data['dn']] = 'ldap_user_group';
				} else if (in_array($class, $this->_user_types)) {
					if (!empty($ype_code) && !in_array('user', $type_code)) continue(2);
					$children[$this->id.':'.$data['dn']] = 'ldap_user';
				}
			}
		}

		$ldap->disconnect();

		return $children;

	}//end getChildren()


}//end class

?>
