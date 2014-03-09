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
* $Id: apache_config.inc,v 1.8 2012/08/30 01:09:19 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/config/config.inc';

define('SQ_APACHE_CONF_CUSTOM_KEYWORD_KEY', 'sq_custom_keyword_key');

/**
* Apache_Config
*
* Purpose
*
*    Manage any Apache related configuration
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/
class Apache_Config extends Config
{


	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	*
	* @var Array()
	* @access private
	*/
	var $config_vars = Array(
						'request_notes'	=> Array('editable' => 1, 'default' => Array()),
					   );


	/**
	* Configured apache request notes
	*
	* @var array
	* @access private
	*/
	var $request_notes = Array();


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/apache.inc';

		if (!file_exists($this->config_file)) return;

		// load request notes
		require_once $this->config_file;
		if (!empty($request_notes)) {
			$this->request_notes = $request_notes;
		}

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean	$send_message		whether to send a message using the messaging system
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, go away
			if (!$is_admin) {
				trigger_localised_error('SYS0267', E_USER_WARNING, $this->name);
				return FALSE;
			}
		}
		if (!$this->writeAccess()) {
				trigger_localised_error('SYS0026', E_USER_WARNING, $this->name);
				return FALSE;
			}
			// attempt to load the config file to make sure we get any current settings
			if (file_exists($this->config_file)) {
				require $this->config_file;
			}

			$changed_vars = Array();
			ob_start();
			foreach ($this->config_vars as $var_name => $data) {

				$current_value = (isset($$var_name)) ? $$var_name : $data['default'];
				if ($data['editable'] && isset($vars[$var_name]) && $current_value !== $vars[$var_name]) {

					$value = $vars[$var_name];

					$changed_vars[$var_name] = Array('old' => $current_value, 'new' => $value);
				} else {
					$value = $current_value;
				}

				if (!$this->_paintConfigVar($var_name, $value)) {
					ob_end_clean();
					return FALSE;
				}

			}//end foreach

			$str = '<'."?php\n".ob_get_contents().'?'.">\n";

			ob_end_clean();
			if ($backup_existing && file_exists($this->config_file)) {

				$i = 0;
				do {
					$i++;
					$old_version = $this->config_file.'.'.$i;
				} while (file_exists($old_version));

				if (!copy($this->config_file, $old_version)) {
					return FALSE;
				}

			}// endif

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			// make sure the directory exists
			if (!create_directory(dirname($this->config_file))) {
				return FALSE;
			}

			// and save the file
			if (!string_to_file($str, $this->config_file)) {
				return FALSE;
			}

			// Check if a message should be sent.
			if ($changed_vars && $send_message) {
				$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
				$ms->openQueue();
				foreach ($changed_vars as $var_name => $values) {
					$message_body = $var_name.' changed from '.var_export($values['old'], TRUE).' to '.var_export($values['new'], TRUE)."\n";
					$config_name = ucwords(str_replace('_', ' ', get_class($this)));
					$msg_reps = Array(
									'setting'	=> $var_name,
									'old_value'	=> var_export($values['old'], TRUE),
									'new_value'	=> var_export($values['new'], TRUE),
								);
					$message = $ms->newMessage(Array(), 'config.'.get_class($this), $msg_reps);
					$ms->enqueueMessage($message);
				}
				$ms->closeQueue();

			}// end if

		return TRUE;


	}//end save()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end canAcquireLock()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!parent::writeAccess()) return FALSE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$class = get_class($this);

		$o->openSection(translate('apache_request_notes'));

			if (!empty($this->request_notes)) {
				$o->openField('');

					?><table class="sq-backend-table">
						<tr>
							<th><?php echo translate('apache_note_name'); ?></th>
							<th><?php echo translate('source'); ?></th>
							<th><?php echo translate('value'); ?></th>
							<th><?php echo translate('apache_example_usage'); ?></th>
							<?php
								if ($write_access) {
									?><th style="text-align: center;"><?php echo translate('remove_question'); ?></th><?php
								}
							?>
						</tr>
					<?php
					foreach (array_reverse($this->request_notes) as $note_name => $note_info) {
						?><tr>
							<td><?php echo $note_name; ?></td>
							<td><?php echo $note_info['type']; ?></td>
							<td> <?php echo $note_info['keyword']; ?></td>
							<td>
							<?php
								echo '%{'.$note_name.'}n';
							?>
							</td>
							<?php
								if ($write_access) {
									?><td style="text-align: center;"><?php echo check_box('remove['.base64_encode($note_name).']'); ?></td><?php
								}
							?>
						</tr><?php
					}
					?></table><?php

				$o->closeField();
			}//end if no request notes

			if ($write_access) {
				$o->openField(translate('new_question'));

					$elements = Array();

					ob_start();
						$keyword_option = Array(
											'sq_keyword_select_none'	=> '--',
											'sq_keyword_select_user'	=> translate('apache_asset_type_user'),
											'sq_keyword_select_asset'	=> translate('apache_asset_type_asset'),
										  );
						// select what asset to list keywords for
						combo_box('sq_keyword_type_option', $keyword_option, FALSE, 'none', 0, 'id="sq_asset_type_select" onchange="sqKeywordSelectUpdate(this)"');
						echo '<br />';
					$asset_type_selector = ob_get_clean();

					?>
					<script language="javascript">
						function sqKeywordSelectUpdate(typeSelector)
						{
							/* hide all select input */
							for (optionNum = 0; optionNum < typeSelector.options.length; optionNum ++) {
								optionValue = typeSelector.options[optionNum].value;
								if (optionValue != 'sq_keyword_select_none') {
									document.getElementById(optionValue).style.display = 'none';
								}
							}

							/* display selected input */
							if (typeSelector.value != 'sq_keyword_select_none') {
								document.getElementById(typeSelector.value).style.display = '';

								document.getElementById(typeSelector.value).parentNode.parentNode.style.display = '';
								document.getElementById('sq_request_note_new').parentNode.parentNode.style.display = '';
							} else {
								document.getElementById(typeSelector.value).parentNode.parentNode.style.display = 'none';
								document.getElementById('sq_request_note_new').parentNode.parentNode.style.display = 'none';
							}

							sqKeywordValueSelectUpdate(document.getElementById(typeSelector.value));
						}

						function sqKeywordValueSelectUpdate(keywordSelector)
						{
							/* display or hide the custom keyword input box*/
							document.getElementById('sq_custom_keyword').parentNode.parentNode.style.display = (keywordSelector.value == 'sq_custom_keyword_key') ? '' : 'none';

						}
					</script>
					<?php

					// standard and user asset keywords
					foreach (Array('asset', 'user') as $asset_type) {
						ob_start();
							$asset = new $asset_type();
							$options = $asset->getAvailableKeywords();
							$options[SQ_APACHE_CONF_CUSTOM_KEYWORD_KEY] = translate('custom_keyword');
							combo_box('sq_keyword_select_'.$asset_type, $options, FALSE, '', 0, 'id="sq_keyword_select_'.$asset_type.'"  onchange="sqKeywordValueSelectUpdate(this)"');
						$elements[] = ob_get_clean();
					}

					ob_start();
						hidden_field('sq_keyword_select_none', '');
					$elements[] = ob_get_clean();

					// input box for the custom keyword
					ob_start();
						text_box('sq_custom_keyword', '', '', '', FALSE, 'id="sq_custom_keyword"');
					$custom_keyword_input = ob_get_clean();

					// input box for new request notes
					ob_start();
						text_box('sq_request_note_new', '', '', '', FALSE, 'id="sq_request_note_new"');
					$note_name_input = ob_get_clean();

					?>
					<table class="sq-backend-table">
						<tr>
							<th><?php echo translate('source'); ?></th>
							<td><?php echo $asset_type_selector; ?></td>
						</tr>
						<tr>
							<th><?php echo translate('value'); ?></th>
							<td><?php echo implode(' ', $elements); ?></td>
						</tr>
						<tr>
							<th><?php echo translate('keyword_name'); ?></th>
							<td><?php echo $custom_keyword_input; ?></td>
						</tr>
						<tr>
							<th><?php echo translate('apache_note_name'); ?></th>
							<td><?php echo $note_name_input; ?></td>
						</tr>
					</table>
					<script language="javascript">
						sqKeywordSelectUpdate(document.getElementById('sq_asset_type_select'));
					</script>
					<?php

				$o->closeField();
			}//end if write access
			$o->sectionNote(translate('apache_note_example'));

		$o->closeSection();
		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object	&$o	reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		$class = get_class_lower($this);

		// wipe the values of any checked entries
		if (isset($_POST['remove']) && !empty($_POST['remove'])) {
			$remove_notes = array_keys($_POST['remove']);
			foreach ($remove_notes as $note_name) {
				unset($this->request_notes[base64_decode($note_name)]);
			}//end foreach
		}

		// process any new request note
		if ($new_request_note = $this->_getNewRequestNote($_POST)) {
			$this->request_notes = array_merge($new_request_note, $this->request_notes);
		}
		$_POST[$class]['request_notes'] = $this->request_notes;

		return parent::processBackend($o);

	}//end processBackend()


	/**
	* Paints the code to define a constant
	*
	* This allows extra things to be added for special cases, and extra checks to be made if necessary on the value
	*
	* @param string		$var_name	the config var name
	* @param mixed		$value		the value that the config var has
	* @param boolean	$var_export	whether to run the value through var_export or not
	*
	* @return boolean
	* @access private
	*/
	function _paintConfigVar($var_name, $value, $var_export=TRUE)
	{
		// we're not going to save this as a constant, since it's an array
		if ($var_name == 'request_notes') {
			echo '$request_notes = '.var_export($value, 1).';'."\n";
		} else {
			parent::_paintConfigVar($var_name, $value, $var_export);
		}
		return TRUE;

	}//end _paintConfigVar()


	/**
	* Given an array, attempt to build information for a new request note
	*
	* @param array	$data	Array to pull request note data from, probably post
	*
	* @return mixed boolean|array
	* @access private
	*/
	function _getNewRequestNote($data)
	{
		// make sure we're really going to get a new request note
		if (!isset($data['sq_keyword_type_option'])) {
			return FALSE;
		}
		if ($data['sq_keyword_type_option'] == 'sq_keyword_select_none') {
			return FALSE;
		}

		$new_req_note = Array();

		// collect all the info we can
		$req_note_name = $data['sq_request_note_new'];
		$new_req_note['type'] = $data['sq_keyword_type_option'];
		$new_req_note['keyword'] = $data[$new_req_note['type']];
		if ($new_req_note['keyword'] == SQ_APACHE_CONF_CUSTOM_KEYWORD_KEY) {
			$new_req_note['keyword'] = trim(strtolower($data['sq_custom_keyword']));
		}

		// make our type an asset type rather
		$new_req_note['type'] = str_replace('sq_keyword_select_', '', $new_req_note['type']);

		// need to have a name to create a new request note
		if (empty($req_note_name)) return FALSE;

		// need to have a name to create a new request note
		if (empty($new_req_note['keyword'])) return FALSE;

		// double check name
		if (in_array($req_note_name, array_keys($this->request_notes))) {
			trigger_localised_error('SYS0314', E_USER_NOTICE);
			return Array();
		}

		return Array($req_note_name => $new_req_note);

	}//end _getNewRequestNote()


}//end class

?>
