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
	* TRIM library class name
	*
	* @var string
	*/
	protected static $_lib_class = 'TRIM_Common';


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Call the specified library function using the appropriate lib class
	* All the library function used should be called via this method
	*
	* @param string $function
	* @param array  $arguments
	*
	* @return mixed
	* @access public
	*/
	public static function lib($function, $arguments=Array())
	{
		return call_user_func_array(Array(static::$_lib_class, $function), $arguments);

	}//end lib()


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

		static::_paintConnectionDetails($prefix, $settings);
		?>

		<div id="<?php echo $prefix ?>_record_type_selection" style="padding: 5px;">
			<p class="sq-backend-section-subheading"><?php echo translate('Create Record Settings'); ?></p>
			<table class="sq-backend-table">
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<tr>
					<th><div style="min-width:200px"><?php echo translate('Record Type');?></div></th>
					<td>
						<?php
							$options	= Array (
											'document'	=> 'Document',
										  );
							combo_box($prefix.'_record_type', $options, FALSE, $record_type);
						?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Create As User');?></th>
					<td>
						<?php
							text_box($prefix.'_creator_username', array_get_index($settings,'creator_username', ''), 35, '', FALSE, ((boolean) array_get_index($settings, 'session_creator_username', FALSE)) ? ' disabled=true' :'');
							?>
							<?php
							check_box($prefix.'_session_creator_username', '1', (boolean) array_get_index($settings, 'session_creator_username', FALSE), '
							if (checked ==1) {
								document.getElementById(\''.$prefix.'_creator_username\').disabled = true;
							} else {
								document.getElementById(\''.$prefix.'_creator_username\').disabled = false;
							}
								', ' style="padding-top:2px;"');

							label(translate('Use current logged in Matrix User.'), $prefix.'_session_creator_username');
						?>
					</td>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 5px;">
			<p class="sq-backend-section-subheading"><?php echo translate('Mandatory Fields Mapping'); ?></p>
			<table class="sq-backend-table">
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<?php
					$required_fields = self::getRecordTypeFields($record_type);
					$security_levels = self::lib('getSecurityLevel');
					foreach ($required_fields as $field_name) {
					?>
					<tr>
						<th><div style="width:200px"><?php echo ucwords(str_replace('_', ' ', $field_name)); ?></div></th>
						<td>
						<?php
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

		<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 5px;">
			<p class="sq-backend-section-subheading"><?php echo translate('Additional Fields Mapping'); ?></p>
			<?php
			if (!empty($additional_fields)) {
				$record_properties	= self::lib('getRecordProperties');
				?>
				<table class="sq-backend-table">
				<colgroup>
					<col width="20%" />
					<col width="70%" />
					<col width="10%" />
				</colgroup>
					<tr>
						<th><div style='width:200px'><?php echo translate('Field Name') ?></div></th>

						<th><?php echo translate('Field Value') ?></th>

						<th><?php echo translate('Delete?'); ?></th>

					</tr>
					<?php
						foreach ($additional_fields as $field_name => $field_value) {
						?>
						<tr>
							<td><?php echo $record_properties[$field_name]; ?></td>
							<td><?php text_area($prefix.'_addtional_field_value_edit['.$field_name.']', $field_value, 100, 3); ?></td>
							<td  class="sq-backend-table-cell"><?php check_box($prefix.'_addtional_field_remove[]', $field_name, FALSE); ?></td>
						</tr>
						<?php
						}//end foreach
					?>
				</table>
				<?php
			}//end if
			?>
		
		<table class="sq-backend-table">
				<tr>
					<th><?php echo translate('Add new additional field');?></th>
				</tr>
				<tr>
					<td><?php
						$record_properties['']	= 'None';
						$record_properties	= array_merge($record_properties, self::lib('getRecordProperties'));
						ksort($record_properties);
						combo_box($prefix.'_addtional_field_name', $record_properties, FALSE, 'None');
					?></td>
				</tr>
			</table>
		</div>
		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @return boolean
	* @access public
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		static::_processConnectionDetails($prefix, $settings);

		$settings['record_type']		= $_POST[$prefix.'_record_type'];
		$required_fields				= self::getRecordTypeFields($settings['record_type']);
		$creator_username				= array_get_index($_POST, $prefix.'_creator_username', NULL);
		$session_creator_username		= array_get_index($_POST, $prefix.'_session_creator_username', FALSE);

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
			$settings['additional_fields'][$_POST[$prefix.'_addtional_field_name']]	= array_get_index($_POST, $prefix.'_addtional_field_value', '');
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
	* Paint the connection detals forms
	*
	* @param string $prefix
	* @param array	$settings TRIM form action settings
	*
	* @return void
	* @access protected
	*/
	protected static function _paintConnectionDetails($prefix, $settings)
	{
		?>
		<div id="<?php echo $prefix ?>_connector_div" style="padding: 5px;">
			<p class="sq-backend-section-subheading"><?php echo translate('Connection Details'); ?></p>
			<table class="sq-backend-table">
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<tr>
					<th><div style="width:200px"><?php echo translate('WSDL'); ?></div></th>
					<td><?php text_box($prefix.'_wsdl', array_get_index($settings, 'wsdl', ''), 120,'',FALSE, ' style="width: 100%;box-sizing: border-box;height: 24px;"'); ?> </td>
				</tr>
				<tr>
					<th><?php echo translate('Username'); ?></th>
					<td><?php text_box($prefix.'_username', array_get_index($settings, 'username', ''), 35); ?></td>
				</tr>
				<tr>
					<th><?php echo translate('Password'); ?></th>
					<td><?php password_box($prefix.'_password', '', 35); ?></td>
				</tr>
				<tr>
					<th><?php echo translate('Authentication Method'); ?></th>
					<td><?php combo_box($prefix.'_method', Array('HTTP_BASIC' => 'HTTP Basic', 'NTLM' => 'NTLM'), FALSE, array_get_index($settings, 'method', 'HTTP_BASIC')); ?></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('NOTE: Proxy setting is only available when using a connector asset.'); ?></td>
				</tr>
			</table>

			<table class="sq-backend-table">
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				<tr>
					<th><div style="min-width:200px"><?php echo translate('TRIM Connector Asset'); ?></div></th>
					<td><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('trim_connector'=>'D')); ?></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('NOTE: If a TRIM connector is selected, the settings in the "Connection Setup" section above will have no effect.'); ?></td>
				</tr>
			</table>
		</div>
		<?php

	}//end paintConnectionDetails()


	/**
	* Process the connection detals forms
	*
	* @param string $prefix
	* @param array	$settings TRIM form action settings
	*
	* @return void
	* @access protected
	*/
	protected static function _processConnectionDetails($prefix, &$settings)
	{
		$settings['connector_assetid']	= $_POST[$prefix.'_connector']['assetid'];
		$new_wsdl						= array_get_index($_POST, $prefix.'_wsdl', '');
		$new_username					= array_get_index($_POST, $prefix.'_username', '');
		$new_password					= array_get_index($_POST, $prefix.'_password', '');
		$method							= array_get_index($_POST, $prefix.'_method', 'HTTP_BASIC');

		if (!is_null($new_wsdl)) {
			$settings['wsdl']		= $new_wsdl;
		}//end if
		if (!is_null($new_username)) {
			$settings['username']	= $new_username;
		}//end if
		if (!empty($new_password)) {
			$settings['password']	= $new_password;
		}//end if
		$settings['method']	= array_get_index($_POST, $prefix.'_method', 'HTTP_BASIC');

	}//end _processConnectionDetails()


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
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('Data Source'); ?>:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['connector_assetid'])) {
						echo get_asset_tag_line($settings['connector_assetid']);
					} else {
						?><span class="sq-backend-warning"><?php echo translate('Connection details not specified.'); ?></span><?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('Create Record Type');?></strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['record_type'])) {
						echo ellipsisize(preg_replace('/\\n/', ' ', htmlspecialchars(ucwords($settings['record_type']))), 512);
					} else {
						?><span class="sq-backend-warning"><?php echo translate('No record type selected.'); ?></span><?php
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
			$connection['method']			= $trim_connector->attr('authentication_type');
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
			$connection['method']			= isset($settings['method']) ? $settings['method'] : 'HTTP_BASIC';
		}//end if

		$new_record_uri	= self::lib('executeCreateRequest', Array($connection, $username, $new_record_info));

		return TRUE;

	}//end execute()


	/**
	* This function replace all the keywords for each of the field
	*
	* @return void
	* @access public
	*/
	public static function getKeywordReplacementsForForm($field_name, $field_content, &$replacements, &$form)
	{
		// Replace global keywords in the contnet
		replace_global_keywords($field_content);

		// Replace form specific keywords
		$keywords = extract_keywords($field_content);
		foreach ($keywords as $keyword) {
			$count = 0;
			do {
				$replacement = $form->_getThankYouKeywordReplacement($keyword);
				$field_content = preg_replace('/%'.preg_quote($keyword).'%/U', $replacement, $field_content, 1, $count);
			} while ($count > 0);
		}//end foreach

		$replacements[$field_name] = $field_content;

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
	private static function _fillDefaults(&$settings) 
	{
		if (!isset($settings['record_type'])) {
			$settings['record_type'] = 'document';
		}//end if

		if (!isset($settings['connector_assetid'])) {
			$settings['connector_assetid'] = 0;
		}//end if

	}//end _fillDefaults()


	protected static function getRecordTypeFields($name)
	{
		$func_name	= 'get'.$name.'TypeFields';
		$fields = static::$func_name();

		return $fields;

	}//end getRecordTypeFields()


	private static function getBoxTypeFields()
	{
		$field_list	= Array (
						'Name', 'Val'
					  );
		return $field_list;

	}//end getBoxTypeFields()

	protected static function getDocumentTypeFields()
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
