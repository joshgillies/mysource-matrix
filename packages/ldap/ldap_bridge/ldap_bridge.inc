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
* $Id: ldap_bridge.inc,v 1.119 2013/04/24 06:46:34 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';

/**
* LDAP_Bridge
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.119 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge extends Asset implements Bridge
{

	/**
	* An array of objectclass values that represent groups in LDAP
	* All values must be lowercase
	*
	* @var array(string => string)
	*/
	var $_group_types = Array(
							'organizationalunit'	=> 'ou',
							'posixgroup'			=> 'cn',
							'group'					=> 'cn',
							'groupofnames'			=> 'cn',
							'groupofuniquenames'	=> 'cn',
							'domain'				=> 'dc',
							'container'				=> 'cn',
							'organization'			=> 'o',
							'country'				=> 'c',
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
						'userproxy'				=> 'cn',
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
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @return object
	* @access public
	*/
	function connectToLdap()
	{
		// If the bridge is archived, make no attempt to connect
		if ($this->status == SQ_STATUS_ARCHIVED) {
			return NULL;
		}

		// A bit silly to connect to LDAP without a hostname, otherwise continue...
		$hostname = $this->attr('host');
		if (empty($hostname)) return NULL;

		$ldap = new Ldap();
		if (!$ldap->connect($this->attr('host'), $this->attr('port'))) {
			return NULL;
		}

		// any non-default ldap connection settings set here
		foreach ($this->attr('ldap_options') as $option => $value) {
			ldap_set_option($ldap->ptr, constant($option), $value);
		}

		// alias dereferencing setting here
		ldap_set_option($ldap->ptr, LDAP_OPT_DEREF, constant($this->attr('aliases')));

		// if the PHP version being used allows us to add the timeout option then do it
		if (PHP_VERSION_ID >= 50300) {
			ldap_set_option($ldap->ptr, LDAP_OPT_NETWORK_TIMEOUT, 10);
		}

		$bind_dn = $this->attr('bind_dn');

		// If the bind DN is empty, anonymous access is assumed, and the
		// password supplied will be ignored
		if (empty($bind_dn)) {
			$password = '';
		} else {
			$password = $this->attr('password');
		}

		if (!$ldap->bind($bind_dn, $password)) {
			return NULL;
		}
		return $ldap;

	}//end connectToLdap()


	/**
	* Returns an array of defined user-customisable LDAP connection options.
	*
	* The options below are customisable by the user. If the user chooses to
	* customise one of the below options, it's value will be set according
	* to the array below. If an option is not customised, the default is assumed.
	*
	* @return array
	* @access public
	*/
	function getConnectionOptions()
	{
		// an array of customisable ldap connection options to their customised value
		// defaults:
		// LDAP_OPT_PROTOCOL_VERSION default = 2
		// LDAP_OPT_REFERRALS = 1
		return Array(
				'LDAP_OPT_PROTOCOL_VERSION'	=> 3,
				'LDAP_OPT_REFERRALS'		=> 0,
			   );

	}//end getConnectionOptions()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$assetid		the entire asset id of the asset to be loaded
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	* @param boolean	$return_null	returns NULL if no user/user data found
	* @param boolean	$get_all_attr	if we being called from paintUserSetup() make sure we get all the attributes
	*									for the ldap users so that we can paint option for sys admin to show/hide them
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::getAsset()
	*/
	function getAsset($assetid, $type_code='', $mute_errors=FALSE, $return_null=FALSE, $get_all_attr = FALSE)
	{
		$asset = NULL;
		$id_parts = explode(':', $assetid);

		if (isset($id_parts[1])) {
			$shadowid = $id_parts[1];
		} else {
			return $asset;
		}

		// Setup user's inbox
		if (isset($id_parts[2]) && $id_parts[2] == 'inbox') {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('inbox');
			$inbox_asset = new Inbox();
			$inbox_asset->id = $this->id.':'.$shadowid.':inbox';
			$inbox_asset->name = 'Inbox';
			$inbox_asset->status = $this->status;
			return $inbox_asset;
		}

		$ldap = @$this->connectToLdap();
			if (is_null($ldap) || is_null($ldap->ptr)) return $return_null ? NULL : $this->_getDummyAsset($shadowid);
			$required_attrs = $this->attr('attributes');

			$shadowid = $this->decodeString($shadowid);

			if (!$get_all_attr) {
				$user_attrs = $this->attr('user_attributes');
				$sort_attr = (!empty($user_attrs['sort_by'])) ? Array($user_attrs['sort_by']) : Array();
				$attr_req = array_merge(
										array_keys(array_get_index($this->attr('user_attributes'), 'show', Array())),
										array_values($required_attrs),
										$sort_attr,
										Array('objectclass')
							);

				$result = $ldap->search($shadowid,'(objectClass=*)', NULL, FALSE, FALSE, array_values(array_unique($attr_req)));
			} else {
				$result = $ldap->search($shadowid,'(objectClass=*)', NULL, FALSE, FALSE);
			}

			if (!$result) return $return_null ? NULL : $this->_getDummyAsset($shadowid);
			$data = $ldap->getEntries($result);
			if (!$data['count']) return $return_null ? NULL : $this->_getDummyAsset($shadowid);

			$data = $data[0];
		$ldap->disconnect();

		if (!isset($data['dn']) && isset($data['distinguishedname'])) {
			$data['dn'] = $data['distinguishedname'];
		}

		unset($data['objectclass']['count']);

		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			if (isset($this->_group_types[$class])) {
				// an LDAP user group
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user_group');
				$asset = new Ldap_User_Group($this->id, $data);
				return $asset;

			} else if (isset($this->_user_types[$class])) {
				// include all three LDAP user assets, in case the type changes
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_backend_user');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_simple_edit_user');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_user');
				// LDAP user or LDAP Simple Edit User or LDAP backend user,
				// depending on the bridge setting
				$user_type = $this->attr('user_type');
				$asset = new $user_type($this->id, $data);
				return $asset;
			}
		}

		return $return_null ? NULL : $this->_getDummyAsset($shadowid);

	}//end getAsset()


	/**
	* Used by LDAP_Bridge::getAsset() to return a dummy asset when it fails to load the LDAP user.
	*
	* @param string	$shadowid
	*
	* @return mixed object|NULL
	* @access protected
	*/
	protected function _getDummyAsset($shadowid)
	{
		// return 'unknown ldap user' when we cannot find a match
		$data['objectclass'] = Array();
		$data['dn'] = $shadowid;
		$user_type = $this->attr('user_type');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
		$asset = new $user_type($this->id, $data);
		return $asset; // this should return a dummy

	}//end _getDummyAsset()


	/**
	* Returns an array of assetid => inherited_types for assets of the passed codes
	*
	* @param array		$assetids			an array of assets to limit
	* @param array		$type_code			an array of types codes to limit the results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	*
	* @return array
	* @access public
	*/
	function getAssetTypeInfo($assetids, $type_code=Array(), $strict_type_code=TRUE)
	{
		if (!is_array($assetids)) {
			$assetids = Array($assetids);
		}
		assert_type($type_code, 'array');
		if (empty($assetids)) return Array();

		$results = Array();
		$type_codes = Array();

		// filter assets and get their type info
		$type_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($assetids, $type_code, $strict_type_code, 'type_code');

		foreach ($type_infos as $assetid => $type_code) {
			if (!isset($type_codes[$type_code])) {
				$sql = 'SELECT inhd_type_code FROM sq_ast_typ_inhd WHERE type_code = :type_code ORDER BY inhd_type_code_lvl DESC';
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'type_code', $type_code);
				$result = MatrixDAL::executePdoAssoc($query, 0);
				$type_codes[$type_code] = $result;
			}

			$results[$assetid] = $type_codes[$type_code];
		}

		ksort($results);

		return $results;

	}//end getAssetTypeInfo()


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
		$ldap = @$this->connectToLdap();
		if (is_null($ldap) || is_null($ldap->ptr)) {
			return Array();
		}

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
								'locked'			=> 0,
							   );

				} else {
					$asset = $this->getAsset($this->id.':'.$minorid);

					if (!empty($type_code)) {
						if ($asset->type() == 'ldap_user_group') {
							if (!((in_array('ldap_user_group', $type_code)) || (!$strict_type_code && in_array('user_group', $type_code)))) {
								return Array();
							}
						} else if ($asset->type() == 'ldap_backend_user') {
							if (!((in_array('ldap_backend_user', $type_code)) || (!$strict_type_code && in_array('user', $type_code)) || (!$strict_type_code && in_array('ldap_user', $type_code)))) {
								return Array();
							}
						} else if ($asset->type() == 'ldap_backend_user') {
							if (!((in_array('ldap_simple_edit_user', $type_code)) || (!$strict_type_code && in_array('user', $type_code)) || (!$strict_type_code && in_array('ldap_user', $type_code)))) {
								return Array();
							}
						} else if ($asset->type() == 'ldap_user') {
							if (!((in_array('ldap_user', $type_code)) || (!$strict_type_code && in_array('user', $type_code)))) {
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
								'locked'			=> 0,
							   );
				}

			} else {

				$entries = NULL;

				$attr = $this->attr('attributes');
				$user_attrs = $this->attr('user_attributes');
				$sort_by = (!empty($user_attrs['sort_by'])) ? $user_attrs['sort_by'] : NULL;

				// If the group defines its own members, use the "Group Members" attribute to
				// provide links to its members. Note that this is mandatory in Active Directory
				// so if nothing is provided, the default "member" will be used.
				if (($this->attr('system_type') == 'activedirectory') && empty($attr['gm'])) {
					$attr['gm'] = 'member';
				}
				if ($assetid != $this->id) {
					$asset = $this->getAsset($this->id.':'.$assetid);
					if ($asset->type() == 'ldap_user_group' && !empty($attr['gm']) && isset($asset->vars[$attr['gm']])) {
						$members = $asset->vars[$attr['gm']];
						$entries = Array();
						foreach ($members as $member) {
							// If members are stored as "member" then it represents member's dn
							if ($attr['gm'] == 'member') {
								$dn = $member;
							} else {
								// Otherwise if its stored as "memberUid" then we have the uid
								// So get the memeber's dn from the "uid"
								$base_dn = $this->decodeString($this->attr('root_dn'));
								$member_result = $ldap->search($base_dn, '(uid='.$member.')', NULL, TRUE, TRUE, array_merge(array_values($attrs), Array('objectclass')));
								if ($member_result) {
									$member_entry = $ldap->getEntries($member_result);
									// There got be just one entry for the given uid
									if ($member_entry['count'] != 1 || !isset($member_entry[0]['dn'])) {
										continue;
									}
									$dn = $member_entry[0]['dn'];
								}
							}
							$user = $this->getAsset($this->id.':'.$dn, '', FALSE, TRUE);
							// dn of user group was stripped out in construction, but we need it this time
							if (!is_null($user)){
								$user->vars['dn'] = $dn;
								$entries[$dn] = $user->vars;
							}
						}
						if (!empty($sort_by)){
							uasort($entries, create_function('$a,$b',
												'$a = (isset($a[\''.$sort_by.'\'][\'value\'])) ? $a[\''.$sort_by.'\'][\'value\'] : "";
												$b = (isset($b[\''.$sort_by.'\'][\'value\'])) ? $b[\''.$sort_by.'\'][\'value\'] : "";
												if ($a == $b) {
													return 0;
												}
												return ($a < $b) ? -1 : 1;'
											)
									);
						}
					}
				}

				if (is_null($entries)) {
					$ldap = @$this->connectToLdap();
					if (is_null($ldap) || is_null($ldap->ptr)) return Array();

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

					$base_dn = $this->decodeString($base_dn);

					// Attributes representing the objectclass values are always required + attribute used for sorting
					$sort_attr = (!empty($sort_by)) ? Array($sort_by) : Array();
					$required_attrs = array_values(array_unique(array_merge($attr, $sort_attr, Array('c', 'o', 'ou', 'cn', 'dc', 'objectclass'))));
					$result = $ldap->search($base_dn, $filter, $sort_by, FALSE, TRUE, $required_attrs);

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

				$i = 0;
				foreach ($entries as $data) {

					$type = '';
					if (isset($data['objectclass']['count'])) {
						unset($data['objectclass']['count']);
					}

					if (isset($data['objectclass']['value'])) {
						$data['objectclass'] = $data['objectclass']['value'];
					}

					if (!is_array($data['objectclass'])) {
						$data['objectclass'] = Array($data['objectclass']);
					}

					foreach ($data['objectclass'] as $class) {
						$class = strtolower($class);
						foreach ($this->_group_types as $key => $value) {
							if ($class == $key) {
								if (!isset($data[$value][0])) {
									continue(3);
								}

								$name = $data[$value][0];
								$type = 'ldap_user_group';
							}
						}

						// if this container is not a group, check the $result to see if it is a user
						if (empty($type)) {
							foreach ($this->_user_types as $key => $value) {
								if ($class == $key) {
									if (!isset($attrs[$value])) {
										$attrs[$value] = $value;
									}

									if ((!isset($data[$attrs[$value]][0]) && !isset($data[$attrs[$value]]['value']))) {
										continue(2);
									}

									if (isset($data[$attrs[$value]]['value']) && isset($data[$attrs[$value]]['value'][0])) {
										$name = $data[$attrs[$value]]['value'][0];
									} else {
										$name = $data[$attrs[$value]][0];
									}
									$type = $this->attr('user_type');
								}
							}
						}
						if (!empty($type)) break;
					}//end foreach

					if (empty($type)) continue;
					if (!empty($type_code)) {

						if ($type == 'ldap_user_group') {
							if (!in_array('user_group', $type_code) && !in_array('ldap_user_group', $type_code)) {
								continue;
							}
						} else {
							// work out the type code we want to list
							if ($this->attr('user_type') == 'ldap_backend_user') {
								$all_types = Array(
												'user',
												'ldap_user',
												'ldap_simple_edit_user',
												'ldap_backend_user',
											 );
							} else if ($this->attr('user_type') == 'ldap_simple_edit_user') {
								$all_types = Array(
												'user',
												'ldap_user',
												'ldap_simple_edit_user',
											 );
							} else {
								// ldap_user
								$all_types = Array(
												'user',
												'ldap_user',
											 );
							}
							$found = array_intersect($all_types, $type_code);
							if (empty($found)) continue;
						}

					}

					// find the DN for the asset
					// make sure we have dn
					if (!isset($data['dn']) && isset($data['distinguishedname'])) {
						$data['dn'] = $data['distinguishedname'];
					}

					if (is_array($data['dn'])) {
						$dn = array_shift($data['dn']);
					} else {
						$dn = $data['dn'];
					}

					$links[] = Array(
								'linkid'			=> 0,
								'majorid'			=> $this->id,
								'minorid'			=> $this->id.':'.$this->encodeString($dn),
								'minor_type_code'	=> $type,
								'value'				=> '',
								'link_type'			=> $this->ldap_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> $i,
								'locked'			=> 0,
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

		// Bug Fix #4487 - allowing 'ldap_user_group' as a valid type code to return data
		$valid_types = Array('user_group', 'ldap_user_group');
		$found_type = array_intersect($valid_types, $type_code);

		if (empty($found_type)) {
			return Array();
		}

		if (!isset($this->_tmp['getParents'][$assetid])) {
			$ldap = @$this->connectToLdap();
			if (is_null($ldap) || is_null($ldap->ptr)) return Array();

			// the attribute mappings to the LDAP schemas
			$attrs = $this->attr('attributes');
			$assetid = $this->decodeString($assetid);
			
			$required_attrs = Array('objectclass');
			if(isset($attrs['mo'])) $required_attrs[] = $attrs['mo'];
			if(isset($attrs['uid'])) $required_attrs[] = $attrs['uid'];
			
			$result = $ldap->search($assetid,'(objectClass=*)', NULL, FALSE, FALSE, $required_attrs);
			if (!$result) return Array();

			$data = $ldap->getEntries($result);
			if (!$data['count']) {
				$ldap->disconnect();
				return Array();
			}
			$data = $data[0];

			$mo_groups = Array();

			if (isset($attrs['mo']) && isset($data[$attrs['mo']])) {
				// User defines the groups it is a part of
				$mo_groups = $data[$attrs['mo']];
				if (!$mo_groups['count']) $mo_groups = Array();
				unset($mo_groups['count']);
			} else if (isset($attrs['gm'])) {
				// Group defines its members
				$base_dn = $this->attr('root_dn');
				$base_dn = $this->decodeString($base_dn);
				$filter = $ldap->escape_filter_value($assetid);
				// If group stores the memberUid (uid)
				if (strtolower($attrs['gm']) == 'memberuid' && !empty($data['uid'][0])) {
					$filter = $data['uid'][0];
				}

				$result = $ldap->search($base_dn, '('.$attrs['gm'].'='.$filter.')', NULL, TRUE, TRUE, Array('objectclass'));
				if ($result) {
					$groups = $ldap->getEntries($result);
					if (!$groups['count']) $mo_groups = Array();
					unset($groups['count']);
					foreach	($groups as $group_data) {
						$mo_groups[] = $group_data['dn'];
					}
				}
			}

			// also add in groups based on our DN
			$ou_groups = Array();
			$matches = Array();
			$reg_expression = '|ou=([^,]+),(.*)|i';
			$reg_string = substr($assetid, strpos($assetid, ',') + 1);

			$matching = TRUE;
			while ($matching){
				$matches = Array();
				preg_match_all($reg_expression, $reg_string, $matches);
				if (!empty($matches[1])) {
					$reg_string = array_shift($matches[2]);
					$ou_groups[array_shift($matches[1])] = $reg_string;
				} else {
					$matching = FALSE;
				}
			}

			foreach ($mo_groups as $group_name) {
				$group_name = $this->decodeString($group_name);
				$parents[$this->id.':'.$group_name] = 'ldap_user_group';
			}

			foreach ($ou_groups as $group_name => $group_dn) {
				$group_name = $this->decodeString($group_name);
				$ou_value = (isset($attrs['ou']) && $attrs['ou'] != '') ? $attrs['ou'] : 'ou';
				$parents[$this->id.':'.$ou_value.'='.$group_name.','.$group_dn] = 'ldap_user_group';
			}

			$ldap->disconnect();

			if (!empty($parents)) {
				// Initialise the list of all parents to prevent infinite recursive loop.
				// As we use the _tmp attribute of this object to store the list, we need
				// a flag to unset it when we finish. Otherwise, the list will be used for
				// the next clean getParents() method call and can cause incorrect results.
				$bridge_getParents_first_level = FALSE;
				if (!isset($this->_tmp['all_parents'])) {
					// Include the asset ID so that it will not be the parent of itself
					$this->_tmp['all_parents'] = Array($this->id.':'.$assetid => '');
					$bridge_getParents_first_level = TRUE;
				}

				// Filter the parents against the list of all parents so that we will not need
				// to process a parent asset twice. That will also prevent infinite recursive loop.
				$valid_parents = Array();
				foreach ($parents as $parent_id => $parent_type_code) {
					if (!isset($this->_tmp['all_parents'][$parent_id])) {
						$valid_parents[$parent_id] = $parent_type_code;
					}
				}

				// Assign valid parents to the parent list.
				$parents = $valid_parents;

				if (!empty($valid_parents)) {
					// Add the valid parents to the list of all parents now to make them
					// available in the recursive getParents() invocation below.
					$this->_tmp['all_parents'] += $valid_parents;
					
					// should we get all recursive parents? this might be a slow process
					$get_all_parents = $this->attr('recursive_parents');
					
					// Get the grandparents of the valid parents recursively.
					$grandparents = Array();
					foreach ($valid_parents as $parent_id => $parent_type_code) {
						$grandparents += $GLOBALS['SQ_SYSTEM']->am->getParents($parent_id, $type_code, $strict_type_code, NULL, NULL, TRUE, NULL, NULL, !$get_all_parents);
					}

					// Add the grandparents to the parent list
					$parents += $grandparents;

				}

				if ($bridge_getParents_first_level) {
					// Unset the list of all parents to prevent it from being used by
					// the next first level call to this bridge's getParents() method.
					unset($this->_tmp['all_parents']);
				} else {
					// We do not want to cache the parents in the array $this->_tmp['getParents']
					// if they have been filtered against the list of all parents as they might be
					// different from the original ones returned from the first level getParents() call.
					return $parents;
				}
			}

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
				$links = $this->getLinks($assetid, SQ_LINK_TYPE_1 | SQ_LINK_TYPE_2, $type_code, $strict_type_code, 'major', $sort_by);

				$children = Array();
				foreach ($links as $link_data) {
					$children[$link_data['minorid']][0]['type_code'] = $link_data['minor_type_code'];
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
		if (is_null($ldap) || is_null($ldap->ptr)) return Array();

		if ($assetid == $this->id) {
			$base_dn = $this->attr('root_dn');
		} else {
			$base_dn = $assetid;
		}

		// the attribute mappings to the LDAP schemas
		$attrs = $this->attr('attributes');

		$base_dn = $this->decodeString($base_dn);
		$result = $ldap->search($base_dn,'(objectClass=*)', NULL, TRUE, TRUE, array_merge(array_values($attrs), Array('objectclass')));
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

		// Go through each linked entry, if its a user group see if has its members defined
		foreach($entries as $data) {
			if (empty($data['dn'])) {
				continue;
			}
			unset($data['objectclass']['count']);
			foreach ($data['objectclass'] as $class) {
				$class = strtolower($class);
				if (isset($this->_group_types[$class])) {
					$ldap_group = $this->getAsset($this->id.':'.$data['dn']);
					// If "Group Memebers" is set and group defines its memebers
					if (!empty($attrs['gm']) && isset($ldap_group->vars[$attrs['gm']])) {
						$members = $ldap_group->vars[$attrs['gm']];
						if (empty($entries)) {
							$entries = Array();
						}
						foreach ($members as $member) {
							// If members are stored as "memeber" then it represnts member's dn
							if ($attrs['gm'] == 'member') {
								$dn = $member;
							} else {
								// Otherwise if its stored as "memberUid" then we have the uid
								// So get the memeber's dn from the "uid"
								$base_dn = $this->decodeString($this->attr('root_dn'));
								$member_result = $ldap->search($base_dn, '(uid='.$member.')', NULL, TRUE, TRUE, array_merge(array_values($attrs), Array('objectclass')));
								if ($member_result) {
									$member_entry = $ldap->getEntries($member_result);
									// There got be just one entry for the given uid
									if ($member_entry['count'] != 1 || !isset($member_entry[0]['dn'])) {
										continue;
									}
									$dn = $member_entry[0]['dn'];
								}
							}
							$user = $this->getAsset($this->id.':'.$dn, '', FALSE, TRUE);
							// dn of user group was stripped out in construction, but we need it this time
							if (!is_null($user)){
								$user->vars['dn'] = $dn;
								$entries[] = $user->vars;
							}//end if
						}//end foreach
					}//end if
				}//end if
			}//end foreach
		}//end foreach

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

		// work out the type code we want to list
		if ($this->attr('user_type') == 'ldap_backend_user') {
			$all_types = Array(
							'user',
							'ldap_user',
							'ldap_simple_edit_user',
							'ldap_backend_user',
						 );
		} else if ($this->attr('user_type') == 'ldap_simple_edit_user') {
			$all_types = Array(
							'user',
							'ldap_user',
							'ldap_simple_edit_user',
						 );
		} else {
			// ldap_user
			$all_types = Array(
							'user',
							'ldap_user',
						 );
		}
		$user_groups = Array('user_group', 'ldap_user_group');
		$found = array_intersect($all_types, $type_code);
		$found_ug = array_intersect($user_groups, $type_code);

		foreach ($entries as $data) {
			if ($data['dn'] == $assetid) continue;

			if (isset($data['objectclass']['count'])) {
				unset($data['objectclass']['count']);
			}
			if (isset($data['objectclass']['value'])) {
				$data['objectclass'] = $data['objectclass']['value'];
			}
			if (!is_array($data['objectclass'])) {
				$data['objectclass'] = Array($data['objectclass']);
			}

			foreach ($data['objectclass'] as $class) {
				$class = strtolower($class);
				if (isset($this->_group_types[$class])) {
					if (!empty($type_code) && empty($found_ug)) {
						continue(2);
					}
					$children[$this->id.':'.$data['dn']][0]['type_code'] = 'ldap_user_group';
				} else if (isset($this->_user_types[$class])) {
					if (!empty($type_code) && empty($found)) {
						continue(2);
					}
					$children[$this->id.':'.$data['dn']][0]['type_code'] = $this->attr('user_type');
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

		$groups = explode(',', $assetid);
		for (reset($groups); NULL !== ($k = key($groups));) {
			$dn = implode(',', $groups);
			if ($dn == $this->attr('root_dn')) break;

			$asset = $this->getAsset($this->id.':'.$dn);
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
			if (!empty($field)) {
				if ($field == 'assetid') {
					$info[$assetid] = $assetid;
				} else if ($field == 'type_code') {
					$asset =& $this->getAsset($assetid);
					if (is_null($asset) === FALSE) {
						$info[$assetid] = $asset->type();
					} else {
						$info[$assetid] = '';
					}
				} else if (in_array($field , Array('version', 'name', 'short_name', 'status'))) {
					$asset =& $this->getAsset($assetid);
					if (is_null($asset) === FALSE) {
						$info[$assetid] = $asset->$field;
					} else {
						$info[$assetid] = '';
					}
				} else {
					$info[$assetid] = $this->$field;
				}
			} else {
				$asset =& $this->getAsset($assetid);
				if (is_null($asset) === FALSE) {
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
				} else {
					$info[$assetid] = Array();
				}
			}//end else
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
		if ($type_code=='inbox') {
			return $links = Array(
								'linkid'			=> 0,
								'majorid'			=> $assetid,
								'minor_type_code'	=> 'inbox',
								'minorid'			=> $assetid.':inbox',
								'value'				=> '',
								'link_type'			=> $this->ldap_shdw_link_type,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,
								'sort_order'		=> 0,
								'locked'			=> 0,
							);
		} else if ($type_code == 'user' && $side_of_link == 'minor') {
			$id_parts = explode(':', $assetid);
			if (isset($id_parts[2]) && $id_parts[2] == 'inbox') {
				return $links = Array(
									'linkid'			=> 0,
									'majorid'			=> $id_parts[0].':'.$id_parts[1],
									'major_type_code'	=> $this->attr('user_type'),
									'minorid'			=> $assetid,
									'value'				=> '',
									'link_type'			=> $this->ldap_shdw_link_type,
									'is_dependant'		=> FALSE,
									'is_exclusive'		=> FALSE,
									'sort_order'		=> 0,
									'locked'			=> 0,
								);
			}
		}

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


	/**
	* Delete Asset
	*
	* @param boolean	$release_lock	TRUE, to release lock
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock)
	{
		if (parent::delete($release_lock)) {
			$this->_deleteStoredContents();
			return TRUE;
		}

		return FALSE;

	}//end delete()


	/**
	* Deletes the contents of the database for this LDAP
	*
	* @return boolean
	* @access private
	*/
	function _deleteStoredContents()
	{

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = 'DELETE FROM sq_shdw_ast_lnk WHERE minorid LIKE :minorid';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'minorid', $this->id.':%');
		$result = MatrixDAL::execPdoQuery($query);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _deleteStoredContents()


	/**
	* Paints the user info based on the setting in the bridge
	*
	* @param array	$info	the attributes of the ldap_user or ldap_backend_user asset
	* @param object	&$o		the outputter class for the ldap_user or ldap_backend_user
	*						pass in a null obj to print details for frontend
	*
	* @return void
	* @access public
	*/
	function paintUserDetails($info, &$o)
	{
		$show = $this->attr('user_attributes');
		if (!isset($show['show'])) {
			$show = Array();
		} else {
			$show = $show['show'];
		}

		if (is_null($o)) {
			?><table border="0" cellspacing="2" cellpadding="2"><?php
		}

		// loop through each field to be shown
		foreach ($show as $field_name => $friendly_name) {
			if (!isset($info[$field_name])) {
				// field not defined
				continue;
			} else {
				$value = $info[$field_name]['value'];
				$count = 0;
				$multi = '';
				if (is_array($value)) {
					$count = count($value);
					$value = implode('<br />', $value);
				}
				if ($count > 1) $multi = ' ('.$count.' values)';

				if (!is_null($o)) {
					$o->openField($friendly_name.$multi);
				} else {
					?><tr><td valign="top"><b><?php echo $friendly_name.$multi; ?></b></td><td valign="top"><?php
				}
				echo $value;
				if (!is_null($o)) {
					echo '<br />';
					// only print extra info if we are on the backend
					// TODO: Leaving this in by request, until we are sure all implementations work
					if (isset($info[$field_name]['attrid'])) {
						echo '<span style="color:#666">(attrid = '.$info[$field_name]['attrid'].')</span> ';
					}
					echo '<span style="color:#666">(type = '.$info[$field_name]['type'].')</span>';
					$o->closeField();
				} else {
					?></td></tr><?php
				}
			}
		}//end foreach

		if (is_null($o)) {
			?></table><?php
		}

	}//end paintUserDetails()


	/**
	* Paints the user info based on the setting in the bridge
	*
	* @param object	&$user	the LDAP user asset that we are going to initialise
	* @param array	$data	the LDAP entry for this user (contains all user data)
	*
	* @return void
	* @access public
	*/
	function initUser(&$user, $data)
	{

		// the attribute mappings to the LDAP schemas
		$attrs = $this->attr('attributes');
		$user->_ldapid = $this->id;

		// set general object properties
		$user->id = $this->id.':'.$this->encodeString($data['dn']);
		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			foreach ($this->_user_types as $key => $value) {
				if (($class == $key) && isset($attrs[$value])) {
					$user->name = $data[$attrs[$value]][0];
				}
			}
		}
		if (empty($user->name)) {
			$user->name = translate('ldap_unknown_user');
		}

		$user->name = $this->decodeString($user->name);

		$user->short_name = $user->name;
		$user->status = $this->status;
		$user->version = '0.1';

		// error_log('initUser: '.$user->name."($this->id, ".$this->attr('user_type').")");

		foreach ($data as $key => $value) {
			if (is_array($value)) {
				if (isset($value['count'])) unset($value['count']);
				if (count($value) > 1) {
					$user->vars[$key] = Array(
											'value'		=> $value,
											'type'		=> 'serialise',
											'attrid'	=> 0,
										);
				} else {
					$user->vars[$key] = Array(
											'value'		=> array_shift($value),
											'type'		=> 'text',
											'attrid'	=> 0,
										);
				}
			}
		}

		// set the data paths
		$user->_loadDataPaths();

		// set variable values
		$user->vars['dn']['value'] = $data['dn'];
		$user->vars['dn']['type'] = 'text';

		$user->vars['username']['value'] = $data['dn'];
		$user->vars['username']['type'] = 'text';
		if (isset($attrs['uid'])) {
			$name = $attrs['uid'];
			$name_lowercase = strtolower($name);
			// try case sensitive first and then lowercase match. It seems PHP always returns LDAP search results in lowercase
			if(isset($data[$name])) {
			    $user->vars['username']['value'] =$data[$name][0];
			}
			else if (isset($data[$name_lowercase])) {
			     $user->vars['username']['value'] =$data[$name_lowercase][0];
			}
		}
		
		$user->vars['email']['value'] = '';
		$user->vars['email']['type'] = 'email';
		if (isset($attrs['email'])) {
			$name = $attrs['email'];
			$name_lowercase = strtolower($name);
			if(isset($data[$name])) {
			    $user->vars['email']['value'] =$data[$name][0];
			}
			else if (isset($data[$name_lowercase])) {
			     $user->vars['email']['value'] =$data[$name_lowercase][0];
			}
		}

		$user->vars['first_name']['value'] = '';
		$user->vars['first_name']['type'] = 'text';
		if (isset($attrs['fn'])) {
			$name = $attrs['fn'];
			$name_lowercase = strtolower($name);
			if(isset($data[$name])) {
			    $user->vars['first_name']['value'] =$data[$name][0];
			}
			else if (isset($data[$name_lowercase])) {
			     $user->vars['first_name']['value'] =$data[$name_lowercase][0];
			}
		}

		$user->vars['last_name']['value'] = '';
		$user->vars['last_name']['type'] = 'text';
		if (isset($attrs['sn'])) {
			$name = $attrs['sn'];
			$name_lowercase = strtolower($name);
			if(isset($data[$name])) {
			    $user->vars['last_name']['value'] =$data[$name][0];
			}
			else if (isset($data[$name_lowercase])) {
			     $user->vars['last_name']['value'] =$data[$name_lowercase][0];
			}
		}

	}//end initUser()


	/**
	* Encode a string
	* Purpose		- For Bug Fix (#3072) LDAP and Matrix cannot handle ampersand(&) in the name
	*						- To fix we encode the ampersand to a hex variable, then change it back for LDAP
	*
	* @param string $string The string to encode
	*
	* @return string
	* @access public
	*/
	function encodeString($string)
	{

			if (is_array($string)) {
					foreach ($string as $key => $value) {
							$string[$key] = $this->encodeString($value);
					}
			} else {
					$string = str_replace('&', '%26', $string);
			}

			return $string;

	}//end encodeString()


	/**
	* Decode a string
	* Purpose		- For Bug Fix (#3072) LDAP and Matrix cannot handle ampersand(&) in the name
	*						- To fix we encode the ampersand to a hex variable, then change it back for LDAP
	*
	* @param string $string The string to encode
	*
	* @return string
	* @access public
	*/
	function decodeString($string)
	{

			if (is_array($string)) {
					foreach ($string as $key => $value) {
							$string[$key] = $this->decodeString($value);
					}
			} else {
					$string = str_replace('%26', '&', $string);
			}

			return $string;

	}//end decodeString()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	public function getAssetMapAssetInfo($assetid)
	{
		return Array();

	}//end getAssetMapAssetInfo()


}//end class

?>
