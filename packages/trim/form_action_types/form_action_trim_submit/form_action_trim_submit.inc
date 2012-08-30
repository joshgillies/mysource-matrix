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
* $Id: form_action_trim_submit.inc,v 1.9 2012/08/30 00:59:28 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';

/**
* Form_Action_TRIM_Submit
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.9 $
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
		$record_type  		= array_get_index($settings, 'record_type', 'document');
		$additional_fields	= array_get_index($settings, 'additional_fields', '');
		?>

		<p class="sq-backend-section-subheading">Connection Details</p>

		<div id="<?php echo $prefix ?>_connector_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td width="100"><p>WSDL</p></td>
					<td><p><?php text_box($prefix.'_wsdl', array_get_index($settings, 'wsdl', ''), 120); ?></p></td>
				</tr>
				<tr>
					<td><?php echo translate('username'); ?></td>
					<td><p><?php text_box($prefix.'_username', array_get_index($settings, 'username', ''), 35); ?><br />
					</p></td>
				</tr>
				<tr>
					<td><?php echo translate('password'); ?></td>
					<td><p><?php password_box($prefix.'_password', array_get_index($settings, 'password', ''), 35); ?></p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_proxy_connector'); ?></td>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_connector_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td><p>TRIM Connector Asset</p></td>
					<td><p><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('trim_connector'=>'D')); ?><br/>
					Note: If a TRIM connector is selected, the settings in the 'Connection Setup' section (above) have no effect. <br />TRIM Connector allows the TRIM settings to be reused on other TRIM assets..</p></td>
				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading">Create Record Settings</p>

		<div id="<?php echo $prefix ?>_record_type_selection" style="padding: 0.5em 0px 2em 30px;">
			<table style="width:auto;padding-left: 30px;">
				<tr>
					<td><strong><?php echo translate('record_type');?></strong><br /><br /></td>
					<td>
						<?php
							$options	= Array (
											'document'	=> 'Document',
										  );
							combo_box($prefix.'_record_type', $options, FALSE, $record_type);
						?>
						<br /><br />
					</td>
				</tr>
				<tr>
					<td><span style="padding-top:10px;"><strong><?php echo translate('creator_username');?></strong></span></td>
					<td>
						<?php
							text_box($prefix.'_creator_username', array_get_index($settings,'creator_username', ''), 35, '', FALSE, ((boolean) array_get_index($settings, 'session_creator_username', FALSE)) ? ' disabled=true' :'');
							?>
							<br />Use current logged in Matrix User
							<?php
							check_box($prefix.'_session_creator_username', '1', (boolean) array_get_index($settings, 'session_creator_username', FALSE), '
							if (checked ==1) {
								document.getElementById(\''.$prefix.'_creator_username\').disabled = true;
							} else {
								document.getElementById(\''.$prefix.'_creator_username\').disabled = false;
							}
								', ' style="padding-top:2px;"');
						?>
						<br />
					</td>
				</tr>
				<tr>
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
						<td class="sq-backend-table-cell" style="width:180px;padding-left:15px;"><strong><?php echo ucwords(str_replace('_', ' ', $field_name)); ?></strong></td>
						<td class="sq-backend-table-cell">
						<?php
							$security_levels	= TRIM_Common::getSecurityLevel();
							if ($field_name == 'security_level') {
								combo_box($prefix.'_'.$record_type.'_security_level', $security_levels, FALSE, isset($settings[$record_type.'_security_level']) ? $settings[$record_type.'_security_level'] : 'Unrestricted');
							} else {
								text_box($prefix.'_'.$record_type.'_'.$field_name, array_get_index($settings, $record_type.'_'.$field_name, ''), 53);
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
				require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';
				$record_properties	= TRIM_Common::getRecordProperties();
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
						<td class="sq-backend-table-cell" style="width:180px;padding-left:15px;"><strong><?php echo $record_properties[$field_name]; ?></strong></td>
						<td class="sq-backend-table-cell" style="width:165px;padding-left:5px;">
						<?php
							text_area($prefix.'_addtional_field_value_edit['.$field_name.']', $field_value, 50, 3);
						?>
						</td>
						<td  class="sq-backend-table-cell">
						<?php
							check_box($prefix.'_addtional_field_remove[]', $field_name, FALSE);
						?>
						</td>
					</tr>
					<?php }//end foreach
					?>
				</table>
			</div>
			<?php
			}//end if

			require_once SQ_PACKAGES_PATH.'/trim/lib/trim_common.inc';
			$record_properties['']	= 'None';
			$record_properties	= array_merge($record_properties, TRIM_Common::getRecordProperties());
			ksort($record_properties);

			?>
			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table style="width:auto;padding-left: 30px;">
					<tr>
						<td>New Field Name</td>
						<td>New Field Value</td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><strong><?php
							combo_box($prefix.'_addtional_field_name', $record_properties, FALSE, 'None');
						?></strong></td>
						<td class="sq-backend-table-cell">
						<?php
							text_area($prefix.'_addtional_field_value', '', 50, 3);
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

		$new_wsdl						= array_get_index($_POST, $prefix.'_wsdl', '');
		$new_username					= array_get_index($_POST, $prefix.'_username', '');
		$new_password					= array_get_index($_POST, $prefix.'_password', '');
		$creator_username				= array_get_index($_POST, $prefix.'_creator_username', NULL);
		$session_creator_username		= array_get_index($_POST, $prefix.'_session_creator_username', FALSE);

		if (!is_null($new_wsdl)) {
			$settings['wsdl']		= $new_wsdl;
		}//end if
		if (!is_null($new_username)) {
			$settings['username']	= $new_username;
		}//end if
		if (!is_null($new_password)) {
			$settings['password']	= $new_password;
		}//end if
		if (!is_null($creator_username)) {
			$settings['creator_username']	= $creator_username;
		}//end if
		if (!is_null($session_creator_username)) {
			$settings['session_creator_username']	= $session_creator_username;
		}//end if

		foreach ($required_fields as $field_name) {
			if (isset($_POST[$prefix.'_'.$settings['record_type'].'_'.$field_name])) {
				$settings[$settings['record_type'].'_'.$field_name]	= $_POST[$prefix.'_'.$settings['record_type'].'_'.$field_name];
			}//end if
		}//end foreach

		if (isset($_POST[$prefix.'_addtional_field_remove'])) {
			$remove_field_names	= $_POST[$prefix.'_addtional_field_remove'];
			foreach ($remove_field_names as $remove_field_name) {
				if (isset($settings['additional_fields'][$remove_field_name])) {
					unset($settings['additional_fields'][$remove_field_name]);
				}//end if
			}//end foreach
		}//end if
		if (isset($_POST[$prefix.'_addtional_field_name']) && !empty($_POST[$prefix.'_addtional_field_name'])) {
			$settings['additional_fields'][$_POST[$prefix.'_addtional_field_name']]	= $_POST[$prefix.'_addtional_field_value'];
		}//end if

		if (isset($_POST[$prefix.'_addtional_field_value_edit']) && !empty($_POST[$prefix.'_addtional_field_value_edit'])) {
			$editing_list	= $_POST[$prefix.'_addtional_field_value_edit'];
			foreach ($editing_list as $field_name	=> $field_value) {
				if (isset($settings['additional_fields'][$field_name]) && $settings['additional_fields'][$field_name] != $field_value) {
					$settings['additional_fields'][$field_name]	= $field_value;
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
						?><span class="sq-backend-warning">No connection details specified.</span><?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Create Record Type</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['record_type'])) {
						echo ellipsisize(preg_replace('/\\n/', ' ', htmlspecialchars(ucwords($settings['record_type']))), 512);
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
		if ($settings['session_creator_username']) {
			if (!empty($GLOBALS['SQ_SYSTEM']->user)) {
				$username	= $GLOBALS['SQ_SYSTEM']->user->attr('username');
			}//end if
		} else if (!empty($settings['creator_username'])) {
			$username = $settings['creator_username'];
		}//end if

		$new_record_info['additional_fields']	= $additional_fields;

		$connection	= Array();
		if (!empty($settings['connector_assetid'])) {
			$trim_connector		= $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
			$connection['wsdl']				= $trim_connector->attr('wsdl');
			$connection['authentication']	= Array (
												'login'		=> $trim_connector->attr('username'),
												'password'	=> $trim_connector->attr('password'),
												'trace'		=> TRUE,
												'exceptions'=> TRUE,
											  );
			// add proxy 
			$proxy = $trim_connector->getProxy();
			$connection['authentication'] = array_merge($connection['authentication'], $proxy);
		} else {
			$connection['wsdl']				= $settings['wsdl'];
			$connection['authentication']	= Array (
												'login'		=> $settings['username'],
												'password'	=> $settings['password'],
												'trace'		=> TRUE,
												'exceptions'=> TRUE,
											  );
		}//end if

		$new_record_uri	= TRIM_Common::executeCreateRequest($connection, $username, $new_record_info);

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

		if (empty($settings['connector_assetid']) && empty($settings['wsdl']) && empty($settings['username'])) {
			return FALSE;
		}//end if

		if (!$settings['session_creator_username'] && empty($settings['creator_username'])) {
			return FALSE;
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
			$settings['record_type'] = 'document';
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
						'record_title', 'parent_record_number', 'security_level',
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
