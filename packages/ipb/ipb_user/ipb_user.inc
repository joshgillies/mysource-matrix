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
* $Id: ipb_user.inc,v 1.12 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/backend_user/backend_user.inc';

/**
* Ipb_User
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_User extends Backend_User
{

	/**
	* The ID of the IPB bridge we are from
	*
	* @var int
	*/
	var $_ipbid = 0;


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the IPB bridge that links to the IPB for this user
	* @param array	$data	the IPB entry for this user (contains all user data)
	*
	*/
	function Ipb_User($ipbid=0, $data=Array())
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		if (!$ipbid || empty($data)) {
			return;
		}

		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ipbid, 'ipb_bridge');
		if (is_null($ipb_bridge)) return;

		$this->_ipbid = $ipbid;

		// set general object properties
		$this->id = $ipb_bridge->id.':'.$data['id'];
		if (!isset($data['name'])) {
			$this->name = translate('ipb_unknown_user');
		} else {
			$this->name = $data['name'];
		}

		$this->short_name = $this->name;
		$this->status = $ipb_bridge->status;
		$this->version = '0.1';
		if (!empty($ipb_bridge->created)) {
			$this->created = date('Y-m-d H:m:i', $ipb_bridge->created);
		}
		if (!empty($ipb_bridge->updated)) {
			$this->updated = date('Y-m-d H:m:i', $ipb_bridge->updated);
		}
		if (!empty($ipb_bridge->published)) {
			$this->published = date('Y-m-d H:m:i', $ipb_bridge->published);
		}
		if (!empty($ipb_bridge->status_changed)) {
			$this->status_changed = date('Y-m-d H:m:i', $ipb_bridge->status_changed);
		}

		// set the data paths
		$this->_loadDataPaths();

		// set variable values
		$this->vars['mgroup']['type']	= 'text';
		$this->vars['mgroup']['value']	= (empty($data['mgroup'])) ? 'none' : $data['mgroup'];

		$this->vars['name']['type']		= 'text';
		$this->vars['name']['value']	= (empty($data['name'])) ? 'none' : $data['name'];

		$this->vars['email']['type']	= 'email';
		$this->vars['email']['value']	= (empty($data['email'])) ? 'none' : $data['email'];

		$this->vars['title']['type']	= 'text';
		$this->vars['title']['value']	= (empty($data['title'])) ? 'none' : $data['title'];

		$this->vars['member_login_key']['type']		= 'text';
		$this->vars['member_login_key']['value']	= (empty($data['member_login_key'])) ? 'none' : $data['member_login_key'];

		$day_empty = empty($data['bday_day']);
		$month_empty = empty($data['bday_month']);
		$year_empty = empty($data['bday_year']);
		$this->vars['date_of_birth']['type'] = 'text';
		if ($day_empty || $month_empty || $year_empty) {
			$this->vars['date_of_birth']['value'] = 'none';
		} else {
			$this->vars['date_of_birth']['value'] = $data['bday_day'].' / '.$data['bday_month'].' / '.$data['bday_year'];
		}

		if (isset($data['photo_type'])) {
			$this->vars['photo_type']['value']	= $data['photo_type'];
		} else {
			$this->vars['photo_type']['value']	= '';
		}
		$this->vars['photo_type']['type']       = 'text';

		if (isset($data['photo_location'])) {
			$this->vars['photo_location']['value']	= $data['photo_location'];
		} else {
			$this->vars['photo_location']['value']  = '';
		}
		$this->vars['photo_location']['type']   = 'text';

		$this->vars['aim_name']['type']			= 'text';
		$this->vars['aim_name']['value']		= (empty($data['aim_name'])) ? 'none' : $data['aim_name'];

		$this->vars['icq_number']['type']		= 'text';
		$this->vars['icq_number']['value']		= (empty($data['icq_number'])) ? 'none' : $data['icq_number'];

		$this->vars['website']['type']			= 'text';
		$this->vars['website']['value']			= (empty($data['website'])) ? 'none' : $data['website'];

		$this->vars['yahoo']['type']			= 'text';
		$this->vars['yahoo']['value']			= (empty($data['yahoo'])) ? 'none' : $data['yahoo'];

		$this->vars['interests']['type']		= 'text';
		$this->vars['interests']['value']		= (empty($data['interests'])) ? 'none' : html_entity_decode($data['interests']);

		$this->vars['msnname']['type']			= 'text';
		$this->vars['msnname']['value']			= (empty($data['msnname'])) ? 'none' : $data['msnname'];

		$this->vars['vdirs']['type']			= 'text';
		$this->vars['vdirs']['value']			= (empty($data['vdirs'])) ? 'none' : $data['vdirs'];

		$this->vars['location']['type']			= 'text';
		$this->vars['location']['value']		= (empty($data['location'])) ? 'none' : $data['location'];

		$this->vars['signature']['type']		= 'text';
		$this->vars['signature']['value']		= (empty($data['signature'])) ? 'none' : $data['signature'];

		$this->vars['avatar_location']['type']	= 'text';
		$this->vars['avatar_location']['value']	= (empty($data['avatar_location'])) ? 'none' : $data['avatar_location'];

		$this->vars['avatar_size']['type']		= 'text';
		$this->vars['avatar_size']['value']		= (empty($data['avatar_size'])) ? 'none' : $data['avatar_size'];

		$this->vars['avatar_type']['type']		= 'text';
		$this->vars['avatar_type']['value']		= (empty($data['avatar_type'])) ? 'none' : $data['avatar_type'];

		// custom fields
		$ipb =& $ipb_bridge->getIpbConn();
		$custom_fields = $ipb->getCustomFields();
		$user_custom_data = $ipb->getMemberCustomFieldData($this->id);
		foreach ($custom_fields as $field) {
			$field_name = strtolower($field['pf_title']);
			$field_name = str_replace(' ', '_', $field_name);
			$field_value = (isset($user_custom_data[0]['field_'.$field['pf_id']]) ? $user_custom_data[0]['field_'.$field['pf_id']] : '');
			$this->vars[$field_name]['value'] = $field_value;
			$this->vars[$field_name]['type'] = 'text';
		}

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
		return 'IPB User';

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
	* Returns null as IPB users do not have a workspace
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
