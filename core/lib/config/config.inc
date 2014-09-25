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
* $Id: config.inc,v 1.37 2013/07/25 23:23:50 lwright Exp $
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
* @version $Revision: 1.37 $
* @package MySource_Matrix
*/
class Config extends MySource_Object
{


	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	public $config_vars = Array();

	/**
	* The file to save the config file to
	* @var string
	*/
	public $config_file = '';

	/**
	* The screen name of the config
	* @var string
	*/
	public $name = '';


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
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
	public function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
		if (!$this->writeAccess()) {
			trigger_localised_error('SYS0026', sprintf(translate('Unable to save "%s", permission denied'), $this->name), E_USER_WARNING);
			return FALSE;
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

		// reset opcache otherwise it would cache the constants of config file and redriect won't help.
		if(function_exists('opcache_reset')) {
		    opcache_reset();
		}

		// Check if a message should be sent.
		if ($changed_vars && $send_message) {
			$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
			$ms->openQueue();

			foreach ($changed_vars as $var_name => $values) {
				if (array_get_index($this->config_vars[$var_name], 'skip_mailing', FALSE)) {
					$msg_reps = Array(
									'screen_name' => $this->name,
									'setting'	=> $var_name,
								);
					$message = $ms->newMessage(Array(), 'config.'.get_class_lower($this).'.no_show_values', $msg_reps);
					$msg_body = translate('Value (not shown here) for %setting% has been changed');
				} else {
					$msg_reps = Array(
					                'screen_name' => $this->name,
									'setting'	  => $var_name,
									'old_value'   => var_export($values['old'], true),
									'new_value'	  => var_export($values['new'], true),
								);
					$message = $ms->newMessage(Array(), 'config.'.get_class_lower($this), $msg_reps);
					$msg_body = translate('%setting% changed from %old_value% to %new_value%');
				}

				$message->subject = translate('%screen_name% Updated');
				$message->body    = $msg_body;

				$ms->enqueueMessage($message);
			}//end foreach

			$ms->closeQueue();

		}// end if

		return TRUE;

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
	protected function _paintConfigVar($var_name, $value, $var_export=TRUE)
	{
		echo "define('", $var_name, "', ";
		if ($var_export) {
			var_export($value);
		} else {
			echo $value;
		}

		echo ");\n";

		return TRUE;

	}//end _paintConfigVar()


	/**
	* Attempt to acquire the lock for this config
	*
	* Returns true on success or the error message on failure

	* @return mixed boolean|string
	* @see MySource::acquireLock()
	*/
	public function acquireLock()
	{
		if (!$this->canAcquireLock()) {
			return 'You are not allowed to acquire the lock on '.$this->name;
		}
		return $GLOBALS['SQ_SYSTEM']->acquireLock(get_class_lower($this));

	}//end acquireLock()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @see MySource::acquireLock()
	*/
	public function canAcquireLock()
	{
		return TRUE;

	}//end canAcquireLock()


	/**
	* Attempt to release the lock for this config
	*
	* Returns true on success or the error message on failure
	*
	* @return mixed boolean|string
	* @see MySource::releaseLock()
	*/
	public function releaseLock()
	{
		return $GLOBALS['SQ_SYSTEM']->releaseLock(get_class_lower($this));

	}//end releaseLock()


	/**
	* Refreshes the lock on a config screen
	*
	* Returns true on success or error message on fail
	*
	* @return mixed boolean|string
	* @see MySource::updateLock();
	*/
	public function updateLock()
	{
		return $GLOBALS['SQ_SYSTEM']->updateLock(get_class_lower($this));

	}//end updateLock()


	/**
	* Returns an array of information about any lock we have
	*
	* @return Array()
	* @access public
	* @see MySource::getLockInfo()
	*/
	public function getLockInfo()
	{
		return $GLOBALS['SQ_SYSTEM']->getLockInfo(get_class_lower($this));

	}//end getLockInfo()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	public function writeAccess()
	{
		if (!isset($GLOBALS['SQ_SYSTEM'])) return TRUE;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		$lock_info = $this->getLockInfo();
		if (!empty($lock_info) && $lock_info['userid'] == $GLOBALS['SQ_SYSTEM']->currentUserId()) {
			return TRUE;
		}
		return FALSE;

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
	public function paintBackend(&$o)
	{
		if (is_null($this->config_file) === FALSE) {
			require_once $this->config_file;
		}

		$map_array = Array (
						'system_config'					=> 'system_configuration',
						'squiz_server_config'			=> 'squiz_server_configuration',
						'apache_config'					=> 'apache_configuration',
						'hipo_config'					=> 'hipo_configuration',
						'external_tools_config'			=> 'external_tools_configuration',
						'password_rules_config'			=> 'password_rules_configuration',
						'messaging_service_config'		=> 'messaging_service_configuration',
						'proxy_authentication_config'	=> 'proxy_configuration',
						'context_config'				=> 'context_configuration',
						'system_maintenance_config'		=> 'system_maintenance_configuration',
					 );

		$config_screen = $map_array[strtolower(get_class($this))];

		$lock = $this->getLockInfo();
		$have_lock = (!empty($lock) && $GLOBALS['SQ_SYSTEM']->currentUserId() == $lock['userid']);

		if (!empty($lock) || $this->canAcquireLock()) {

			$o->openRaw();
			?>
				<div class="sq-backend-section-div-wrapper section custom first">
					<div class="sq-backend-section-div">
						<div id="sq-aquire-locks-wrapper" class="clearfix">
							<?php
							echo '<div class="sq-locks-button-wrapper">';
								if (!empty($lock)) {
									if ($have_lock) {
										submit_button('sq_lock_release_manual', translate('Release Lock(s)'), 'set_hidden_field("process_form", "0");', 'accesskey="r" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button release"');

									}
								} else {
									submit_button('sq_lock_acquire', translate('Acquire Lock(s)'), 'set_hidden_field("process_form", "0");', 'accesskey="a" class="sq-form-field sq-btn-large sq-btn-blue sq-lock-button acquire"');

								}
							echo '</div>';
							echo '<p class="sq-lock-message">';
								if (!empty($lock)) {
									// this asset is currently locked
									// so display message to the user
									$user = $GLOBALS['SQ_SYSTEM']->am->getAsset($lock['userid']);

									$now = time();

									require_once SQ_FUDGE_PATH.'/general/datetime.inc';
									$expires_in = easy_time_total(($lock['expires'] - $now), TRUE);
									if (!$expires_in) $expires_in = '1 second';
									$expires_in = sprintf(translate('The lock is due to expire in %s'), $expires_in);

									if ($have_lock) {
										echo sprintf(translate('Press the <span class="sq-backend-button-fake">%1$s</span> button to release the locks you hold on "<strong>%2$s</strong>".'), translate('Release Lock(s)'), translate($config_screen));


									}
									if ($user->id != $GLOBALS['SQ_SYSTEM']->currentUserId()) {
										echo sprintf(translate('Held by user "%s"'), htmlspecialchars($user->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)).'. ';

									}
									echo ' '.$expires_in.'.';

								} else {
									echo sprintf(translate('Press the <span class="sq-backend-button-fake">%1$s</span> button to lock "<strong>%2$s</strong>" for editing.'), translate('Acquire Lock(s)'), translate($config_screen));


								}
							echo '</p>';
							?>
						</div><!--#sq-aquire-locks-wrapper-->
					</div><!--.sq-backend-section-div-->
				</div><!--.sq-backend-section-div-wrapper-->
			<?php
			$o->closeRaw();

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
	public function processBackend(&$o)
	{
		$saved = FALSE;
		if (!empty($_POST['process_form'])) {


			if (!empty($_POST[get_class_lower($this)])) {
				$saved = $this->save($_POST[get_class_lower($this)]);
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
