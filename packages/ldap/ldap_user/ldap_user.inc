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
* $Id: ldap_user.inc,v 1.1 2004/02/06 03:12:38 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user.inc';

/**
* Ldap_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User extends Backend_User
{


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the LDAP bridge that links to the LDAP directory for this user
	* @param array	$data	the LDAP entry for this user (contains all user data)
	*
	*/
	function Ldap_User($ldapid=0, $data=Array())
	{
		if (!$ldapid || empty($data)) return;
		$ldap = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid, 'ldap_bridge');
		if (is_null($ldap)) return;

		// the attribute mappings to the LDAP schemas
		$attrs = $ldap->attr('attributes');

		// set general object properties
		$this->id = $ldap->id.':'.$data['dn'];
		$this->name = $data[$attrs['cn']][0];
		$this->short_name = $this->name;
		$this->status = $ldap->status;
		$this->version = '0.1';

		// set the data paths
		$this->data_path = $ldap->data_path.'/'.$data['dn'];
		$this->data_path_suffix = $ldap->data_path_suffix.'/'.$data['dn'];
		$this->data_path_public = $ldap->data_path_public.'/'.$data['dn'];

		// set variable values
		$this->vars['dn']['value'] = $data['dn'];
		$this->vars['dn']['type'] = 'text';
		
		if (isset($data[$attrs['uid']])) $this->vars['username']['value'] = $data[$attrs['uid']][0];
		else $this->vars['username']['value'] = $data['dn'];
		$this->vars['username']['type'] = 'text';
		
		if (isset($data[$attrs['email']])) $this->vars['email']['value'] = $data[$attrs['email']][0];
		else $this->vars['email']['value'] = '';
		$this->vars['email']['type'] = 'text';

		if (isset($data[$attrs['fn']])) $this->vars['first_name']['value'] = $data[$attrs['fn']][0];
		else $this->vars['first_name']['value'] = '';
		$this->vars['first_name']['type'] = 'text';

		if (isset($data[$attrs['sn']])) $this->vars['last_name']['value'] = $data[$attrs['sn']][0];
		else $this->vars['last_name']['value'] = '';
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
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access public
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
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
		return true;

	}//end canLogin()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$info = $this->getUserData();
		if (empty($info)) return;

		?>
		<table border="0" cellspacing="2" cellpadding="2">
		<?php
			for ($i = 0; $i < $info['count']; $i++) {
				$field_name = $info[$i];
				if ($info[$info[$i]]['count'] > 1) $field_name .= ' ('.$info[$info[$i]]['count'].' values)';
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
	* Get an array of attribute data for this user
	*
	* @return array
	* @access public
	*/
	function getUserData()
	{
		$id_parts = explode(':', $this->id);
		$ldapid = $id_parts[0];
		$shadowid = $id_parts[1];

		$ldap_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid);
		if (is_null($ldap_asset)) return Array();

		require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';
		$ldap = $ldap_asset->connectToLdap();
		$result = $ldap->search($shadowid,'(objectClass=*)');
		if (!$result) return Array();
		$info = $ldap->getEntries($result);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ldap);

		if (!isset($info[0])) return Array();
		else return $info[0];

	}//end getUserData()


}//end class

?>