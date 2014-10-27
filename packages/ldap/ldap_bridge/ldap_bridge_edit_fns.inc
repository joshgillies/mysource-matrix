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
* $Id: ldap_bridge_edit_fns.inc,v 1.32 2012/08/30 00:58:58 ewang Exp $
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
* @version $Revision: 1.32 $
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
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'attributes',
													   ),
									'permissions'	=> Array(
														'name'			=> 'Permissions',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'permissions',
													   ),
									'linking'		=> Array(
														'name'			=> 'Linking',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'links',
													   ),
									'settings'		=> Array(
														'name'			=> 'Settings',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'all',
													   ),
									'history'		=> Array(
														'name'			=> 'History',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'logs'			=> Array(
														'name'			=> 'Logs',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint a test connection to the LDAP bridge
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintConnectionTest(&$asset, &$o, $prefix)
	{
		$conn = @$asset->connectToLdap();
		if (is_null($conn) || is_null($conn->ptr)) {
			// the conncection failed
			echo '<span style="color: red"><b>'.translate('Unable to connect').'</b></span>';

		} else {
			echo '<span style="color: green"><b>'.translate('Connected').'</b></span>';

		}

		return FALSE;

	}//end paintConnectionTest()


	/**
	* Processes the interface for editing the test connection
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processConnectionTest(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processConnectionTest()


	/**
	* Paints an interface that allows the user customisation of LDAP connection options.
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintConnectionOptions(&$asset, &$o, $prefix)
	{
		$configured_options = $asset->attr('ldap_options');

		// display defined connection options
		$connection_opt_names = Array(
			'LDAP_OPT_PROTOCOL_VERSION' => translate('Use Protocol Version 3'),
			'LDAP_OPT_REFERRALS'        => translate('Disallow Referrals'),
		);
		foreach ($asset->getConnectionOptions() as $option => $checked_value) {
			$checked = FALSE;
			if (isset($configured_options[$option])) {
				$checked = TRUE;
			}

			if ($asset->writeAccess('attributes')) {
				$element_name = $prefix.'_ldap_options['.$option.']';

				check_box($element_name, $checked_value, $checked);
				?>
				<label for="<?php echo $element_name; ?>"><?php echo $connection_opt_names[$option]; ?></label><br />
				<?php
			} else {
				$display_image = 'cross';
				if ($checked) $display_image = 'tick';

				$mask = '<img src="'.sq_web_path('lib').'/web/images/'.$display_image.'.gif" />';
				echo $mask.' '.$connection_opt_names[$option].'<br />';
			}
		}//end for
		return $asset->writeAccess('attributes');

	}//end paintConnectionOptions()


	/**
	* Process interface allowing user customisation of LDAP connection options.
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processConnectionOptions(&$asset, &$o, $prefix)
	{
		$configured_options = Array();
		$element_name = $prefix.'_ldap_options';

		// for security, use defined connection options only
		foreach ($asset->getConnectionOptions() as $option => $value) {
			if (isset($_REQUEST[$element_name][$option])) {
				$configured_options[$option] = $value;
			}
		}//end for

		// only non-default LDAP options are stored, otherwise omitted
		$asset->setAttrValue('ldap_options', $configured_options);

		return $asset->writeAccess('attributes');

	}//end processConnectionOptions()


	/**
	* Paint the interface for editing schema attributes
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAttributes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('attributes');
		$required = Array(
						'user'	=> Array(
									'uid'	=> translate('User Id'),
									'cn'	=> translate('Common Name'),
									'fn'	=> translate('First Name'),
									'sn'	=> translate('Last Name'),
									'email'	=> translate('Email Address'),
								   ),
						'group'	=> Array(
									'mo'	=> translate('Group Membership'),
									'gm'	=> translate('Group Members'),
									'ou'	=> translate('Group Name'),

								   ),
					);

		$section_names = Array(
			'user'  => translate('User Attributes'),
			'group' => translate('Group Attributes'),
		)

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		}

		foreach ($required as $section => $section_data) {
			$o->openSection($section_names[$section]);
			foreach ($section_data as $code => $name) {
				$o->openField($name);
					$default = (isset($current[$code])) ? $current[$code] : '';
					if ($write_access) {
						text_box($prefix.'_attributes['.$code.']', $default, 20);
					} else {
						echo $default;
					}
				$o->closeField();
			}
			$o->closeSection();
		}

		return $write_access;

	}//end paintAttributes()


	/**
	* Processes the interface for editing schema attributes
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
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

		return FALSE;

	}//end processAttributes()


	/**
	* Paint the interface for editing schema attributes
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
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
		$user = $asset->getAsset($asset->id.':'.$asset->attr('bind_dn'), '', FALSE, FALSE, TRUE);
		if (is_null($user)) {
			trigger_localised_error('LDAP0001', translate('Failed getting user attributes from LDAP directory'), E_USER_WARNING);
			return;
		}

		$user_type = $asset->attr('user_type');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($user_type);
		$ldap_asset = new $user_type();
		$ldap_asset_attrs = array_keys($ldap_asset->vars);

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('LDAP Attribute Name'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Common Name'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Display?'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Use for Sorting ?'); ?></td>

			</tr>
		<?php

		foreach ($user->vars as $field_name => $field_data) {
			if (isset($current['show'][$field_name])) {
				$show = TRUE;
				$display_name = $current['show'][$field_name];
			} else {
				$show = FALSE;
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
						echo ($show) ? translate('Yes') : '';

					}
					?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
					if ($write_access) {
						if (!in_array($field_name, $ldap_asset_attrs)){
							radio_button($prefix.'_sort_by', $field_name, ($field_name == $sort_by));
						}
					} else {
						echo ($sort_by == $field_name) ? translate('Yes') : '';

					}
					?>
				</td>
			</tr>
			<?php
		}//end foreach $user fields

		?></table><?php

		return $write_access;

	}//end paintUserSetup()


	/**
	* Processes the interface for editing schema attributes
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUserSetup(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_user_attribute_fields'])) {
			return FALSE;
		}

		$user_attributes = Array('show' => Array(), 'hide' => Array());
		foreach ($_POST[$prefix.'_user_attribute_fields'] as $field_name => $field_data) {
			$show_hide = (isset($field_data['show']) && $field_data['show']) ? 'show' : 'hide';
			$user_attributes[$show_hide][$field_name] = $field_data['name'];
		}
		$user_attributes['sort_by'] = (isset($_POST[$prefix.'_sort_by'])) ? $_POST[$prefix.'_sort_by'] : '';

		return $asset->setAttrValue('user_attributes', $user_attributes);

	}//end processUserSetup()


	/**
	* Paints the message for LDAP not being available
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintLDAPNotAvailable(&$asset, &$o, $prefix)
	{
		?>
		<p class="sq-backend-warning"><strong><?php echo translate('The LDAP extension has not been enabled on this PHP installation.'); ?></strong></p>

		<p><?php echo translate('LDAP support needs to be compiled into/enabled in PHP, before the LDAP Bridge can work. For further information, please contact your system administrator.'); ?></p>

		<?php
		return FALSE;

	}//end paintLDAPNotAvailable()


	/**
	* Processes the (non-existant) interface for LDAP not being available
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLDAPNotAvailable(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processLDAPNotAvailable()


	/**
	* Show-if conditional: show if LDAP is available
	*
	* @return boolean
	* @access public
	*/
	function ldapAvailable()
	{
		return extension_loaded('ldap');

	}//end ldapAvailable()


	/**
	* Show-if conditional: show if LDAP is NOT available
	*
	* @return boolean
	* @access public
	*/
	function ldapNotAvailable()
	{
		return !$this->ldapAvailable();

	}//end ldapNotAvailable()


}//end class

?>
