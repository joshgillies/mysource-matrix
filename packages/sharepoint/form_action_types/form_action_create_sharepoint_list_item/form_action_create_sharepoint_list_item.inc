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
* $Id: form_action_create_sharepoint_list_item.inc,v 1.6 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';

/**
* Form_Action_Create_Sharepoint_List_Item
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage data
*/


class Form_Action_Create_Sharepoint_List_Item extends Form_Action
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
		$create_list  		= array_get_index($settings, 'create_list', 'None');
		$sharepoint_fields	= array_get_index($settings, 'sharepoint_fields', '');
		$sharepoint_url		= array_get_index($settings, 'url', '');
		$username			= array_get_index($settings, 'username', '');
		$password			= array_get_index($settings, 'password', '');
		$has_connection		= FALSE;

		require_once SQ_PACKAGES_PATH.'/sharepoint/lib/sharepoint_common.inc';

		$list_names	= Array (
						'None'	=> 'None'	// We want to keep this option on top
					  );
		$all_lists		= Array();
		$lists_fields	= Array();

		$wsdl_disabled	= (!empty($settings['connector_assetid'])) ? ' disabled="disabled"' : '';
		if (!empty($settings['connector_assetid'])) {
			$sharepoint_connector 	= $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
			$sharepoint_url			= $sharepoint_connector->attr('url');
			$username				= $sharepoint_connector->attr('username');
			$password				= $sharepoint_connector->attr('password');

		}//end if

		$connection	= Array();
		if (!empty($sharepoint_url) && !empty($username)) {
			$has_connection	= TRUE;
			$connection	= Array (
							'url'				=> $sharepoint_url,
							'authentication'	=> Array (
													'login'		=> $username,
													'password'	=> $password,
													'trace'		=> TRUE,
													'exception'	=> TRUE,
												  ),
						  );

			$valid = Sharepoint_Common::test_sharepoint_connection($connection);
			if ($valid) {
				$all_lists	= Sharepoint_Common::getListCollections($connection, TRUE);
				$sp_lists	= Sharepoint_Common::getListNames($connection, $all_lists);
				$list_names	+= $sp_lists;
			}//end if


			foreach ($list_names as $id	=> $name) {
				if ($id != 'None') {
					$available_fields_by_list	= Sharepoint_Common::getListAvailableFields($connection, $id);
					if (isset($available_fields_by_list['FileLeafRef'])) {
						unset($available_fields_by_list['FileLeafRef']);
					}//end
					$lists_fields[$id]			= $available_fields_by_list;
				}//end if
			}//end foreach
		}

		?>
		<script type="text/javascript">
			var description_list	= {};
			var	field_list			= {};
			var	custom_lists		= {};

			<?php
				foreach ($all_lists as $id => $list_info) {
					?>
					description_list['<?php echo $id; ?>']	= '<?php echo $list_info['Description']; ?>';
					<?php
					if ((int) $list_info['ServerTemplate'] == 100) {
					?>
						custom_lists['<?php echo $id; ?>']	= '<?php echo $list_info['Title']; ?>';
					<?php
					}//end if
				}//end foreach

				foreach ($lists_fields as $id => $avail_fields) {
					?>
					field_list['<?php echo $id; ?>']	= {};
					<?php foreach ($avail_fields as $field_name => $display_name) {
						?>
							field_list['<?php echo $id; ?>']['<?php echo $field_name; ?>']	= '<?php echo $display_name; ?>';
						<?php
						}
					?>
					<?php
				}//end foreach ?>

			function replace_description(id) {

				var desc = '';
				if (id == 'None') {
					desc = '';
				} else {
					desc = 'Description: '+description_list[id];
				}//end else

				document.getElementById('list_description').innerHTML	= desc;

			}//end replace_description()


			function cleanTable(table)
			{
				if ( table.hasChildNodes() ) {
				    while (table.childNodes.length > 0) {
				        table.removeChild( table.firstChild );
				    }//end while
				}//end if
			}//end cleanTable

			function updateListAvailableFields(id) {

				var table	= document.getElementById('field_input_table');
				cleanTable(table);

				new_tbody	= document.createElement("tbody");
				if (id != 'None') {
					var list_fields = null;

					list_fields = field_list[id];

					for (var field_name in list_fields) {

						new_tr		= document.createElement("tr");
						new_td_1	= document.createElement("td");

						var new_text		= document.createTextNode(list_fields[field_name]);
						new_td_1.className	= 'sq-backend-table-cell';
						new_td_1.style.width	= '180px';
						new_td_1.style.padding	= '8 0 0 15px';
						new_td_1.appendChild(new_text);

						new_tr.appendChild(new_td_1);

						new_td_2	= document.createElement('td');
						new_td_2.className	= "sq-backend-table-cell";
						new_td_2.style.width	= '180px';
						new_td_2.style.padding	= '5 0 0 15px';

						new_td_2.innerHTML = '<input type="text" id="'+field_name+'" name="sharepoint_fields['+field_name+']" style="width:200px;" />';

						new_tr.appendChild(new_td_2);
						new_tbody.appendChild(new_tr);
					}//end for

				} else {
					new_tr		= document.createElement("tr");
					new_td_1	= document.createElement("td");

					new_td_1.innerHTML	= '<strong>No List Selected</strong>';
					new_tr.appendChild(new_td_1);
					new_tbody.appendChild(new_tr);
				}//end else

				table.appendChild(new_tbody);

			}//end updateListAvailableFields()

			function paintAttachmentMapping(id, attachment_question)
			{
				var table	= document.getElementById('attachment_input_table');
				cleanTable(table);

				new_tbody	= document.createElement("tbody");
				if (custom_lists[id] != null) {
						new_tr		= document.createElement("tr");
						new_td_1	= document.createElement("td");

						var new_text		= document.createTextNode('Attachment Question Asset ID');
						new_td_1.className	= 'sq-backend-table-cell';
						new_td_1.style.width	= '180px';
						new_td_1.style.padding	= '8 0 0 15px';
						new_td_1.appendChild(new_text);

						new_tr.appendChild(new_td_1);

						new_td_2	= document.createElement('td');
						new_td_2.className	= "sq-backend-table-cell";
						new_td_2.style.width	= '180px';
						new_td_2.style.padding	= '5 0 0 15px';

						new_td_2.innerHTML = '<input type="text" id="sp_attachment" name="sp_attachment" value="'+attachment_question+'" style="width:200px;" />';

						new_tr.appendChild(new_td_2);
						new_tbody.appendChild(new_tr);

				} else {
					new_tr		= document.createElement("tr");
					new_td_1	= document.createElement("td");

					new_td_1.innerHTML	= '<strong>File attachment not available.</strong>';
					new_tr.appendChild(new_td_1);
					new_tbody.appendChild(new_tr);
				}//end else

				table.appendChild(new_tbody);
			}

		</script>

		<p class="sq-backend-section-subheading">Connection Details</p>

		<div id="<?php echo $prefix ?>_connector_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td width="100"><p>System URL</p></td>
					<td><p><?php text_box($prefix.'_url', array_get_index($settings, 'url', ''), 120, '', FALSE, $wsdl_disabled); ?></p></td>
				</tr>
				<tr>
					<td><?php echo translate('Username'); ?></td>

					<td><p><?php text_box($prefix.'_username', array_get_index($settings, 'username', ''), 35, '', FALSE, $wsdl_disabled); ?><br />
					</p></td>
				</tr>
				<tr>
					<td><?php echo translate('Password'); ?></td>

					<td><p><?php password_box($prefix.'_password', array_get_index($settings, 'password', ''), 35, '', $wsdl_disabled); ?></p></td>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_connector_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" style="width:auto">
				<tr>
					<td><p>Sharepoint Connector Asset</p></td>
					<td><p><?php asset_finder($prefix.'_connector', $settings['connector_assetid'], Array('sharepoint_connector'=>'D')); ?><br/>
					Note: If a Sharepoint connector is selected, the settings in the 'Connection Setup' section (above) have no effect. <br />Sharepoint Connector allows the Sharepoint connection settings to be reused on other Sharepoint assets..</p></td>
				</tr>
			</table>
		</div>

		<p class="sq-backend-section-subheading">Create Record Settings</p>

		<div id="<?php echo $prefix ?>_record_type_selection" style="padding: 0.5em 0px 2em 30px;">
			<table style="width:auto;padding-left: 30px;">
				<tr>
					<td valign="top" width="75"><strong><?php echo 'Select List&nbsp;&nbsp;&nbsp;&nbsp;';?></strong><br /><br /></td>
					<td>
						<?php
							combo_box($prefix.'_create_list', $list_names, FALSE, $create_list, 0, ' onChange="replace_description(this.value);paintAttachmentMapping(this.value, \'\');updateListAvailableFields(this.value);document.getElementById(\'selected_list\').value=(this.options[this.selectedIndex].text);"');
							hidden_field('selected_list', '');
						?>
						<br /><br />
						<span id="list_description"></span>
					</td>
				</tr>

				<tr>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">

			<p class="sq-backend-section-subheading">Available Fields</p>

			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table id="field_input_table" name="field_input_table" style="width:auto;padding-left: 30px;">
					<tr>
						<td style="width:180px;"><strong>No List Selected</strong></td>
					</tr>
				</table>
			</div>

			<p class="sq-backend-section-subheading">Attachment</p>
			<p class="sq-backend-smallprint" style="padding-left:60px;">Only entry created in a Sharepoint custom list can have file attached.</p>

			<div id="<?php echo $prefix ?>_mandatory_fields" style="padding: 0.5em 0px 2em 30px;">
				<table id="attachment_input_table" name="attachment_input_table" style="width:auto;padding-left: 30px;">
					<tr>
						<td style="width:180px;"><strong>File attachment not available</strong></td>
					</tr>
				</table>
			</div>

		</div>

		<?php if ($create_list != 'None' && $has_connection) { ?>
		<script type="text/javascript">
			replace_description('<?php echo $create_list; ?>');
			updateListAvailableFields('<?php echo $create_list; ?>');
			paintAttachmentMapping('<?php echo $create_list; ?>', '<?php echo array_get_index($settings, 'attachment_assetid', ''); ?>')
		<?php

			if (!empty($sharepoint_fields)) {
				foreach ($sharepoint_fields as $field_id => $value) {
					?>
					var text_box	= document.getElementById('<?php echo $field_id; ?>');
					if (text_box != null) {
						text_box.value	= '<?php echo $value; ?>';
					}//end if
					<?php
				}//end foreach
			}//end if
		?>
		</script>
		<?php }

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
		$new_create_list				= $_POST[$prefix.'_create_list'];
		$create_list_name				= $_POST['selected_list'];
		$settings['sharepoint_fields']	= Array();
		$settings['attachment_assetid']	= array_get_index($_POST, 'sp_attachment', '');

		if (isset($settings['_create_list'])) unset($settings['_create_list']);

		if (!empty($create_list_name)) {
			$settings['create_list_name']	= $create_list_name;
		}//end if

		if (!(isset($settings['create_list'])) || $new_create_list != $settings['create_list']) {
			$settings['create_list']		= $new_create_list;
			$settings['sharepoint_fields']	= Array();
		}//end if

		if (isset($_POST['sharepoint_fields'])) {
			foreach ($_POST['sharepoint_fields'] as $field_id => $value) {
				$settings['sharepoint_fields'][$field_id]	= $value;
			}//end foreach
		}//end if

		$new_url						= array_get_index($_POST, $prefix.'_url', NULL);
		$new_username					= array_get_index($_POST, $prefix.'_username', NULL);
		$new_password					= array_get_index($_POST, $prefix.'_password', NULL);
		$creator_username				= array_get_index($_POST, $prefix.'_creator_username', NULL);
		$session_creator_username		= array_get_index($_POST, $prefix.'_session_creator_username', FALSE);

		if (!is_null($new_url)) {
			$settings['url']		= $new_url;
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
					} else if (!empty($settings['url']) && !empty($settings['username'])) {
						?><span class="""><?php echo $settings['url']; ?></span><?php
					} else {
						?><span class="sq-backend-warning">No connection details specified.</span><?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Create In List</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['create_list']) || !empty($settings['create_list_name'])) {
						$list_name	= (!empty($settings['create_list_name']) && $settings['create_list'] != 'None') ? $settings['create_list_name'] : $settings['create_list'];
						echo ellipsisize(preg_replace('/\\n/', ' ', htmlspecialchars($list_name)), 512);
					} else {
						?><span class="sq-backend-warning">No list has been selected.</span><?php
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

		$create_list  		= array_get_index($settings, 'create_list', 'None');
		$sharepoint_fields	= array_get_index($settings, 'sharepoint_fields', '');

		foreach ($sharepoint_fields as $field_name => $field_content) {
			self::getKeywordReplacementsForForm($field_name, $field_content, $sharepoint_fields, $form);
		}//end foreach

		$form_sub_id_array	= Array();
		if (!empty($settings['attachment_assetid'])) {
			$form_sub_id_array	= Array (
									'submission_id'	=> '%form_submission_id%',
								  );
			self::getKeywordReplacementsForForm(key($form_sub_id_array), $form_sub_id_array[key($form_sub_id_array)], $form_sub_id_array, $form);
			$submission_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($form_sub_id_array['submission_id']);
			$sub_asset_attr		= $submission_asset->attr('attributes');
		}//end if


		// Handling file upload
		$base64_content		= NULL;
		if (!empty($sub_asset_attr) && isset($sub_asset_attr['answers'][$settings['attachment_assetid']])) {
			$extra_data	= array_get_index($sub_asset_attr['answers'][$settings['attachment_assetid']], 'extra_data', Array());
			$uploaded_file_path = NULL;
			if (isset($extra_data['new_file_assetid'])) {
				$uploaded_file_assetid	= $sub_asset_attr['answers'][$settings['attachment_assetid']]['extra_data']['new_file_assetid'];
				$file_asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($uploaded_file_assetid);
				$file_info   = $file_asset->getExistingFile();
				if (!empty($file_info) && isset($file_info['path'])) {
					$uploaded_file_path	= $file_info['path'];
				}//end if
			} else if (isset($extra_data['filesystem_path']) && !empty($extra_data['filesystem_path'])) {
				$uploaded_file_path	= $extra_data['filesystem_path'];
				if(strpos($uploaded_file_path, '/') !== 0) $uploaded_file_path = SQ_SYSTEM_ROOT.'/'.$uploaded_file_path;
			}//end else

			// I dont know why we dont have to do base64_encode, if we do, the content in Sharepoint will be messed up.
			// So we leave the content untouched.
			$base64_content	= file_get_contents($uploaded_file_path);

		}//end if

		// Construct the connection
		$username	= '';
		if ($settings['session_creator_username']) {
			if (!empty($GLOBALS['SQ_SYSTEM']->user)) {
				$username	= $GLOBALS['SQ_SYSTEM']->user->attr('username');
			}//end if
		} else if (!empty($settings['creator_username'])) {
			$username = $settings['creator_username'];
		}//end if

		$connection	= Array();
		if (!empty($settings['connector_assetid'])) {
			$trim_connector		= $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['connector_assetid']);
			$connection['url']				= $trim_connector->attr('url');
			$connection['authentication']	= Array (
												'login'		=> $trim_connector->attr('username'),
												'password'	=> $trim_connector->attr('password'),
												'trace'		=> TRUE,
												'exception'	=> TRUE,
											  );
		} else {
			$connection['url']				= $settings['url'];
			$connection['authentication']	= Array (
												'login'		=> $settings['username'],
												'password'	=> $settings['password'],
												'trace'		=> TRUE,
												'exception'	=> TRUE,
											  );
		}//end if

		$ows_ID	= Sharepoint_Common::addListItem($connection, $settings['create_list'], $sharepoint_fields);
		if (!is_int($ows_ID)) {
			trigger_error($ows_ID, E_USER_WARNING);
		} else {
			// We are ready for attachment
			if (!empty($base64_content)) {
				$attach_resutt	= Sharepoint_Common::addAttachment($connection, $settings['create_list'], $ows_ID, $uploaded_file_path, $base64_content);
			}//end if
		}//end else

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

		// Must selected a record type to create
		if (empty($settings['create_list'])) {
			return FALSE;
		}//end if


		// Must selected a record type to create
		if (empty($settings['sharepoint_fields'])) {
			return FALSE;
		}//end if

		if (empty($settings['connector_assetid']) && empty($settings['url']) && empty($settings['username'])) {
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
		if (!isset($settings['create_list'])) {
			$settings['create_list'] = 'None';
		}//end if

		if (!isset($settings['connector_assetid'])) {
			$settings['connector_assetid'] = 0;
		}//end if

	}//end _fillDefaults()


}//end class

?>
