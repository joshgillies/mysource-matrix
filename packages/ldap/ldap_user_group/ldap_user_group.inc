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
* $Id: ldap_user_group.inc,v 1.1 2004/02/06 03:12:38 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Ldap_User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Group extends User_Group
{


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the LDAP bridge that links to the LDAP directory for this group
	* @param array	$data	the LDAP entry for this group (contains all group data)
	*
	*/
	function Ldap_User_Group($ldapid=0, $data=Array())
	{
		if (!$ldapid || empty($data)) return;
		$ldap = &$GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid, 'ldap_bridge');
		if (is_null($ldap)) return;

		// the attribute mappings to the LDAP schemas
		$attrs = $ldap->attr('attributes');

		// set general object properties
		$this->id = $ldap->id.':'.$data['dn'];
		$this->name = $data[$attrs['ou']][0];
		$this->short_name = $this->name;
		$this->status = $ldap->status;
		$this->version = '0.1';

		// set the data paths
		$this->data_path = $ldap->data_path.'/'.$data['dn'];
		$this->data_path_suffix = $ldap->data_path_suffix.'/'.$data['dn'];
		$this->data_path_public = $ldap->data_path_public.'/'.$data['dn'];

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Get an array of attribute data for this group
	*
	* @return array
	* @access public
	*/
	function getGroupData()
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

	}//end getGroupData()


}//end class

?>