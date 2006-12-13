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
* $Id: ldap_user.inc,v 1.15.4.2 2006/12/13 01:05:35 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user.inc';

/**
* Ldap_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15.4.2 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User extends Backend_User
{

	/**
	* The ID of the LDAP bridge we are from
	*
	* @var int
	*/
	var $_ldapid = 0;


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the LDAP bridge that links to the LDAP directory for this user
	* @param array	$data	the LDAP entry for this user (contains all user data)
	*
	*/
	function Ldap_User($ldapid=0, $data=Array())
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		if (!$ldapid || empty($data)) {
			return;
		}

		$ldap =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid, 'ldap_bridge');
		if (is_null($ldap)) return;

		$this->_ldapid = $ldapid;

		// the attribute mappings to the LDAP schemas
		$attrs = $ldap->attr('attributes');

		// set general object properties
		$this->id = $ldap->id.':'.$data['dn'];
		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			foreach ($ldap->_user_types as $key => $value) {
				if ($class == $key && isset($attrs[$value])) {
					$this->name = $data[$attrs[$value]][0];
				}
			}
		}
		if (empty($this->name)) {
			$this->name = translate('ldap_unknown_user');
		}

		$this->short_name = $this->name;
		$this->status = $ldap->status;
		$this->version = '0.1';

		foreach ($data as $key => $value) {
			if (is_array($value)) {
				if (isset($value['count'])) unset($value['count']);
				if (count($value) > 1) {
					$this->vars[$key] = Array(
											'value'	=> $value,
											'type'		=> 'serialise',
											'attrid'	=> 0,
										);
				} else {
					$this->vars[$key] = Array(
											'value'		=> array_shift($value),
											'type'		=> 'text',
											'attrid'	=> 0,
										);
				}
			}
		}

		// set the data paths
		$this->_loadDataPaths();

		// set variable values
		$this->vars['dn']['value'] = $data['dn'];
		$this->vars['dn']['type'] = 'text';

		if (isset($attrs['uid']) && isset($data[$attrs['uid']])) {
			$this->vars['username']['value'] = $data[$attrs['uid']][0];
		} else {
			$this->vars['username']['value'] = $data['dn'];
		}

		$this->vars['username']['type'] = 'text';

		if (isset($attrs['email']) && isset($data[$attrs['email']])) {
			$this->vars['email']['value'] = $data[$attrs['email']][0];
		} else {
			$this->vars['email']['value'] = '';
		}

		$this->vars['email']['type'] = 'email';

		if (isset($attrs['fn']) && isset($data[$attrs['fn']])) {
			$this->vars['first_name']['value'] = $data[$attrs['fn']][0];
		} else {
			$this->vars['first_name']['value'] = '';
		}

		$this->vars['first_name']['type'] = 'text';

		if (isset($attrs['sn']) && isset($data[$attrs['sn']])) {
			$this->vars['last_name']['value'] = $data[$attrs['sn']][0];
		} else {
			$this->vars['last_name']['value'] = '';
		}

		$this->vars['last_name']['type'] = 'text';

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns name of the user
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return 'LDAP User';

	}//end _getName()


	/**
	* Check if this user can log in
	*
	* Note that this is not for backend logins only, it applys to all logins
	*
	* @return boolean
	* @access public
	*/
	function canLogin()
	{
		return TRUE;

	}//end canLogin()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$info = $this->vars;
		if (empty($info)) return;

		if (!$this->_ldapid) return Array();
		$ldap_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return Array();

		$show = $ldap_bridge->attr('user_attributes');
		$show = $show['show'];

		?>
		<table border="0" cellspacing="2" cellpadding="2">
		<?php
			for ($i = 0; $i < $info['count']; $i++) {
				if (!isset($show[$info[$i]])) continue;
				$field_name = $show[$info[$i]];

				if ($info[$info[$i]]['count'] > 1) {
					$field_name .= ' ('.$info[$info[$i]]['count'].' values)';
				}

				unset($info[$info[$i]]['count']);
				?>
				<tr>
					<td valign="top"><b><?php echo $field_name; ?></b></td>
					<td valign="top">
						<?php
						foreach ($info[$info[$i]] as $id => $value) {
							echo $value.'<br/>';
						}
						?>
					</td>
				</tr>
				<?php
			}//end for
		?>
		</table>
		<?php

	}//end printBody()


	/**
	* Returns NULL as LDAP users do not have a workspace
	*
	* @return object
	* @access public
	*/
	function &getWorkspace()
	{
		$null = NULL;
		return $null;

	}//end getWorkspace()


}//end class

?>
