<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ldap_edit_fns.inc,v 1.2 2011/04/12 03:07:27 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source_edit_fns.inc';

/**
* Data_Source_Ldap_Edit_Fns
*
* Purpose
*
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Ldap_Edit_Fns extends Data_Source_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
		$conn = null;
		$conn = @$asset->connectToLdap();
		
		if (is_null($conn) || is_null($conn->ptr)) {
			// the conncection failed
			echo '<span style="color: red"><b>'.translate('ldap_unable_to_connect').'</b></span>';
		} else {
			echo '<span style="color: green"><b>'.translate('ldap_connected').'</b></span>';
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
		foreach ($asset->getConnectionOptions() as $option => $checked_value) {
			$checked = FALSE;
			if (isset($configured_options[$option])) {
				$checked = TRUE;
			}

			if ($asset->writeAccess('attributes')) {
				$element_name = $prefix.'_ldap_options['.$option.']';

				check_box($element_name, $checked_value, $checked);
				?>
				<label for="<?php echo $element_name; ?>"><?php echo translate('option_'.strtolower($option)); ?></label><br />
				<?php
			} else {
				$display_image = 'cross';
				if ($checked) $display_image = 'tick';

				$mask = '<img src="'.sq_web_path('lib').'/web/images/'.$display_image.'.gif" />';
				echo $mask.' '.translate('option_'.strtolower($option)).'<br />';
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
	* Paints the root node selection box for an LDAP Bridge asset
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintLdapBridgeAsset(&$asset, &$o, $prefix)
	{
		// get root links
		$ldap_bridge_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'ldap_bridge', FALSE, 'ldap_bridge');
		if (empty($ldap_bridge_link)) {
			$ldap_bridge_asset_id = NULL;
		} else {
			$ldap_bridge_asset_id = $ldap_bridge_link['minorid'];
		}

		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_ldap_bridge', $ldap_bridge_asset_id, Array('ldap_bridge'=>'D'));
		} else {
			if (empty($ldap_bridge_asset_id)) {
				echo 'No LDAP bridge selected';
			} else {
				echo get_asset_tag_line($ldap_bridge_asset_id);
			}
		}
		return TRUE;

	}//end paintLdapBridgeAsset()


	/**
	* Process the root node selection box for an LDAP Bridge asset
	*
	* @param object	&$asset	asset being painted
	* @param object	&$o		backend outputter
	* @param string	$prefix	prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processLdapBridgeAsset(&$asset, &$o, $prefix)
	{	
		// This one is last edit process function for attribute available in details screen
		// hence make call to connectToLdap() to sync the asset's LDAP bridge's attributes
		if ($asset->writeAccess('attributes')) {
			@$asset->connectToLdap(TRUE);
		}

		$new_ldap_bridge_id = @$_REQUEST[$prefix.'_ldap_bridge']['assetid'];

		if (!$asset->writeAccess('links')) {			
			return FALSE;
		}

		$current_ldap_bridge_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'ldap_bridge', FALSE, 'ldap_bridge');
		$current_ldap_bridge_asset_id = @$current_ldap_bridge_link['minorid'];

		if (!empty($current_ldap_bridge_link)) {
			// if the new ldap_bridge id is not null and is equal to the current
			// ldap_bridge asset id then return
			if ((!empty($new_ldap_bridge_id)) && ($current_ldap_bridge_asset_id === $new_ldap_bridge_id)) {
				return TRUE;
			} else {
				// else means that we want to create a new link or we want to
				// clear the link
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($current_ldap_bridge_link['linkid']);

				// if the new ldap_bridge id is empty means we want to clear the
				// link we got nothing else to do so return TRUE
				if (empty($new_ldap_bridge_id))return TRUE;
			}
		}

		if (!empty($new_ldap_bridge_id)) {
			$new_ldap_bridge_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_ldap_bridge_id);
			$result_id = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $new_ldap_bridge_asset, SQ_LINK_NOTICE, 'ldap_bridge');
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_ldap_bridge_asset);
			
			if (empty($result_id)) {
				trigger_error('Could not link the ldap_bridge to this asset', E_USER_WARNING);
				return FALSE;
			}
		}

		return TRUE;

	}//end processLdapBridgeAsset()
	

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
		<p class="sq-backend-warning"><strong><?php echo translate('ldap_not_available_line_1'); ?></strong></p>
		<p><?php echo translate('ldap_not_available_line_2'); ?></p>
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
	
	
//-- DYNAMIC PARAMETERS --//

	/**
	* Paint the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function paintDynamicData(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$var_list = $asset->attr('dynamic_vars');

		$o->openRaw();

		echo '<table class="sq-backend-table">';
		echo '<tr>';
		echo '<th>Name</th>';
		echo '<th>Default Value</th>';
		if ($write_access) {
			echo '<th width="10%" align="right" style="border-left: dotted 1px grey;">Delete</th>';
		}
		echo '</tr>';
		foreach ($var_list as $var => $value) {
			$var_id = base64_encode($var);

			echo '<tr>';
			echo '<td>';
			if ($write_access) {
				text_box($prefix.'_var_name['.$var_id.']', $var);
			} else {
				echo $var;
			}
			echo '</td>';
			echo '<td valign="top" align="left">';
			$value = urldecode($value);
			if ($write_access) {
				text_box($prefix.'_var_value['.$var_id.']', urlencode($value));
			} else {
				echo urlencode($value);
			}
			echo '</td>';
			if ($write_access) {
				echo '<td style="border-left: dotted 1px grey;">';
				check_box($prefix.'_delete_var['.$var_id.']');
				echo '</td>';
			}
			echo '</tr>';
		}

		if ($write_access) {
			echo '<tr class="new-data-row">';
			echo '<th colspan="3" align="left">';
			echo '<em style="color:#666">Add New Variables Below</em>';
			echo '</th>';
			echo '</tr>';

			echo '<tr class="new-data-row">';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_name[]', NULL);
			echo '</td>';
			echo '<td>';
			text_box($prefix.'_new_dynamic_data_value[]', NULL);
			echo '</td>';
			echo '<td style="border-left: dotted 1px grey;">';
			echo '</td>';
			echo '</tr>';
		}
		echo '</table>';

		$o->closeRaw();

		$o->sectionNote(translate('ldap_data_source_variable_section_note'));

		$o->closeSection();

		$o->openSection('Data Mappings');
		$o->openRaw();
		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();

		foreach ($var_list as $var_name => $default) {
			$map->setParameter($var_name, 'Variable: '.$var_name);
		}

		$map->paint($prefix.'_parameter_map', !$write_access);
		$o->closeRaw();

		$o->sectionNote(translate('ldap_data_source_mappings_section_note'));

		return $write_access;

	}//end paintDynamicData()


	/**
	* Process the dynamic data interface
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	&$o		Backend outputter
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function processDynamicData(&$asset, &$o, $prefix)
	{
		$var_name_pattern = '/^[a-zA-Z_\-0-9\.]+$/';


		$write_access = $asset->writeAccess('attributes');
		if (!$write_access) return FALSE;

		$to_delete = array_get_index($_REQUEST, $prefix.'_delete_var', Array());
		$var_value = array_get_index($_REQUEST, $prefix.'_var_value', Array());
		$var_name = array_get_index($_REQUEST, $prefix.'_var_name', Array());

		$var_list = $asset->attr('dynamic_vars');

		$map = $asset->getAttribute('parameter_map');

		$parameter_names = $map->getParameters();
		foreach ($var_list as $name => $default) {
			$map->setParameter($name, 'Variable: '.$name);
		}

		$map->process($prefix.'_parameter_map');
		
		foreach ($var_value as $hash => $value) {
			$name = base64_decode($hash);
			if (isset($to_delete[$hash])) {
				unset($var_list[$name]);
				// also need to delete it from the PARAMETER MAP
				if (in_array($name, $parameter_names)) {
					$map->unsetParameter($name);
				}

				continue;
			}
			$value = urldecode($value);
			$var_list[$name] = urlencode($value);
		}

		$new_var_list = Array();
		$rename_success = FALSE;

		// now we rename variables
		foreach ($var_name as $hash => $new_name) {
			$new_name = trim($new_name);

			if (empty($new_name)) {
				$rename_success = FALSE;
				trigger_error('Variable name cannot be empty. No variables were renamed.', E_USER_NOTICE);
				break;
			}

			// check new name against regex
			$new_name_is_valid = preg_match($var_name_pattern, $new_name);
			if (!$new_name_is_valid) {
				$rename_success = FALSE;
				trigger_error('Variable name ['.htmlentities($new_name).'] contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
				break;
			}

			$name = base64_decode($hash);

			// skip the deleted vars
			if (!isset($var_list[$name])) continue;

			if (isset($new_var_list[$new_name])) {
				$rename_success = FALSE;
				trigger_error('Could not rename variables because variable name "'.htmlentities($new_name).'" already exists', E_USER_NOTICE);
				break;
			}

			if ($new_name != $name) {
				if (in_array($name, $parameter_names)) {
					$map->moveParameter($name, $new_name);
				}
				$rename_success = TRUE;
			}

			$new_var_list[$new_name] = $var_list[$name];

		}//end foreach


		if ($rename_success) $var_list = $new_var_list;

		$new_names = array_get_index($_REQUEST, $prefix.'_new_dynamic_data_name', Array());
		if (!empty($new_names)) {

			foreach ($new_names as $seq_number => $new_var_name) {
				$new_var_name = trim($new_var_name);

				$new_var_value = array_get_index($_REQUEST[$prefix.'_new_dynamic_data_value'], $seq_number, '');
				if (empty($new_var_name)) {
					if (!empty($new_var_value)) {
						trigger_error('New variable name cannot be empty', E_USER_WARNING);
					}
					continue;
				}

				// check new name against regex
				$new_name_is_valid = preg_match($var_name_pattern, $new_var_name);
				if (!$new_name_is_valid) {
					trigger_error('Variable name "'.htmlentities($new_var_name).'" contains characters that are not allowed. Can only use letters (A-Za-z), digits(0-9), underscore (_), dash (-) and dot (.)', E_USER_NOTICE);
					continue;
				}

				if (isset($var_list[$new_var_name])) {
					trigger_error('Variable Name "'.htmlentities($new_var_name).'" is already in use', E_USER_WARNING);
					continue;
				}
				$new_var_value = urldecode($new_var_value);
				$var_list[$new_var_name] = urlencode($new_var_value);
				$map->setParameter($new_var_name, 'Variable: '.$new_var_name);
			}
		}

		$asset->setAttrValue('dynamic_vars', $var_list);
		$asset->setAttrValue('parameter_map', unserialize($map->value));

		return TRUE;

	}//end processDynamicData()


//-- LDAP SEARCH FILTER QUERY --//

	/**
	* Paints the interface for editing search filter query
	*
	* @param object	&$asset	the DB Data source asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintLdapSearchFilter(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$search_filter = $asset->attr('search_filter');

		if ($wa) {
			$dynamic_vars = $asset->attr('dynamic_vars');
			if (!empty($dynamic_vars)) {
				foreach ($dynamic_vars as $var => $value) {
					$keywords['%%'.$var.'%%'] = $var;
				}
				keyword_selector($keywords, $prefix.'_search_filter');
			}
			text_area($prefix.'_search_filter', $search_filter, 60, 100, 0, 'style="width: 100%; height: 30em; font-family: \'Courier New\', Courier, monospace; display: block"');
		} else {
			if (!empty($search_filter)) {
				echo nl2br($search_filter);
				hidden_field($prefix.'_search_filter', $search_filter);
			}
		}
		echo('&nbsp;');

		return $wa;

	}//end paintLdapSearchFilter()


	/**
	* Processes the interface for editing search filter query
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLdapSearchFilter(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_search_filter'])) {
			return $asset->setAttrValue('search_filter', $_POST[$prefix.'_search_filter']);
		}

		return FALSE;

	}//end processLdapSearchFilter()


}//end class

?>