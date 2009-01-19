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
* $Id: ldap_user_group.inc,v 1.15 2009/01/19 00:16:18 mbrydon Exp $
*
*/

check_ssv_licence();
require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group.inc';

/**
* Ldap_User_Group
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
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


	/**
	* Prints a list of users for the passed group
	*
	* @param string	$groupid	the ID of the group that we are printing the user list for
	*
	* @return void
	* @access public
	*/
	function printUserList($groupid)
	{
		if (!$this->_ldapid) return;

		$ldap_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_ldapid, 'ldap_bridge');
		if (is_null($ldap_bridge)) return;

		$group =& $GLOBALS['SQ_SYSTEM']->am->getAsset($groupid);

		$show = $ldap_bridge->attr('user_attributes');
		if (!isset($show['show'])) {
			$show = Array();
		} else {
			$show = $show['show'];
		}

		$special_attrs = $ldap_bridge->attr('attributes');

		?>
		<table class="sq_user_group">
			<tr>
				<td class="sq_user_group_heading" colspan="<?php echo count($show); ?>">
					<?php
					$url = $group->getURL();
					if (!empty($url)) echo '<a href="'.$url.'">';
					echo $group->name;
					if (!empty($url)) echo '</a>';
					?>
				</td>
			</tr>
			<?php
			$users = $GLOBALS['SQ_SYSTEM']->am->getLinks($groupid, SQ_LINK_TYPE_1, 'user');
			if (!empty($users)) {
				?>
				<tr>
					<?php
					foreach ($show as $field_name => $display_name) {
						?><td class="sq_user_group_display_name"><?php echo $display_name; ?></td><?php
					}
					?>
				</tr>
				<?php
				foreach ($users as $link_data) {
					$user =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link_data['minorid'], $link_data['minor_type_code']);

					$info = $user->vars;
					if (empty($info)) continue;

					echo '<tr>';
					foreach ($show as $field_name => $display_name) {
						if (isset($info[$field_name])) {
							$field_value = '';
							$field_data = $info[$field_name];
							$class = 'sq_user_group_user_data';

							if ($special_attrs['email'] == $field_name) {
								$class = 'sq_user_group_user_email';
								foreach ($field_data as $email) {
									if (!empty($field_value)) $field_value .= '<br />';
									$field_value .= '<a href="mailto:'.$email.'">'.$email.'</a>';
								}
							} else {
								if ($special_attrs['cn'] == $field_name) {
									$class = 'sq_user_group_user_name';
								}
								$field_value = implode('<br />', $info[$field_name]);
							}
						} else {
							$field_value = '&nbsp;';
						}
						?><td class="<?php echo $class; ?>"><?php echo $field_value; ?></td><?php
					}
					echo '</tr>';

				}//end foreach

			}//end if there are users

		echo '</table>';

	}//end printUserList()


}//end class

?>
