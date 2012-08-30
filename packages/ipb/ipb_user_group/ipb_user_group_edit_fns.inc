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
* $Id: ipb_user_group_edit_fns.inc,v 1.5 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user_group/user_group_edit_fns.inc';

/**
* Ipb_User_Group_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_User_Group_Edit_Fns extends User_Group_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_User_Group_Edit_Fns()
	{
		$this->User_Group_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint the interface for editing group details
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDetails(&$asset, &$o, $prefix)
	{

		if ($asset->_prefix == '0') {
			$o->openField(translate('details'));
				echo translate('list_of_ipb_user_groups');
			$o->closeField();

		} else {
			$info = $asset->vars;
			if (empty($info)) return;

			foreach ($info as $field_name => $field_data) {
				$count = count($field_data);
				if ($count > 1) $field_name .= ' ('.$count.' values)';
				$o->openField($field_name);
					echo implode('<br />', $field_data);
				$o->closeField();
			}
		}
		// no committing allowed
		return FALSE;

	}//end paintDetails()


	/**
	* Processes the interface for editing group details
	*
	* Editing the details of an LDAP group is not yet implemented
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processDetails(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processDetails()


	/**
	* Paint the interface for listing user groups
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintListOfUserGroups(&$asset, &$o, $prefix)
	{
		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$groups = $ipb->getListOfGroups();

		if (empty($groups)) {
			$o->openField(translate('list_of_ipb_user_groups'));
				echo translate('no_user_groups_in_ipb');
			$o->closeField();
		} else {
			$o->openField(translate('list_of_ipb_user_groups'));
				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 50%"><?php echo translate('name'); ?></th>
						<th style="width: 50%"><?php echo translate('asset'); ?></th>
					</tr>
				<?php
				foreach ($groups as $group) {
					$data = $ipb->getGroupInfo($ipb_bridge->id.':'.'g_'.$group['id']);
				?>
					<tr>
						<td style="font-weight: bold;"><?php echo $data['g_title']; ?></td>
						<td><?php echo  get_asset_tag_line($asset->_ipbid.':'.'g_'.$data['g_id']); ?></td>
					</tr>
				<?php
				}
			$o->closeField();
		}
		return FALSE;

	}//end paintListOfUserGroups()


	/**
	* Processes the interface for listing user groups
	*
	* Editing the details of an LDAP group is not yet implemented
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processListOfUserGroups(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processListOfUserGroups()


	/**
	* Function that returns TRUE is this ipb_user_group is not
	* a top category.
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function notCategory(&$asset, $prefix)
	{
		$id_parts = explode(':', $asset->id);
		if ($id_parts[1] != 'g_0') return TRUE;

		return FALSE;

	}//end notCategory()


	/**
	* Function that returns TRUE is this ipb_user_group is a top category.
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function isCategory(&$asset, $prefix)
	{
		return !$this->notCategory($asset, $prefix);

	}//end isCategory()


}//end class

?>
