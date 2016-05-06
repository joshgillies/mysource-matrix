<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: saml_account_manager_edit_fns.inc,v 1.3 2012/08/30 00:59:03 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_builder/page_asset_builder_edit_fns.inc';

/**
* Physical_Folder_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class SAML_Account_Manager_Edit_Fns extends Page_Asset_Builder_Edit_Fns
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
	* Paint the interface to display if simplesamlphp lib is installed
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSimplesamlphpInstall (&$asset, &$o, $prefix)
	{
		$installed = $asset->isSimplesamlphpInstalled();
		if ($installed) {
			echo '<span style="color:green;font-weight:bold">Installed</span>';
		} else {
			echo '<span style="color:red;font-weight:bold">Not Installed</span>';
		}//end if

	}//end paintSimplesamlphpInstall()


	/**
	* Paint the interface for choosing authSource defined in simplesamlphp library
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAuthSource(&$asset, &$o, $prefix)
	{
		$installed = $asset->isSimplesamlphpInstalled();
		if ($installed) {
			require SQ_TOOL_SIMPLESAMLPHP_PATH.'/config/authsources.php';
			$options[''] = '';
			foreach ($config as $name => $value) {
			    $options[$name] = $name;
			}

			if (!$asset->writeAccess('attributes')) {
			    echo $options[$asset->attr('auth_source')];
			} else {
				combo_box($prefix.'_auth_source', $options, FALSE, $asset->attr('auth_source'));
			}
		}

	}//end paintAuthSource()


	/**
	* Process the interface of setting authSource
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAuthSource(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_auth_source'])) {
			$id = trim($_POST[$prefix.'_auth_source']);
			if (empty($id)) return FALSE;
			$asset->setAttrValue('auth_source', $id);
			return TRUE;
		}
		return FALSE;

	}//end processAuthSource()

	/**
	* Paint the SP metadata download URL
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMetadataUrl(&$asset, &$o, $prefix)
	{
		if($asset->attr('auth_source')) {
			 $url = $asset->getUrl();
			 echo '<a href="'.$url.'?showMetadata">'.$url.'?showMetadata'.'</a>';
		}
	}

		/**
	* Paint the SP metadata download URL
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAcsUrl(&$asset, &$o, $prefix)
	{
		$acs_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'saml2_acs', TRUE, 'saml2_assertion_consumer_service', 'major', '1');
		$acs_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($acs_asset_link['minorid']);
		$asset_urls = $acs_asset->getLookups();
		usort($asset_urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
		$acs_urls = Array();

		if ($acs_asset) {
			echo '<ul>';
			foreach ($asset_urls as $index => $acs_url_info) {
				if ($acs_url_info['http']) {
					echo '<li> http://'.$acs_url_info['url'].'</li>';
				}
				if ($acs_url_info['https']) {
					echo '<li> https://'.$acs_url_info['url'].'</li>';
				}
			}
			echo '</ul>';
		}

	}//end paintAcsUrl()


	/**
	* Paint the SP metadata download URL
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSlsUrl(&$asset, &$o, $prefix)
	{
		$sls_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'saml2_sls', TRUE, 'saml2_single_logout_service', 'major', '1');
		$sls_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sls_asset_link['minorid']);
		$asset_urls = $sls_asset->getLookups();
		usort($asset_urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
		$sls_urls = Array();

		if ($sls_asset) {
			echo '<ul>';
			foreach ($asset_urls as $index => $sls_url_info) {
				if ($sls_url_info['http']) {
					echo '<li> http://'.$sls_url_info['url'].'</li>';
				}
				if ($sls_url_info['https']) {
					echo '<li> https://'.$sls_url_info['url'].'</li>';
				}
			}
			echo '</ul>';
		}

	}//end paintSlsUrl()


	/**
	* Paint the interface for selecting the asset to redirect to
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRedirectAssetid(&$asset, &$o, $prefix)
	{
		$redirect_asset = $asset->getRedirectAsset();
		if ($asset->writeAccess('links')) {
			asset_finder($prefix.'_redirect_assetid', (is_null($redirect_asset) ? 0 : $redirect_asset->id));
		} else {
			if (is_null($redirect_asset)) {
				echo translate('None Set');

			} else {
				echo get_asset_tag_line($redirect_asset->id);
			}
		}
		return TRUE;

	}//end paintRedirectAssetid()


	/**
	* Process the interface for selecting the asset to redirect to
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRedirectAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_redirect_assetid']['assetid']) || (isset($asset->_tmp['reverting_to_system_version']) && $asset->_tmp['reverting_to_system_version'])) {
			return TRUE;
		}

		$redirect_assetid = $_POST[$prefix.'_redirect_assetid']['assetid'];

		if ($redirect_assetid) {
			$redirect_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($redirect_assetid);
		} else {
			$redirect_asset = NULL;
		}

		return $asset->setRedirectAsset($redirect_asset);

	}//end processRedirectAssetid()



	/**
	* Paint the interface for choosing the user type that is created
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			parent::paintAssetTypes($asset, $o, $prefix);
		} else {
			$user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user');

			$options = Array('user' => 'User');
			foreach ($user_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, FALSE, array_keys($asset->attr('create_type')));
		}

	}//end paintAddCreateType()


	/**
	* Process the section for choosing the user type that is created
	*
	* This is overridden from Asset Builder because it makes sense for the account
	* manager to still be only allowed to create one type of user from the frontend interface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddCreateType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_create_type'])) {
			$type = trim($_POST[$prefix.'_create_type']);
			if (empty($type)) return FALSE;
			$types = Array($type => TRUE);
			$asset->setAttrValue('create_type', $types);
			return TRUE;
		}
		return FALSE;

	}//end processAddCreateType()



	/**
	* Paints the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCreateFormCustomisation(&$asset, &$o, $prefix)
	{
		$customised_types = $asset->attr('create_form_customisations');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			hidden_field($prefix.'_customise_form_submitted', 1);
			echo check_box($prefix.'_customise_form', 1, !empty($customised_types));
		} else {
			$fn = !empty($customised_types) ? 'tick' : 'cross';
			echo '<img src="'.sq_web_path('lib').'/web/images/'.$fn.'.png" />';
		}

	}//end paintCreateFormCustomisation()


	/**
	* Processes the interface for choosing create types whose form should be customised
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateFormCustomisation(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes') && isset($_POST[$prefix.'_customise_form_submitted'])) {
			$customise = array_get_index($_POST, $prefix.'_customise_form');
			$customised_types = $customise ? array_keys($asset->attr('create_type')) : Array();
			return $asset->setAttrValue('create_form_customisations', $customised_types);
		}
		return FALSE;

	}//end processCreateFormCustomisation()




	/**
	* A tool to parse IDP XML metadata to simplesamlphp config array
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintParseMetadata (&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$xmldata = isset($_POST[$prefix.'_parse_metadata']) ? trim($_POST[$prefix.'_parse_metadata']) : '';
		if ($wa) {
		    if (!empty($xmldata)) {
			try {
			SimpleSAML_Utilities::validateXMLDocument($xmldata, 'saml-meta');
			$entities = SimpleSAML_Metadata_SAMLParser::parseDescriptorsString($xmldata);

			/* Get all metadata for the entities. */
			foreach($entities as &$entity) {
				$entity = array(
					'saml20-idp-remote' => $entity->getMetadata20IdP(),
					);

			}

			/* Transpose from $entities[entityid][type] to $output[type][entityid]. */
			$entity_id = array_get_index(array_keys($entities), 0);
			if(empty($entity_id)) return FALSE;
			$output =array_get_index(array_get_index(SimpleSAML_Utilities::transposeArray($entities), 'saml20-idp-remote'), $entity_id);
			if(empty($output)) return FALSE;
			}
			catch (Exception $e) {
			    trigger_error($e->getMessage());
			    return TRUE;
			}

			// export the config array in the simplesamlphp format
			echo "<pre>";
			echo ('$metadata[\''.$entity_id.'\'] = ');
			var_export($output);
			echo (';');
			echo "</pre>";
			return TRUE;
		    }
		    else {
			text_area($prefix.'_parse_metadata', '', 100, 5);
		    }
		}
		return TRUE;
	}//end paintParseMetadata()





	/**
	* Use extensions elements in Auth Request?
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function use_extensions (&$asset, $prefix)
	{
	    $use_extensions = $asset->attr('use_extensions');
	    return ($use_extensions);
	} // end use_extensions()

	/**
	* Does SAML user id exist in  returned user attributes?
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function saml_userid_in_attribute (&$asset, $prefix)
	{
	    $location = $asset->attr('saml_userid_location');
	    return ($location == 'attribute');
	} // end saml_userid_in_attribute()

	/**
	* Paint the interface for prefilling/sync attributes from user profile api response
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	* @param boolean $prefill prefill attributes or sync attributes, they are stored in different places
	*
	* @return boolean
	* @access public
	*/
	function paintPrefillSyncFields(&$asset, &$o, $prefix, $prefill = TRUE)
	{
		$write_access = $asset->writeAccess('attributes');
		$current = $prefill ? $asset->attr('prefilled_fields') : $asset->attr('synced_fields');
		$metadata_schemas = Array();
		$metadata_schema_list = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'schema_to_apply');
		foreach ($metadata_schema_list as $metadata_schema) {
			$metadata_schemas[] = $metadata_schema['minorid'];
		}

		foreach (array_keys($asset->attr('create_type')) as $create_type) {

			$attributes = Array();
			$attrs = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name'));
			foreach ($attrs as $name) {
				$attributes[$name] = ucwords(str_replace('_', ' ', $name));
			}

			if (!isset($current[$create_type])) {
				$current[$create_type] = Array();
			}
			$type_prefix = $prefix.'_'.$create_type;


			if (!empty($current[$create_type])) {
				?>
				<table class="sq-backend-table">
					<tr>
						<th class="sq-backend-table-header" style="width:190px;" ><?php echo translate('Attribute') ?></th>

						<th class="sq-backend-table-header"><?php echo translate('Value') ?></th>

						<?php
						if ($write_access) {
							?><th class="sq-backend-table-header" align="center" style="text-align:center;"><?php echo translate('Delete?') ?></th><?php

						}
						?>
					</tr>
					<?php
					foreach ($current[$create_type] as $attr => $location) {
						$required_name = '';
						$pos = strpos($attr, $prefix.'_metadata_');
						if ($pos !== FALSE) {
							if (empty($metadata_schemas)) continue;
							$metadata_field = substr($attr, strlen($prefix.'_metadata_'));
							$am = $GLOBALS['SQ_SYSTEM']->am;
							$mf_asset = $am->getAsset($metadata_field);
							if (!is_null($mf_asset)) {
								$required_name = 'Metadata: '.$mf_asset->name;
							} else {
								$required_name = translate('Unknown Metadata Field');

							}
						} else {
							$required_name = $attributes[$attr];
						}
						?>
						<tr>
							<td class="sq-backend-table-cell"><?php echo $required_name; ?></td>
							<td class="sq-backend-table-cell">
								<?php
								if ($write_access) {
									text_box($type_prefix.'_prefilled_field['.$attr.']', $location, 50);
								} else {
									echo $location;
								}
								?>
							</td>
							<?php
							if ($write_access) {
								?>
								<td class="sq-backend-table-cell" align="center">
									<?php check_box($type_prefix.'_delete_prefilled['.$attr.']'); ?>
								</td>
								<?php
							}
							?>
						</tr>
						<?php
					}//end foreach
					?>
				</table>
				<?php
			}//end else

			if ($write_access) {

				$new_required = Array('' => '-- '.translate('Select Attribute').' --');

				foreach ($attributes as $attr => $name) {
					if (isset($current[$create_type][$attr])) continue;
					$new_required[$attr] = $name;
				}
				// If a metadata schema is applied, allow the option to choose Metadata as a required field
				if (!empty($metadata_schemas)) {
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$am = $GLOBALS['SQ_SYSTEM']->am;
					$metadata_fields = $mm->getMetadataFields($metadata_schemas);
					foreach ($metadata_fields as $metadata_field => $metadata_field_type_code) {
						if (isset($current[$create_type][$prefix.'_metadata_'.$metadata_field])) continue;
						// Get each field from the schema
						$m_asset = $am->getAsset($metadata_field);
						if (!is_null($m_asset)) {
							$current_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($m_asset->id, SQ_LINK_TYPE_2, 'metadata_section', FALSE, 'minor', NULL, TRUE);
							foreach ($current_links as $link) {
								$section = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['major_type_code']);
								if (is_null($section)) continue;
								$restrictions = $section->attr('restrict');
								if (empty($restrictions) || (!is_null($create_type) && array_key_exists($create_type, $restrictions))) {
									$new_required[$prefix.'_metadata_'.$metadata_field] = 'Metadata: '.$m_asset->name;
								}
							}//end foreach
						}//end if
						$am->forgetAsset($m_asset);
					}//end foreach
				}

				$o->openField(translate('Add Attribute'));

				combo_box($type_prefix.'_new_prefilled', $new_required, FALSE, '');
				$o->closeField();
			}


		}//end foreach

		return $write_access;

	}//end paintPrefillFields()


	/**
	* Processes the interface for prefilling/sync attributes from user profile api response
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	* @param boolean $prefill prefill attributes or sync attributes, they are stored in different places
	*
	* @return boolean
	* @access public
	*/
	function processPrefillSyncFields(&$asset, &$o, $prefix, $prefill = TRUE)
	{
		$attr_name = $prefill ? 'prefilled_fields' : 'synced_fields';
		$current = $asset->attr($attr_name);
		// preserves the Populate and Sync User Attributes on change of create user type
		$current_type = key($current);

		$required_fields = Array();

		foreach (array_keys($asset->attr('create_type')) as $create_type) {

			if (!isset($current[$create_type])) {
				// preserves the Populate and Sync User Attributes on change of create user type
				if (isset($current[$current_type])) {
					$current[$create_type] = $current[$current_type];
				} else {
					$current[$create_type] = Array();
				}
			}
			$type_prefix = $prefix.'_'.$create_type;

			// construct new  fields array removing deleted fields and updating location
			foreach ($current[$create_type] as $attr => $location) {
				if (!isset($_POST[$type_prefix.'_delete_prefilled'][$attr]) || !$_POST[$type_prefix.'_delete_prefilled'][$attr]) {
					if (isset($_POST[$type_prefix.'_prefilled_field'][$attr])) {
						$required_fields[$create_type][$attr] = trim($_POST[$type_prefix.'_prefilled_field'][$attr]);
					} else {
						$required_fields[$create_type][$attr] = $current[$create_type][$attr];
					}//end else
				}//end if
			}//end foreach

			// add new required field
			if (isset($_POST[$type_prefix.'_new_prefilled']) && !empty($_POST[$type_prefix.'_new_prefilled'])) {
				$new_required = $_POST[$type_prefix.'_new_prefilled'];
				if (!isset($required_fields[$create_type][$new_required])) {
					$required_fields[$create_type][$new_required] = '';
				}
			}
		}

		return $asset->setAttrValue($attr_name, $required_fields);

	}//end processPrefillFields()



	/**
	* Paint the interface for syncing attributes from user profile api response
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*/
	function paintSyncFields(&$asset, &$o, $prefix)
	{
		$this->paintPrefillSyncFields($asset, $o, $prefix.'_sync', FALSE);
	}//end paintSyncFields()

	/**
	* Process the interface for syncing attributes from user profile api response
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*/
	function processSyncFields(&$asset, &$o, $prefix)
	{
		$this->processPrefillSyncFields($asset, $o, $prefix.'_sync', FALSE);
	}//end processSyncFields()


	/**
	* Paint the interface for prefilling attributes from user profile api response
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*/
	function paintPrefillFields(&$asset, &$o, $prefix)
	{
		$this->paintPrefillSyncFields($asset, $o, $prefix, TRUE);
	}//end paintPrefillFields()

	/**
	* Process the interface for prefilling attributes from user profile api response
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*/
	function processPrefillFields(&$asset, &$o, $prefix)
	{
		$this->processPrefillSyncFields($asset, $o, $prefix, TRUE);
	}//end processPrefillFields();

	/**
	* Paint the interface for setting the ldap bridge
	*
	* @param SAML_LDAP_Account_Manager  $asset  the asset whose interface we are painting
	* @param Backend_Outputter          $o      the Backend Outputter object
	* @param string                     $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintLDAPBridge(&$asset, Backend_Outputter $o, $prefix)
	{
		if($GLOBALS['SQ_SYSTEM']->userSystemAdmin()){
			return $this->paintNoticeLinkedAsset($asset, 'ldap_bridge', Array('ldap_bridge' => 'I'));
		}
		
		return FALSE;
	}//end paintRootNode()
	
	
	/**
	* Processes the setting of ldap bridge
	*
	* @param SAML_LDAP_Account_Manager  $asset  the asset whose interface we are painting
	* @param Backend_Outputter          $o      the Backend Outputter object
	* @param string                     $prefix prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processLDAPBridge(&$asset, Backend_Outputter $o, $prefix)
	{
		if($GLOBALS['SQ_SYSTEM']->userSystemAdmin()){
			return $this->processNoticeLinkedAsset($asset, 'ldap_bridge');
		}
		
		return FALSE;
	}//end processLDAPBridge()

	/**
	* Is the user a System admin or higher?
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function saml_ldap_lookup_options(&$asset, $prefix){
		if($GLOBALS['SQ_SYSTEM']->userSystemAdmin()){
			return TRUE;
		}
		
		return FALSE;
	}// end saml_ldap_lookup_options()

}//end class

?>
