<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: apache_config.inc,v 1.1 2006/04/21 03:40:38 bcaldwell Exp $
*
*/

require_once SQ_LIB_PATH.'/config/config.inc';

/**
* Apache_Config
*
* Purpose
*
*    Manage any Apache related configuration
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Apache_Config extends Config
{


	/**
	*/
	var $config_vars = Array(
						'SQ_APACHE_REQ_NOTE_USER_ID'	=> Array('editable'	=> 1, 'default'	=> ''),
						'SQ_APACHE_REQ_NOTE_USER_NAME'	=> Array('editable'	=> 1, 'default'	=> ''),
					   );


	/**
	* Constructor
	*
	*/
	function Apache_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/apache.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean 	$send_message		whether to send a message using the messaging system
	*
	* @return boolean	indicates whether the file was written
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

		return parent::save($vars, $backup_existing, $send_message);

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
		if (!parent::writeAccess()) return false;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
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
	* @param object Backend_Outputter	&$o		reference to the backend outputter
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

		$request_notes = Array();
		foreach ($this->config_vars as $var_name => $var_info) {
			// ignore non-editable vars
			if (!$var_info['editable']) continue;

			// we expect the following prefix
			if (strpos($var_name, 'SQ_APACHE_REQ_NOTE') === 0) {
				$request_notes[] = $var_name;
			}
		}//end foreach

		$configured_notes = Array();
		foreach ($request_notes as $note_name) {
			// must have value set
			$constant_value = constant($note_name);
			if (empty($constant_value)) continue;

			// this had definetly been configured
			$configured_notes[] = $note_name;
		}//end foreach

		// we can now calculate what hasn't been configured
		$unconfigured_notes = array_diff($request_notes, $configured_notes);

		// build select box based on unconfigured notes
		$note_options = Array('SQ_APACHE_REQ_NOTE_NONE' => '');
		foreach ($unconfigured_notes as $note_name) {
			$note_options[$note_name] = translate(strtolower($note_name));
		}//end foreach

		?><script langauge="javascript">
			function processVariableChange(variableSelector)
			{
				valueInputBox = document.getElementById('SQ_APACHE_REQ_NOTE_NEW');
				if (variableSelector.value == 'SQ_APACHE_REQ_NOTE_NONE') {
					valueInputBox.name = '';
				} else {
					valueInputBox.name = '<?php echo $class; ?>[' + variableSelector.value + ']';
				}
			}
		</script><?php

		if (!$write_access && empty($configured_notes)) return;

		$o->openSection(translate('apache_request_notes'));

			if (!empty($configured_notes)) {
				$o->openField('');

					?><table class="sq-backend-table">
						<tr>
							<th><?php echo translate('value'); ?></th>
							<th><?php echo translate('apache_note_name'); ?></th>
							<th><?php echo translate('apache_example_usage'); ?></th>
							<?php
								if ($write_access) {
									?><th style="text-align: center;"><?php echo translate('remove_question'); ?></th><?php
								}
							?>
						</tr>
					<?php

					foreach ($configured_notes as $note_name) {
						?><tr>
							<td><?php echo translate(strtolower($note_name)); ?></td>
							<td>
							<?php
								if ($write_access) {
									text_box($class.'['.$note_name.']', constant($note_name));
								} else {
									echo constant($note_name);
								}
							?>
							</td>
							<td>
							<?php
								echo '%{'.constant($note_name).'}n';
							?>
							</td>
							<?php
								if ($write_access) {
									?><td style="text-align: center;"><?php echo check_box('remove['.$note_name.']'); ?></td><?php
								}
							?>
						</tr><?php
					}

					?></table><?php

				$o->closeField();
			}

			if ($write_access) {
				$o->openField(translate('new_question'));

					combo_box('', $note_options, FALSE, 'SQ_APACHE_REQ_NOTE_NONE', 0, 'onchange="processVariableChange(this);"');
					echo '&nbsp;';
					text_box('', '', '', '', FALSE, 'id="SQ_APACHE_REQ_NOTE_NEW"');

				$o->closeField();
			}
			$o->sectionNote(translate('apache_note_example'));

		$o->closeSection();
		if ($write_access) $o->commitButton('', true);

	}//end paintBackend();


	/**
	* Saves the config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		$class = get_class($this);

		// wipe the values of any checked entries
		if (isset($_POST['remove']) && !empty($_POST['remove'])) {
			$remove_notes = array_keys($_POST['remove']);
			foreach ($remove_notes as $note_name) {
				$_POST[$class][$note_name] = '';
			}//end foreach
		}

		// idiot proofing
		if (isset($_POST[$class]) && !empty($_POST[$class])) {
			$duplicate_notes = array_keys(array_diff_assoc($_POST[$class], array_unique($_POST[$class])));
			foreach($duplicate_notes as $note_name) {
				if (empty($_POST[$class][$note_name])) continue;

				$_POST[$class][$note_name] = '';
				trigger_localised_error('SYS0314', E_USER_NOTICE, translate(strtolower($note_name)));
			}
		}
		return parent::processBackend($o);

	}//end processBackend()


}//end class

?>
