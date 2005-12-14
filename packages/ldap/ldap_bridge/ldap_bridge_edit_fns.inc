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
* $Id: ldap_bridge_edit_fns.inc,v 1.19 2005/12/14 03:11:35 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* LDAP_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage ldap
*/
class LDAP_Bridge_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function LDAP_Bridge_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> true,
														'lock_type'		=> 'attributes',
													   ),
									'permissions'	=> Array(
														'name'			=> 'Permissions',
														'force_unlock'	=> true,
														'lock_type'		=> 'permissions',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Directory Tree',
														'force_unlock'	=> true,
														'lock_type'		=> 'none',
													   ),
									'linking'		=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> true,
														'lock_type'		=> 'links',
													   ),
									'settings'		=> Array(
														'name'			=> 'Settings',
														'force_unlock'	=> true,
														'lock_type'		=> 'all',
													   ),
									'history'		=> Array(
														'name'			=> 'History',
														'force_unlock'	=> true,
														'lock_type'		=> 'none',
													   ),
									'logs'			=> Array(
														'name'			=> 'Logs',
														'force_unlock'	=> true,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint a test connection to the LDAP bridge
	*
	* @param object LDAP_Bridge			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintConnectionTest(&$asset, &$o, $prefix)
	{
		$conn = @$asset->connectToLdap();
		if (is_null($conn->ptr)) {
			// the conncection failed
			echo '<span style="color: red"><b>'.translate('ldap_unable_to_connect').'</b></span>';
		} else {
			echo '<span style="color: green"><b>'.translate('ldap_connected').'</b></span>';
		}


		return false;

	}//end paintConnectionTest()


	/**
	* Processes the interface for editing the test connection
	*
	* @param object LDAP_Bridge			&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processConnectionTest(&$asset, &$o, $prefix)
	{
		return false;

	}//end processConnectionTest()


	/**
	* Paint the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAttributes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('attributes');
		$required = Array(
						'uid'	=> translate('ldap_user_id'),
						'cn'	=> translate('ldap_common_name'),
						'fn'	=> translate('ldap_first_name'),
						'sn'	=> translate('ldap_last_name'),
						'email'	=> translate('ldap_email_address'),
						'mo'	=> translate('ldap_group_membership'),
						'ou'	=> translate('ldap_group_name'),
					);

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		}

		foreach ($required as $code => $name) {
			$o->openField($name);
				$default = (isset($current[$code])) ? $current[$code] : '';
				if ($write_access) {
					text_box($prefix.'_attributes['.$code.']', $default, 20);
				} else {
					echo $default;
				}
			$o->closeField();
		}

		return $write_access;

	}//end paintAttributes()


	/**
	* Processes the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAttributes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_attributes'])) {
			$new_attributes = Array();
			foreach ($_POST[$prefix.'_attributes'] as $code => $name) {
				$name = (string)$name;
				if (empty($name)) continue;
				$new_attributes[(string)$code] = $name;
			}
			return $asset->setAttrValue('attributes', $new_attributes);
		}

		return false;

	}//end processAttributes()


	/**
	* Paint the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUserSetup(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('user_attributes');
		$sort_by = (isset($current['sort_by'])) ? $current['sort_by'] : '';

		// connect through the bridge and find out what type of information
		// we get back for the users
		$user = &$asset->getAsset($asset->id.':'.$asset->attr('bind_dn'));
		if (is_null($user)) {
			trigger_localised_error('LDAP0001', E_USER_WARNING);
			return;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('ldap_attribute_name'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('ldap_common_name'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('display_question'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('ldap_use_for_sorting_question'); ?></td>
			</tr>
		<?php

		foreach ($user->vars as $field_name => $field_data) {
			if (isset($current['show'][$field_name])) {
				$show = true;
				$display_name = $current['show'][$field_name];
			} else {
				$show = false;
				$display_name = (isset($current['hide'][$field_name])) ? $current['hide'][$field_name] : $field_name;
			}

			?>
			<tr>
				<td class="sq-backend-table-cell"><b><?php echo $field_name; ?></b></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_user_attribute_fields['.$field_name.'][name]', $display_name);
					} else {
						echo $display_name;
					}
					?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						check_box($prefix.'_user_attribute_fields['.$field_name.'][show]', 1, $show);
					} else {
						echo ($show) ? translate('yes') : '';
					}
					?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						radio_button($prefix.'_sort_by', $field_name, ($field_name == $sort_by));
					} else {
						echo ($sort_by == $field_name) ? translate('yes') : '';
					}
					?>
				</td>
			</tr>
			<?php
		}//end for

		?></table><?php

		return $write_access;

	}//end paintUserSetup()


	/**
	* Processes the interface for editing schema attributes
	*
	* @param object LDAP_Bridge			&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUserSetup(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_user_attribute_fields'])) {
			return false;
		}

		$user_attributes = Array('show' => Array(), 'hide' => Array());
		foreach ($_POST[$prefix.'_user_attribute_fields'] as $field_name => $field_data) {
			$show_hide = (isset($field_data['show']) && $field_data['show']) ? 'show' : 'hide';
			$user_attributes[$show_hide][$field_name] = $field_data['name'];
		}
		$user_attributes['sort_by'] = (isset($_POST[$prefix.'_sort_by'])) ? $_POST[$prefix.'_sort_by'] : '';
		return $asset->setAttrValue('user_attributes', $user_attributes);

	}//end processUserSetup()


}//end class

?>
