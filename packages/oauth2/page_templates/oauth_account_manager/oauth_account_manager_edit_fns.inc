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
* $Id: oauth_account_manager_edit_fns.inc,v 1.1.1.1 2013/07/19 00:29:26 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_builder/page_asset_builder_edit_fns.inc';

/**
* Oauth_Account_manager_Edit_Fns
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth_Account_Manager_Edit_Fns extends Page_Asset_Builder_Edit_Fns
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
	* Paint the interface for setting the oauth token
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintOauthToken(&$asset, Backend_Outputter $o, $prefix)
	{
		return $this->paintNoticeLinkedAsset($asset, 'oauth_token_source', Array('oauth2_token' => 'D'));
	}//end paintOauthToken()


	/**
	* Processes the setting of oauth token
	*
	* @param Site				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processOauthToken(&$asset, Backend_Outputter $o, $prefix)
	{
		return $this->processNoticeLinkedAsset($asset, 'oauth_token_source');

	}//end processOauthToken()

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
		return $this->paintNoticeLinkedAsset($asset, 'redirect_asset');
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
		return $this->processNoticeLinkedAsset($asset, 'redirect_asset');
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



	public function paintLinkedAccounts(&$asset, &$o, $prefix)
	{
		$page_size = 100;

		$o->openRaw();

		$bind_vars = Array(
				'assetid'      => $asset->id,
			    );
		$result = MatrixDAL::executeAll('oauth2_package', 'getLinks', $bind_vars);

		if (count($result) > 0) {
			$num_pages = ceil(count($result) / $page_size);
			$page_num = array_get_index($_POST, $prefix.'_usage_page', 1);
			$page_num = max($page_num, 1);
			$page_num = min($page_num, $num_pages);

			if ($num_pages == 1) {
				$page_tag = sprintf(translate('Page %1$s of %2$s'), $page_num, $num_pages);

			} else {
				$page_tag = sprintf(translate('Page <a style="color: #fff" href="#" onclick="return sq_pager_jump(\'%3$s\', sq_pager_prompt(1, %2$s));">%1$s</a> of %2$s'), $page_num, $num_pages, $prefix.'_usage_page');

			}
			$asset_count_tag = sprintf(translate('Displaying <strong>%1$s</strong> - <strong>%2$s</strong> of <strong>%3$s</strong> total %4$s'), ($page_num - 1) * $page_size + 1, min(count($result), $page_num * $page_size), count($result), strtolower(translate('Links')));



			$result = array_slice($result, ($page_num - 1) * $page_size, $page_size);

			$links = Array(
						'first'		=> '&lt;&lt;',
						'previous'	=> '&lt;',
						'page'		=> $page_tag,
						'next'		=> '&gt;',
						'last'		=> '&gt;&gt;',
					);

			if ($page_num > 1) {
				$links['first'] = '<a title="'.translate('Go to the first page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', 1)">'.$links['first'].'</a>';

				$links['previous'] = '<a title="'.translate('Go to the previous page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num - 1).')">'.$links['previous'].'</a>';

			} else {
				$links['first'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['first'].'</span>';

				$links['previous'] = '<span title="'.translate('This is the first page; you cannot go any further back').'." style="color: #333">'.$links['previous'].'</span>';

			}

			if ($page_num < $num_pages) {
				$links['last'] = '<a title="'.translate('Go to the last page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.$num_pages.')">'.$links['last'].'</a>';

				$links['next'] = '<a title="'.translate('Go to the next page').'" style="text-decoration: none; color: #fff" href="#" onClick="return sq_pager_jump(\''.$prefix.'_usage_page\', '.($page_num + 1).')">'.$links['next'].'</a>';

			} else {
				$links['last'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['last'].'</span>';

				$links['next'] = '<span title="'.translate('This is the last page; you cannot go any further forward').'." style="color: #333">'.$links['next'].'</span>';

			}

			hidden_field($prefix.'_usage_page', $page_num); ?>
			<table class="sq-backend-table">
			<thead>
			<tr class="sq-backend-table-row">
				<td class="sq-backend-table-header-header"><?php echo implode(' &nbsp; &nbsp; ', $links) ?></td>
				<td class="sq-backend-table-header-header" colspan="2" style="text-align: right; font-weight: normal"><?php echo $asset_count_tag; ?></span></td>
			</tr>
			<tr class="sq-backend-table-row">
				<th class="sq-backend-table-cell"><?php echo translate('Matrix User'); ?></th>

				<th style="" class="sq-backend-table-cell"><?php echo translate('Oauth User') ?></th>

				<th style="" class="sq-backend-table-cell"><?php echo translate('Created') ?></th>

			</tr>
			</thead>
			<tbody>
			<?php

			foreach ($result as $data) {
					?><tr class="sq-backend-table-row">
						<td class="sq-backend-table-cell">
						<?php

						$tag_line = get_asset_tag_line($data['matrix_userid'], 'details');
						echo sprintf(translate('%1$s'), $tag_line);

						?></td>
						<td class="sq-backend-table-cell">
							<?php echo htmlspecialchars($data['oauth_userid']); ?>
						</td>
						<td class="sq-backend-table-cell">
							<?php echo $data['created']; ?>
						</td>
					</tr>
					<?php

			}//end foreach top tree
			?>

			</tbody>
			</table>
			<?php

		} else {
			$o->note(translate('There is no linked accounts.'));

		}

		$o->closeRaw();

		return FALSE;

	}//end paintLinkedAccounts()




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
	}//end processPrefillFields()


}//end class

?>
