<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_user_group.inc,v 1.5 2007/01/11 05:37:25 amiller Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Ipb_User_Group
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_User_Group extends User_Group
{

	/**
	* The ID of the LDAP bridge we are from
	*
	* @var int
	*/
	var $_ipbid = 0;

	/**
	* The the first character of the shadow id which identifies the type of
	* this asset
	*
	* @var int
	*/
	var $_prefix = '';

	/**
	* The the last character of the shadow id which helps identify the type of
	* this asset
	*
	* @var int
	*/
	var $_suffix = '';


	/**
	* Constructor
	*
	* @param int	$ipbid	the ID of the LDAP bridge that links to the LDAP directory for this group
	* @param array	$data	the LDAP entry for this group (contains all group data)
	*
	*/
	function Ipb_User_Group($ipbid=0, $data=Array())
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
		$tmp = explode('_', $data['id']);
		$this->_prefix = $tmp[0];
		$this->_suffix = $tmp[1];
		$this->id = $ipb_bridge->id.':'.$data['id'];
		if ($this->_suffix != '0') {
			$this->name = $data['g_title'];
		} else {
			$this->name = $data['name'];
		}
		if (empty($this->name)) {
			$this->name = translate('ipb_unknown_user');
		}

		$this->short_name = $this->name;
		$this->status = $ipb_bridge->status;
		$this->version = '0.1';

		// set the data paths
		$this->_loadDataPaths();

		$this->vars['name']['type'] = 'text';
		$this->vars['name']['value'] = (empty($data['name'])) ? 'none' : $data['name'];

		if ($this->_suffix != '0') {

			foreach (array_keys($data) as $key) {
				$this->vars[$key]['type'] = 'text';

				// We have to prettify some settings
				switch ($key) {

					case 'g_photo_max_vars':
						if (!empty($data[$key])) {
							$tmp = explode(':', $data[$key]);
							$value = $tmp[0].' KB';
							if (!empty($tmp[1])) {
								$value .= ', Max Width (px): '.$tmp[1];
							}//end if
							if (!empty($tmp[2])) {
								$value .= ', Max Height (px): '.$tmp[2];
							}//end if
							$this->vars[$key]['value'] = $value;
						} else {
							$this->vars[$key]['value'] = '0';
						}//end if
					break;

					case 'g_email_friend' :
						if ($data[$key]) {
							$value = 'Yes';
							$tmp = explode(':', $data['g_email_limit']);
							if ($tmp[0] != '0') {
								$value .= '. Only allow '.$tmp[1].' emails in a 24hr period';
							}
							if ($tmp[1] != '0') {
								$value .= ' and only allow 1 email every '.$tmp[1].' minutes';
							}
							$this->vars[$key]['value'] = $value;
						} else {
							$this->vars[$key]['value'] = 'No';
						}

					break;

					case 'g_promotion' :
						$tmp = explode('&', $data[$key]);
						$value = 'none';
						if ($tmp[0] != '-1') {
							$g_id = $tmp[0];
							$ipb =& $ipb_bridge->getIpbConn();
							$list_of_groups = $ipb->getMemberGroupInfo('groups');
							$value = 'Promote members of this group to : '.$list_of_groups['g_'.$g_id]['g_title'];
						}
						if ($tmp[1] != '-1') {
							$value .= ' when they reach '.$tmp[1].' posts';
						}
						$this->vars['g_promotion']['value'] = $value;

					break;

					default:
						$this->vars[$key]['value'] = $data[$key];
					break;

				}//end switch

			}//end foreach

		}//end if

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
