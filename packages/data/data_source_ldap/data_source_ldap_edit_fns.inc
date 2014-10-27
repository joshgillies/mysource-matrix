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
* $Id: data_source_ldap_edit_fns.inc,v 1.5 2012/08/30 00:58:16 ewang Exp $
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
* @version $Revision: 1.5 $
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