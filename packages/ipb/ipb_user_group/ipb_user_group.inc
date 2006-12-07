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
* $Id: ipb_user_group.inc,v 1.3 2006/12/07 03:11:02 bcaldwell Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Ipb_User_Group
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.3 $
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
			// Global Settings
			$this->vars['g_title']['type'] = 'text';
			$this->vars['g_title']['value'] = $data['g_title'];

			$this->vars['g_access_offline']['type'] = 'text';
			$this->vars['g_access_offline']['value'] = ($data['g_access_offline']) ? 'Yes' : 'No';

			$this->vars['g_icon']['type'] = 'text';
			$this->vars['g_icon']['value'] = (empty($data['g_icon'])) ? 'none' : $data['g_icon'];

			$this->vars['prefix']['type'] = 'text';
			$this->vars['prefix']['value'] = (empty($data['prefix'])) ? 'none' : $data['prefix'];

			$this->vars['suffix']['type'] = 'text';
			$this->vars['suffix']['value'] = (empty($data['suffix'])) ? 'none' : $data['suffix'];

			$this->vars['g_hide_from_list']['type'] = 'text';
			$this->vars['g_hide_from_list']['value'] = ($data['g_hide_from_list']) ? 'Yes' : 'No';

			// Upload Permissions
			$this->vars['g_attach_max']['type'] = 'text';
			$this->vars['g_attach_max']['value'] = (empty($data['g_attach_max'])) ? '0' : $data['g_attach_max'];

			$this->vars['g_attach_per_post']['type'] = 'text';
			$this->vars['g_attach_per_post']['value'] = (empty($data['g_attach_per_post'])) ? '0' : $data['g_attach_per_post'];

			$this->vars['g_photo_max_vars']['type'] = 'text';
			if (!empty($data['g_photo_max_vars'])) {
				$tmp = explode(':', $data['g_photo_max_vars']);
				$value = $tmp[0].' KB';
				if (!empty($tmp[1])) {
					$value .= ' Max Width(px):'.$tmp[1];
				}
				if (!empty($tmp[2])) {
					$value .= ' Max Height(px):'.$tmp[2];
				}
				$this->vars['g_photo_max_vars']['value'] = $value;
			} else {
				$this->vars['g_photo_max_vars']['value'] = '0';
			}

			$this->vars['g_avatar_upload']['type'] = 'text';
			$this->vars['g_avatar_upload']['value'] = ($data['g_avatar_upload']) ? 'Yes' : 'No';

			$this->vars['g_can_msg_attach']['type'] = 'text';
			$this->vars['g_can_msg_attach']['value'] = ($data['g_can_msg_attach']) ? 'Yes' : 'No';

			// Global Permissions
			$this->vars['g_view_board']['type'] = 'text';
			$this->vars['g_view_board']['value'] = ($data['g_view_board']) ? 'Yes' : 'No';

			$this->vars['g_access_offline']['type'] = 'text';
			$this->vars['g_access_offline']['value'] = ($data['g_access_offline']) ? 'Yes' : 'No';

			$this->vars['g_mem_info']['type'] = 'text';
			$this->vars['g_mem_info']['value'] = ($data['g_mem_info']) ? 'Yes' : 'No';

			$this->vars['g_other_topics']['type'] = 'text';
			$this->vars['g_other_topics']['value'] = ($data['g_other_topics']) ? 'Yes' : 'No';

			$this->vars['g_use_search']['type'] = 'text';
			$this->vars['g_use_search']['value'] = ($data['g_use_search']) ? 'Yes' : 'No';

			$this->vars['g_search_flood']['type'] = 'text';
			$this->vars['g_search_flood']['value'] = (empty($data['g_search_flood'])) ? '0' : $data['g_search_flood'];

			$this->vars['g_email_friend']['type'] = 'text';
			if ($data['g_email_friend']) {
				$value = 'Yes';
				$tmp = explode(':', $data['g_email_limit']);
				if ($tmp[0] != '0') {
					$value .= '. Only allow '.$tmp[1].' emails in a 24hr period.';
				}
				if ($tmp[1] != '0') {
					$value .= '.. and only allow 1 email every '.$tmp[1].' minutes';
				}
				$this->vars['g_email_friend']['value'] = $value;
			} else {
				$this->vars['g_email_friend']['value'] = 'No';
			}

			$this->vars['g_edit_profile']['type'] = 'text';
			$this->vars['g_edit_profile']['value'] = ($data['g_edit_profile']) ? 'Yes' : 'No';

			$this->vars['g_use_pm']['type'] = 'text';
			$this->vars['g_use_pm']['value'] = ($data['g_use_pm']) ? 'Yes' : 'No';

			$this->vars['g_max_mass_pm']['type'] = 'text';
			$this->vars['g_max_mass_pm']['value'] = (empty($data['g_max_mass_pm'])) ? '0' : $data['g_max_mass_pm'];

			$this->vars['g_max_messages']['type'] = 'text';
			$this->vars['g_max_messages']['value'] = (empty($data['g_max_messages'])) ? '0' : $data['g_max_messages'];

			// Posting Permissions
			$this->vars['g_post_new_topics']['type'] = 'text';
			$this->vars['g_post_new_topics']['value'] = ($data['g_post_new_topics']) ? 'Yes' : 'No';

			$this->vars['g_reply_own_topics']['type'] = 'text';
			$this->vars['g_reply_own_topics']['value'] = ($data['g_reply_own_topics']) ? 'Yes' : 'No';

			$this->vars['g_reply_other_topics']['type'] = 'text';
			$this->vars['g_reply_other_topics']['value'] = ($data['g_reply_other_topics']) ? 'Yes' : 'No';

			$this->vars['g_edit_posts']['type'] = 'text';
			$this->vars['g_edit_posts']['value'] = ($data['g_edit_posts']) ? 'Yes' : 'No';

			$this->vars['g_edit_cutoff']['type'] = 'text';
			$this->vars['g_edit_cutoff']['value'] = (empty($data['g_edit_cutoff'])) ? '0' : $data['g_edit_cutoff'];

			$this->vars['g_append_edit']['type'] = 'text';
			$this->vars['g_append_edit']['value'] = ($data['g_append_edit']) ? 'Yes' : 'No';

			$this->vars['g_delete_own_posts']['type'] = 'text';
			$this->vars['g_delete_own_posts']['value'] = ($data['g_delete_own_posts']) ? 'Yes' : 'No';

			$this->vars['g_open_close_posts']['type'] = 'text';
			$this->vars['g_open_close_posts']['value'] = ($data['g_open_close_posts']) ? 'Yes' : 'No';

			$this->vars['g_edit_topic']['type'] = 'text';
			$this->vars['g_edit_topic']['value'] = ($data['g_edit_topic']) ? 'Yes' : 'No';

			$this->vars['g_delete_own_topics']['type'] = 'text';
			$this->vars['g_delete_own_topics']['value'] = ($data['g_delete_own_topics']) ? 'Yes' : 'No';

			$this->vars['g_post_polls']['type'] = 'text';
			$this->vars['g_post_polls']['value'] = ($data['g_post_polls']) ? 'Yes' : 'No';

			$this->vars['g_vote_polls']['type'] = 'text';
			$this->vars['g_vote_polls']['value'] = ($data['g_vote_polls']) ? 'Yes' : 'No';

			$this->vars['g_avoid_flood']['type'] = 'text';
			$this->vars['g_avoid_flood']['value'] = ($data['g_avoid_flood']) ? 'Yes' : 'No';

			$this->vars['g_avoid_q']['type'] = 'text';
			$this->vars['g_avoid_q']['value'] = ($data['g_avoid_q']) ? 'Yes' : 'No';

			$this->vars['g_calendar_post']['type'] = 'text';
			$this->vars['g_calendar_post']['value'] = ($data['g_calendar_post']) ? 'Yes' : 'No';

			$this->vars['g_dohtml']['type'] = 'text';
			$this->vars['g_dohtml']['value'] = ($data['g_dohtml']) ? 'Yes' : 'No';

			$this->vars['g_bypass_badwords']['type'] = 'text';
			$this->vars['g_bypass_badwords']['value'] = ($data['g_bypass_badwords']) ? 'Yes' : 'No';

			// Moderation Permissions
			$this->vars['g_is_supmod']['type'] = 'text';
			$this->vars['g_is_supmod']['value'] = ($data['g_is_supmod']) ? 'Yes' : 'No';

			$this->vars['g_access_cp']['type'] = 'text';
			$this->vars['g_access_cp']['value'] = ($data['g_access_cp']) ? 'Yes' : 'No';

			$this->vars['g_post_closed']['type'] = 'text';
			$this->vars['g_post_closed']['value'] = ($data['g_post_closed']) ? 'Yes' : 'No';

			// Group Permissions
			$this->vars['g_promotion']['type'] = 'text';
			$tmp = explode('&', $data['g_promotion']);
			$value = 'none';
			if ($tmp[0] != '-1') {
				$g_id = $tmp[0];
				$ipb =& $ipb_bridge->getIpbConn();
				$list_of_groups = $ipb->getMemberGroupInfo('groups');
				$value = 'Promote members of thsi group to : '.$list_of_groups['g_'.$g_id]['g_title'];
			}
			if ($tmp[1] != '-1') {
				$value .= ' when they reach '.$tmp[1].' posts';
			}
			$this->vars['g_promotion']['value'] = $value;

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
