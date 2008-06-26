<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: form_action_trim_submit.inc,v 1.1 2008/06/26 04:18:52 hnguyen Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';

/**
* Form_Action_TRIM_Submit
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/


class Form_Action_TRIM_Submit extends Form_Action
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
		self::_fillDefaults($settings);
		$record_type  		= array_get_index($settings, 'record_type', '');
		$additional_fields	= array_get_index($settings, 'additional_fields', '');
		?>

		<p class="sq-backend-section-subheading">Connection Details</p>

		<div id="<?php echo $prefix ?>_connector_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td><p>TRIM Data Source Asset</p></td>
					<td><p><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('data_source_trim'=>'D')); ?><br/>
					Note: Select a TRIM Data Source used to connect to TRIM.</p></td>
				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading">Create Record Settings</p>

		<div id="<?php echo $prefix ?>_record_type_selection" style="padding: 0.5em 0px 2em 30px;">
			<table style="width:auto;padding-left: 30px;">
				<tr>
					<td><strong><?php echo translate('record_type');?></strong></td>
					<td>
						<?php
							$options	= Array (
											'document'	=> 'Document',
										  );
							combo_box($prefix.'_record_type', $options, FALSE, $record_type);
						?>
					</td>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">

			<p class="sq-backend-section-subheading">Mandatory Fields Mapping</p>

			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table style="width:auto;padding-left: 30px;">
					<?php
						$required_fields = self::getRecordTypeFields($record_type);
						foreach ($required_fields as $field_name) {
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width:118px;padding-left:15px;"><strong><?php echo ucwords(str_replace('_', ' ', $field_name)); ?></strong></td>
						<td class="sq-backend-table-cell">
						<?php
							if ($field_name == 'record_notes') {
								text_area($prefix.'_'.$record_type.'_'.$field_name, array_get_index($settings, $record_type.'_'.$field_name, ''), 35, 4);
							} else {
								text_box($prefix.'_'.$record_type.'_'.$field_name, array_get_index($settings, $record_type.'_'.$field_name, ''), 35);
							}//end if
						?>
						</td>
					</tr>
					<?php }//end foreach
					?>
				</table>
			</div>

			<p class="sq-backend-section-subheading">Additional Fields Mapping</p>

			<?php

			if (!empty($additional_fields)) {
			?>
			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table style="width:auto;padding-left: 30px;">
					<tr>
						<td><?php echo translate('field_name') ?></td>
						<td><?php echo translate('field_value') ?></td>
						<td><?php echo translate('delete_question'); ?></td>
					</tr>
					<?php
						foreach ($additional_fields as $field_name => $field_value) {
					?>
					<tr>
						<td class="sq-backend-table-cell" style="width:118px;padding-left:15px;"><strong><?php echo $field_name; ?></strong></td>
						<td class="sq-backend-table-cell" style="width:165px;padding-left:15px;">
						<?php
							echo $field_value;
						?>
						</td>
						<td  class="sq-backend-table-cell">
						<?php
							check_box($prefix.'_addtional_field_remove', $field_name, FALSE);
						?>
						</td>
					</tr>
					<?php }//end foreach
					?>
				</table>
			</div>
			<?php
			}//end if
			?>
			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table style="width:auto;padding-left: 30px;">
					<tr>
						<td>New Field Name</td>
						<td>New Field Value</td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><strong><?php text_box($prefix.'_addtional_field_name', '', 25); ?></strong></td>
						<td class="sq-backend-table-cell">
						<?php
							text_box($prefix.'_addtional_field_value', '', 35);
						?>
						</td>
					</tr>
				</table>
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
		$settings['record_type']		= $_POST[$prefix.'_record_type'];
		$required_fields				= self::getRecordTypeFields($settings['record_type']);
		foreach ($required_fields as $field_name) {
			if (isset($_POST[$prefix.'_'.$settings['record_type'].'_'.$field_name])) {
				$settings[$settings['record_type'].'_'.$field_name]	= $_POST[$prefix.'_'.$settings['record_type'].'_'.$field_name];
			}//end if
		}//end foreach

		if (isset($_POST[$prefix.'_addtional_field_remove'])) {
			$remove_field_name	= $_POST[$prefix.'_addtional_field_remove'];
			if (isset($settings['additional_fields'][$remove_field_name])) {
				unset($settings['additional_fields'][$remove_field_name]);
			}//end if
		}//end if
		if (isset($_POST[$prefix.'_addtional_field_name']) && !empty ($_POST[$prefix.'_addtional_field_name'])) {
			$settings['additional_fields'][$_POST[$prefix.'_addtional_field_name']]	= $_POST[$prefix.'_addtional_field_value'];
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
		self::_fillDefaults($settings);

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
						?><span class="sq-backend-warning">No data source specified.</span><?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Create Record Type</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['record_type'])) {
						echo ellipsisize(preg_replace('/\\n/', ' ', htmlspecialchars($settings['record_type'])), 512);
					} else {
						?><span class="sq-backend-warning">No record type selected.</span><?php
					} ?></p></td>
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
		$found = 0;
		$datasource_trim	= $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
		$required_fields	= self::getRecordTypeFields($settings['record_type']);
		$additional_fields	= array_get_index($settings, 'additional_fields', Array());

		$new_record_info['record_type']	= array_get_index($settings, 'record_type', '');

		foreach ($required_fields as $field_name) {
			if (isset($settings[$settings['record_type'].'_'.$field_name])) {
				$new_record_info[$field_name] = $settings[$settings['record_type'].'_'.$field_name];
				// We will replace all responses, including raw values.
				// We will also replace keywords relating to the form submission asset
				// (eg. submission assetid, time, IP address)
				$field_content	= $settings[$settings['record_type'].'_'.$field_name];
				self::getKeywordReplacementsForForm($field_name, $field_content, $new_record_info, $form);
			}//end if
		}//end foreach

		foreach ($additional_fields as $field_name => $field_content) {
			self::getKeywordReplacementsForForm($field_name, $field_content, $additional_fields, $form);
		}//end foreach

		$username	= '';
		if (!empty($GLOBALS['SQ_SYSTEM']->user)) {
			$username	= $GLOBALS['SQ_SYSTEM']->user->attr('username'); log_dump($username);
			$username	= 'TRIMAdmin';	// REMOVE THIS LINE
		}//end if

		$new_record_info['additional_fields']	= $additional_fields;

		$new_record_uri	= $datasource_trim->executeCreateRequest($username, $new_record_info);

		return TRUE;

	}//end execute()


	/**
	* This function replace all the keywords for each of the field
	*
	* @return void
	* @access public
	*/
	public static function getKeywordReplacementsForForm($field_name, $field_content, &$replacement, &$form)
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
	}//end getKeywordReplacementsForForm()


	/**
	* Execute form action
	*
	* @return void
	* @access public
	*/
	public static function isValid(Form $form, $settings)
	{
		self::_fillDefaults($settings);

		// Must have a TRIM Bridge to be valid
		if (empty($settings['connector_assetid'])) {
			return FALSE;
		}

		// Must selected a record type to create
		if (empty($settings['record_type'])) {
			return FALSE;
		}//end if

		// All the mandatory fields needs to be filled in
		if (!empty($settings['record_type'])) {
			$required_fields = self::getRecordTypeFields($settings['record_type']);

			foreach ($required_fields as $field_name) {
				if (empty($settings[$settings['record_type'].'_'.$field_name])) {
					return FALSE;
				}//end if
			}//end foreach
		}//end if

		return TRUE;

	}//end isValid()


	/**
	* Fill any missing values from settings
	*
	* @param array	&$settings	Action settings
	*
	* @return void
	*/
	private static function _fillDefaults(&$settings) {
		if (!isset($settings['record_type'])) {
			$settings['record_type'] = '';
		}//end if

		if (!isset($settings['connector_assetid'])) {
			$settings['connector_assetid'] = 0;
		}//end if

	}//end _fillDefaults()


	private static function getRecordTypeFields($name)
	{
		$func_name	= 'get'.$name.'TypeFields';
		$fields = self::$func_name();

		return $fields;
	}//end getRecordTypeFields()


	private static function getBoxTypeFields()
	{
		$field_list	= Array (
						'Name', 'Val'
					  );
		return $field_list;

	}//end getBoxTypeFields()

	private static function getDocumentTypeFields()
	{
		$field_list	= Array (
						'record_title', 'parent_container', 'securiry_level', 'record_notes'
					  );

		return $field_list;

	}//end getBoxTypeFields()

	private static function getFileTypeFields()
	{
		$field_list	= Array (
						'Name', 'FileSize'
					  );
		return $field_list;

	}//end getBoxTypeFields()


}//end class

?>
