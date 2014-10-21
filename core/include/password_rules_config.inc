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
* $Id: password_rules_config.inc,v 1.17 2013/09/12 06:42:54 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* System_Config
*
* Purpose
*
*    Allows the configuration of rules for passwords
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.17 $
* @package MySource_Matrix
*/
class Password_Rules_Config extends Config
{

	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
						'SQ_PASSWD_RULE_LENGTH'				=> Array('editable' => 1, 'default' => 6),
						'SQ_PASSWD_RULE_DISALLOW_USER_INFO'	=> Array('editable' => 1, 'default' => 1),
						'SQ_PASSWD_RULE_NUM_CAPS'			=> Array('editable' => 1, 'default' => 0),
						'SQ_PASSWD_RULE_NUM_LOWER'			=> Array('editable' => 1, 'default' => 0),
						'SQ_PASSWD_RULE_NUM_INT'			=> Array('editable' => 1, 'default' => 0),
						'SQ_PASSWD_RULE_NUM_PUNC'			=> Array('editable' => 1, 'default' => 0),
						'SQ_PASSWD_RULE_NUM_SPC'			=> Array('editable' => 1, 'default' => 0),
						'SQ_PASSWD_RULE_DISALLOWED'			=> Array('editable' => 0, 'default' => -1),
						'SQ_PASSWD_RULE_DISABLED'			=> Array('editable' => 0, 'default' => 0),
						'SQ_PASSWD_RULE_BLACK_LIST'			=> Array('editable' => 1, 'default' => ''),
						'SQ_PASSWD_RULE_BLACK_LIST_EXACT'	=> Array('editable' => 1, 'default' => 1),
						'SQ_PASSWD_RULE_PASSWORD_HISTORY'	=> Array('editable' => 1, 'default' => 0),
					   );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/password_rules.inc';
		$this->name = translate('Password Rules Configuration');

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
				trigger_localised_error('SYS0267', sprintf(translate('You need to be either the Root User or a System Administrator to save the %s'), $this->name), E_USER_WARNING);
				return FALSE;
			}
		}

		if (isset($vars['SQ_PASSWD_RULE_BLACK_LIST'])) {
			$vars['SQ_PASSWD_RULE_BLACK_LIST']= trim($vars['SQ_PASSWD_RULE_BLACK_LIST']);
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
	* @see User::validatePassword()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$class = get_class_lower($this);

		$o->openSection(translate('Manage Password Rules'));



			$o->openField(translate('Minimum Length'), '', translate('Specify a minimum password length'));


				if ($write_access && $is_root) {
					text_box($class.'[SQ_PASSWD_RULE_LENGTH]', SQ_PASSWD_RULE_LENGTH, 5);
				} else {
					echo SQ_PASSWD_RULE_LENGTH;
				}
			$o->closeField();

			$o->openField(translate('Disallow User Info'), '', translate('If yes, then prevent users having their username or other user information as their password.'));


				if ($write_access && $is_root) {
					combo_box($class.'[SQ_PASSWD_RULE_DISALLOW_USER_INFO]', Array('0' => translate('Yes'), '1' => translate('No')), FALSE, SQ_PASSWD_RULE_DISALLOW_USER_INFO);


				} else {
					echo (SQ_PASSWD_RULE_DISALLOW_USER_INFO) ? translate('No') : translate('Yes');


				}
			$o->closeField();

			$o->openField(translate('Character Rules'), '', translate('Enforce the use of a certain type of character in passwords by specifying the minimum number of times it must be used.<br />Disallowing a character type will prevent users from including it in their passwords.'));



				?>
				<table class="sq-backend-table">
				<tr>
					<th style="width: 50px; text-align: center;" class="sq-backend-table-header"><?php echo translate('Active?'); ?></th>

					<th style="width: 200px;" class="sq-backend-table-header"><?php echo translate('Character Type'); ?></th>

					<th class="sq-backend-table-header"><?php echo translate('Minimum Number'); ?></th>

					<th style="width: 150px;" class="sq-backend-table-header"><?php echo translate('Disallow Character Type?'); ?></th>

				</tr>
				<?php
				foreach (get_constant_values('SQ_PASSWD_RULE_NUM') as $password_rule => $value) {
					?>
					<tr>
						<td align="middle" class="sq-backend-table-cell">
						<?php
							if ($write_access) {
								check_box($class.'_active'.'['.$password_rule.']', '1', ($value != SQ_PASSWD_RULE_DISABLED ? TRUE : FALSE));
							} else {
								?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $value != SQ_PASSWD_RULE_DISABLED ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
							}
						?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							switch ($password_rule) {
								case 'SQ_PASSWD_RULE_NUM_CAPS':
									echo translate('Capital letters');

								break;
								case 'SQ_PASSWD_RULE_NUM_LOWER':
									echo translate('Lower case letters');

								break;
								case 'SQ_PASSWD_RULE_NUM_INT':
									echo translate('Numeric digits');

								break;
								case 'SQ_PASSWD_RULE_NUM_PUNC':
									echo translate('Punctuation characters');

								break;
								case 'SQ_PASSWD_RULE_NUM_SPC':
									echo translate('Spaces');

								break;

								default:
									continue;
							}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							if ($value != SQ_PASSWD_RULE_DISABLED && $value != SQ_PASSWD_RULE_DISALLOWED) {
								if ($write_access) {
									int_text_box($class.'['.$password_rule.']', $value, FALSE, 5);
								} else {
									echo $value;
								}
							} else {
								echo '&nbsp;';
							}
							?>
						</td>
						<td class="sq-backend-table-cell" style="text-align: center;">
							<?php
								// if disabled
								if ($value == SQ_PASSWD_RULE_DISABLED) {
									echo '&nbsp;';
								} else {
									if ($write_access) {
										check_box($class.'_disallow'.'['.$password_rule.']', '1', ($value == SQ_PASSWD_RULE_DISALLOWED));
									} else {
										?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $value == SQ_PASSWD_RULE_DISALLOWED ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
									}
								}
							?>
						</td>
					</tr>
					<?php
				}//end foreach
				?>
				</table>
				<?php
			$o->closeField();


			$o->openField(translate('Password History Enforcement'), '', translate('Number of unique new passwords that have to be associated with and used by a user before an old password can be reused again. Enter 0 to disable this feature and stop Matrix from recording history passwords.'));


				if ($write_access && $is_root) {
					text_box($class.'[SQ_PASSWD_RULE_PASSWORD_HISTORY]', SQ_PASSWD_RULE_PASSWORD_HISTORY, 0);
				} else {
					echo SQ_PASSWD_RULE_PASSWORD_HISTORY;
				}
			$o->closeField();

		$o->closeSection();

		$o->openSection(translate('Password Blacklist'));

			$o->openField(translate('Word List'), '', translate('New line separated list of words that are not allowed to be used as password.'));



				if ($write_access) {
					text_area($class.'[SQ_PASSWD_RULE_BLACK_LIST]', SQ_PASSWD_RULE_BLACK_LIST, 30, 8);
				} else {
					echo nl2br(SQ_PASSWD_RULE_BLACK_LIST);
				}
			$o->closeField();

			$o->openField(translate('Exact Match'), '', translate('If ticked, the system will only stop users using a word from the Word List as a password if it\'s an exact match.'));



				if ($write_access) {
					check_box($class.'[SQ_PASSWD_RULE_BLACK_LIST_EXACT]', '1', SQ_PASSWD_RULE_BLACK_LIST_EXACT);
					label(translate('Only blacklist exactly matched words'), $class.'[SQ_PASSWD_RULE_BLACK_LIST_EXACT]');

				} else {
					?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo SQ_PASSWD_RULE_BLACK_LIST_EXACT ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
				}

			$o->closeField();

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
	* @see User::validatePassword()
	*/
	function processBackend(&$o)
	{
		$class = get_class_lower($this);
		require_once $this->config_file;

		$const = get_constant_values('SQ_PASSWD_RULE_NUM');
		foreach ($const as $password_rule => $value) {
			// get the values from $_POST[password_rules_config(_enable)][password_rule]
			eval('$active_check_box = isset($_POST'."['".$class.'_active'."']"."['".$password_rule."']".');');
			eval('$disallow_check_box = isset($_POST'."['".$class.'_disallow'."']"."['".$password_rule."']".');');
			eval('$text_box  = (isset($_POST'."['".$class."']"."['".$password_rule."']".') ? $_POST'."['".$class."']"."['".$password_rule."']".' : false);');

			if ($disallow_check_box) {
				if ($active_check_box) {
					$_POST[$class][$password_rule] = SQ_PASSWD_RULE_DISALLOWED;
				} else {
					$_POST[$class][$password_rule] = SQ_PASSWD_RULE_DISABLED;
				}
			} else {
				if ($active_check_box) {
					if ($text_box === FALSE) {
						$_POST[$class][$password_rule] = 1;
					}
				} else {
					$_POST[$class][$password_rule] = SQ_PASSWD_RULE_DISABLED;
				}
			}
		}

		if (!isset($_POST[$class]['SQ_PASSWD_RULE_BLACK_LIST_EXACT'])) {
			$_POST[$class]['SQ_PASSWD_RULE_BLACK_LIST_EXACT'] = 0;
		} else {
			$_POST[$class]['SQ_PASSWD_RULE_BLACK_LIST_EXACT'] = 1;
		}

		return parent::processBackend($o);

	}//end processBackend()


}//end class

?>
