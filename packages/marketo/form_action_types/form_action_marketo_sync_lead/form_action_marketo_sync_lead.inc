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
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_PACKAGES_PATH.'/marketo/marketo_bridge/marketo_bridge.inc';

/**
* Form_Action_Marketo_Sync_Lead
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage marketo
*/


class Form_Action_Marketo_Sync_Lead extends Form_Action
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
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		$attributes	= array_get_index($settings, 'attributes_mapping', '');
		?>

		<p class="sq-backend-section-subheading"><?php echo translate('Connection Details'); ?></p>


		<div id="<?php echo $prefix ?>_connector_div">
			<table class="sq-backend-table">
			<col width="15%" />
			<col width="85%" />
				<tr>
					<th style='min-width:207px'><?php echo translate('Marketo Bridge Asset'); ?></th>

					<td><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('marketo_bridge'=>'D')); ?></td>
				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading"><?php echo translate('Attribute Sync Settings'); ?></p>


			<?php

			if (!empty($attributes)) {
			?>
			<div id="<?php echo $prefix ?>_attribute_mapping" >
				<table border="1" style="margin-bottom: 7px" class="sq-backend-table">
				<col width="15%" />
				<col width="75%" />
				<col width="10%" />
					<tbody>
					<tr>
						<th style='min-width:200px'><?php echo translate('Attribute Name'); ?></th>

						<th><?php echo translate('Attribute Value'); ?></th>

						<th><?php echo translate('Delete?'); ?></th>

					</tr>
					<?php
						foreach ($attributes as $attribute_name => $attribute_value) {
					?>
					<tr>
						<td class="sq-backend-table-cell" style="padding-left:15px;"><strong>
						<div style='min-width:200px'><?php echo $attribute_name; ?></strong></div>
						<td class="sq-backend-table-cell" style="padding-left:5px;">
						<?php
							text_box($prefix.'_attribute_value_edit['.$attribute_name.']', $attribute_value, 60);
						?>
						</td>
						<td  class="sq-backend-table-cell" style="width:100%; text-align: center;vertical-align: middle;">
						<?php
							check_box($prefix.'_attribute_remove[]', $attribute_name, FALSE);
						?>
						</td>
					</tr>
					</tbody>
					<?php }//end foreach
					?>
				</table>
			</div>
			<?php
			}//end if

			?>
			<div id="<?php echo $prefix ?>_new_attribute_mapping">
				<table border="1" style="margin-bottom: 7px;" class="sq-backend-table">
					<tr>
						<th><div style='min-width:207px'><?php echo translate('Attribute Name'); ?></div></th>
						<td class="sq-backend-table-cell" style="width:100%"><?php
							text_box($prefix.'_new_attribute_name','',60);
							?></td>
					</tr>
					<tr>
						<th><?php echo translate('Attribute Value'); ?></th>
						<td class="sq-backend-table-cell"><?php
							text_box($prefix.'_new_attribute_value','', 60);
							?></td>
					</tr>
				</table>
			    <div class="sq-backend-section-note"><?php echo translate('<strong>Note: </strong>Marketo Cookie is required to be set for this action to work.<br/>Global keywords and form response keywords are supported in the attribute value. <br/>If Email attribute is specified, it will be passed to Marketo directly. Verified Email address will be automatically synced to Marketo.') ?></div>

			</div>
		</div>

		<?php
		//self::execute($form, $settings);

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @return boolean
	* @access public
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		$settings['connector_assetid']	= $_POST[$prefix.'_connector']['assetid'];


		if (isset($_POST[$prefix.'_attribute_remove'])) {
			$remove_field_names	= $_POST[$prefix.'_attribute_remove'];
			foreach ($remove_field_names as $remove_field_name) {
				if (isset($settings['attributes_mapping'][$remove_field_name])) {
					unset($settings['attributes_mapping'][$remove_field_name]);
				}//end if
			}//end foreach
		}//end if
		if (isset($_POST[$prefix.'_new_attribute_name']) && !empty($_POST[$prefix.'_new_attribute_name'])) {
			$settings['attributes_mapping'][$_POST[$prefix.'_new_attribute_name']]	= $_POST[$prefix.'_new_attribute_value'];
		}//end if

		if (isset($_POST[$prefix.'_attribute_value_edit']) && !empty($_POST[$prefix.'_attribute_value_edit'])) {
			$editing_list	= $_POST[$prefix.'_attribute_value_edit'];
			foreach ($editing_list as $field_name	=> $field_value) {
				if (isset($settings['attributes_mapping'][$field_name]) && $settings['attributes_mapping'][$field_name] != $field_value) {
					$settings['attributes_mapping'][$field_name]	= $field_value;
				}//end if
			}//end foreach

		}//end if

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @return void
	* @access public
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{

		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Data Source:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['connector_assetid'])) {
						echo get_asset_tag_line($settings['connector_assetid']);
					} else {
						?><span class="sq-backend-warning">No connection details specified.</span><?php
					}
					?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	* @param
	* @param
	*
	* @return boolean
	* @access public
	*/
	public static function execute(Form $form, $settings)
	{

		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
		$attributes	= array_get_index($settings, 'attributes_mapping', Array());


		// replace form keywords
		foreach ($attributes as $name => $value) {
			self::getKeywordReplacementsFromForm($name, $value, $attributes, $form);
		}
		// replace global keywords
		foreach ($attributes as $name => $value) {
			$attributes[$name] = replace_global_keywords($value);
		}

		// at least one of those user identifier attributes has to be available
		$user_identifier = Array();
		if(isset($_COOKIE[MARKETO_COOKIE_NAME])) {
		    $user_identifier['cookie'] = $_COOKIE[MARKETO_COOKIE_NAME];
		}
		if(isset($_SESSION['SQ_VERIFY_EMAIL_VERIFIED'])) {
		    $user_identifier['email'] = $_SESSION['SQ_VERIFY_EMAIL_VERIFIED'];
		}
		if(empty($user_identifier)) {
		    log_error('User identifier is missing. Make sure Marketo Cookie is set or a verified Email address is stored in session.');
		    return FALSE;
		}

		$bridge->syncLead($user_identifier, $attributes);

		return TRUE;

	}//end execute()


	/**
	* This function replace all the keywords for each of the field
	*
	* @return void
	* @access public
	*/
	public static function getKeywordReplacementsFromForm($field_name, $field_content, &$replacement, &$form)
	{
		$matches = Array();
		$found = preg_match_all('/%(response_(\\d+_)?q\\d+(_raw)?)%/U', $field_content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(question_name_(\\d+_)?q\\d+(_raw)?)%/U', $field_content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(form_submission_.*)%/U', $field_content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);

		foreach ($matches as $match) {
			if (empty($match)) continue;
			$count = 0;
			do {
				$field_content = preg_replace('/%'.$match[1].'%/U', $form->_getThankYouKeywordReplacement($match[1]), $field_content, 1, $count);
				$replacement[$field_name]	= $field_content;
			} while ($count > 0);
		}//end foreach
	}//end getKeywordReplacementsFromForm()


	/**
	* Execute form action
	*
	* @return void
	* @access public
	*/
	public static function isValid(Form $form, $settings)
	{

		// Must have a TRIM Bridge to be valid
		if (empty($settings['connector_assetid'])) {
			return FALSE;
		}

		return TRUE;

	}//end isValid()


}//end class

?>
