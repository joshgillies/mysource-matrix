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
* $Id: config.inc,v 1.28 2006/06/30 05:58:16 tbarrett Exp $
*
*/


/**
* Config
*
* Purpose
*
*    Looks after the creation of the config files that are in PHP code
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.28 $
* @package MySource_Matrix
*/
class Config extends MySource_Object
{
	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array();

	/**
	* The file to save the config file to
	* @var string
	*/
	var $config_file = '';

	/**
	* The screen name of the config
	* @var string
	*/
	var $name = '';


	/**
	* Constructor
	*
	*/
	function Config()
	{
		$this->MySource_Object();
		$this->name = ucwords(str_replace('_', ' ', get_class($this)));

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @param boolean 	$send_message		whether to send a message using the messaging system
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false, $send_message=true)
	{
		if (!$this->writeAccess()) {
			trigger_localised_error('SYS0026', E_USER_WARNING, $this->name);
			return false;
		}

		// attempt to load the config file to make sure we get any current settings
		if (file_exists($this->config_file)) {
			require_once $this->config_file;
		}

		$changed_vars = Array();
		ob_start();
		foreach ($this->config_vars as $var_name => $data) {

			$current_value = (defined($var_name)) ? constant($var_name) : $data['default'];
			if ($data['editable'] && isset($vars[$var_name]) && $current_value !== $vars[$var_name]) {

				// OK to make sure that everything is fine, we are going to force the issue of setting the type
				// of the vars to that of the default
				$default_type = gettype($data['default']);

				$value = $vars[$var_name];

				$changed_vars[$var_name] = Array('old' => $current_value, 'new' => $value);

			} else {
				$value = $current_value;
			}

			if (!$this->_paintConfigVar($var_name, $value)) {
				ob_end_clean();
				return false;
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
				return false;
			}

		}// endif

		if (SQ_PHP_CLI) {
			echo "----------------------------------\n".($str)."----------------------------------\n";
		}
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory(dirname($this->config_file))) {
			return false;
		}

		// and save the file
		if (!string_to_file($str, $this->config_file)) {
			return false;
		}

		// Check if a message should be sent.
		if ($changed_vars && $send_message) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->openQueue();
			foreach ($changed_vars as $var_name => $values) {
				$message_body = $var_name.' changed from '.var_export($values['old'], true).' to '.var_export($values['new'], true)."\n";
				$config_name = ucwords(str_replace('_', ' ', get_class($this)));
				$msg_reps = Array(
								'setting'	=> $var_name,
								'old_value'	=> var_export($values['old'], true),
								'new_value'	=> var_export($values['new'], true),
							);
				$message = $ms->newMessage(Array(), 'config.'.get_class($this), $msg_reps);
				$ms->enqueueMessage($message);
			}
			$ms->closeQueue();

		}// end if

		return true;

	}//end save()


	/**
	* Paints the code to define a constant
	*
	* This allows extra things to be added for special cases, and extra checks to be made if necessary on the value
	*
	* @param string		$name		the config var name
	* @param mixed		$value		the value that the config var has
	* @param boolean	$var_export	whether to run the value through var_export or not
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value, $var_export=true)
	{
		echo "define('", $var_name, "', ";
		if ($var_export) {
			var_export($value);
		} else {
			echo $value;
		}

		echo ");\n";

		return true;

	}//end _paintConfigVar()


	/**
	* Attempt to acquire the lock for this config
	*
	* @return true on success or the error message on failure
	* @access public
	* @see MySource::acquireLock()
	*/
	function acquireLock()
	{
		if (!$this->canAcquireLock()) {
			return 'You are not allowed to acquire the lock on '.$this->name;
		}
		return $GLOBALS['SQ_SYSTEM']->acquireLock(get_class($this));

	}//end acquireLock()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		return true;

	}//end canAcquireLock()


	/**
	* Attempt to release the lock for this config
	*
	* @return true on success or the error message on failure
	* @access public
	* @see MySource::releaseLock()
	*/
	function releaseLock()
	{
		return $GLOBALS['SQ_SYSTEM']->releaseLock(get_class($this));

	}//end releaseLock()


	/**
	* Returns an array of information about any lock we have
	*
	* @return Array()
	* @access public
	* @see MySource::getLockInfo()
	*/
	function getLockInfo()
	{
		return $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class($this));

	}//end getLockInfo()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!isset($GLOBALS['SQ_SYSTEM'])) return true;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
		}

		$lock_info = $this->getLockInfo();
		if (!empty($lock_info) && $lock_info['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			return true;
		}
		return false;

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		require_once $this->config_file;

		$lock = $this->getLockInfo();
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		if (!empty($lock) || $this->canAcquireLock()) {

			$o->openSection(translate('locking_/_editing'));
			$o->openRaw();
			?>
			<table border="0" cellspacing="3" cellpadding="1">
				<tr>
					<td>
						<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/'.((empty($lock)) ? 'un' : '').'locked.png', 16, 16); ?>
					</td>
					<td>
						<p class="sq-lock-message">
						<?php
						if (!empty($lock)) {
							// this asset is currently locked
							// so display message to the user
							$user = &$GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

							$now = time();

							require_once SQ_FUDGE_PATH.'/general/datetime.inc';
							$expires_in = easy_time_total(($lock['expires'] - $now), true);
							if (!$expires_in) $expires_in = '1 second';
							$expires_in = translate('due_to_expire', $expires_in);
							echo translate('release_lock', translate('release_lock_button'), translate('user_preferences')); 
							echo '<br />';
							echo $expires_in; 

						} else {
							echo translate('acquire_lock', translate('acquire_lock_button'), translate('user_preferences'));
						}
						echo '</p>';
						if (!empty($lock)) {
							if ($have_lock) {
								submit_button('sq_lock_release_manual', translate('release_lock_button'), 'set_hidden_field("process_form", "0");', 'accesskey="r"');
							}
						} else {
							submit_button('sq_lock_acquire', translate('acquire_lock_button'), 'set_hidden_field("process_form", "0");', 'accesskey="a"');
						}
						?>
					</td>
				</tr>
			</table>
			<?php
			$o->closeRaw();
			$o->closeSection();

		}//end if (!empty($lock) || $this->canAcquireLock())

	}//end paintBackend()


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
		$saved = false;
		if (!empty($_POST['process_form'])) {

			if (!empty($_POST[get_class($this)])) {
				$saved = $this->save($_POST[get_class($this)]);
			}
		}

		if (!empty($_POST['sq_lock_release']) || !empty($_POST['sq_lock_release_manual'])) {
			$this->releaseLock();
		}

		// if there is no lock currently and we want it
		if (!empty($_POST['sq_lock_acquire'])) {
			$this->acquireLock();
		}

		// if the save was successful, we need to redirect because the constants
		// that are already defined cannot be redefined in this script execution
		if ($saved) $o->setRedirect($o->getCurrentLocation());

		return $saved;

	}//end processBackend()


}//end class

?>
