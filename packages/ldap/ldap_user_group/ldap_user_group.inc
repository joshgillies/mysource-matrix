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
* $Id: ldap_user_group.inc,v 1.19 2012/08/30 00:58:58 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Ldap_User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class Ldap_User_Group extends User_Group
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
	* @param int	$ldapid	the ID of the LDAP bridge that links to the LDAP directory for this group
	* @param array	$data	the LDAP entry for this group (contains all group data)
	*
	*/
	function Ldap_User_Group($ldapid=0, $data=Array())
	{
		if (!$ldapid || empty($data)) {
			return;
		}

		$ldap = $GLOBALS['SQ_SYSTEM']->am->getAsset($ldapid, 'ldap_bridge');
		if (is_null($ldap)) return;
		$this->_ldapid = $ldapid;

		// the attribute mappings to the LDAP schemas
		$attrs = $ldap->attr('attributes');

		// set general object properties
		$this->id = $ldap->id.':'.$data['dn'];
		unset($data['objectclass']['count']);
		foreach ($data['objectclass'] as $class) {
			$class = strtolower($class);
			foreach ($ldap->_group_types as $key => $value) {
				if ($class == $key) {
					if (!isset($attrs[$value])) {
						$attrs[$value] = $value;
					}


					if (isset($data[$attrs[$value]])) {
						$this->name = $data[$attrs[$value]][0];
					}
				}
			}
		}

		$this->short_name = $this->name;
		$this->status = $ldap->status;
		$this->version = '0.1';

		foreach ($data as $key => $value) {
			if (!is_array($value)) continue;
			if (isset($value['count'])) unset($value['count']);
			$this->vars[$key] = $value;
		}

		// set the data paths
		$this->_loadDataPaths();

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



}//end class

?>
