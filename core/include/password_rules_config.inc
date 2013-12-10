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

		$o->openSection(translate('manage_passwd_rules'));


			$o->openField(translate('min_length'), '', translate('specify_min_length_passwd'));
				if ($write_access && $is_root) {
					text_box($class.'[SQ_PASSWD_RULE_LENGTH]', SQ_PASSWD_RULE_LENGTH, 5);
				} else {
					echo SQ_PASSWD_RULE_LENGTH;
				}
			$o->closeField();

			$o->openField(translate('disallow_user_info'), '', translate('disallow_user_info_note'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_PASSWD_RULE_DISALLOW_USER_INFO]', Array('0' => translate('yes'), '1' => translate('no')), FALSE, SQ_PASSWD_RULE_DISALLOW_USER_INFO);
				} else {
					echo (SQ_PASSWD_RULE_DISALLOW_USER_INFO) ? translate('no') : translate('yes');
				}
			$o->closeField();

			$o->openField(translate('character_rules'), '', translate('character_rules_note'));

				?>
				<table class="sq-backend-table">
				<tr>
					<th style="width: 50px; text-align: center;" class="sq-backend-table-header"><?php echo translate('active_question'); ?></th>
					<th style="width: 200px;" class="sq-backend-table-header"><?php echo translate('character_type'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('min_number'); ?></th>
					<th style="width: 150px;" class="sq-backend-table-header"><?php echo translate('disallow_char_type_question'); ?></th>
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
									echo ucfirst(translate('capital_letters'));
								break;
								case 'SQ_PASSWD_RULE_NUM_LOWER':
									echo ucfirst(translate('lowercase_letters'));
								break;
								case 'SQ_PASSWD_RULE_NUM_INT':
									echo ucfirst(translate('numeric_digits'));
								break;
								case 'SQ_PASSWD_RULE_NUM_PUNC':
									echo ucfirst(translate('punctuation_characters'));
								break;
								case 'SQ_PASSWD_RULE_NUM_SPC':
									echo ucfirst(translate('spaces'));
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
			
			
			$o->openField(translate('password_history_enforcement'), '', translate('password_history_enforcement_note'));
				if ($write_access && $is_root) {
					text_box($class.'[SQ_PASSWD_RULE_PASSWORD_HISTORY]', SQ_PASSWD_RULE_PASSWORD_HISTORY, 0);
				} else {
					echo SQ_PASSWD_RULE_PASSWORD_HISTORY;
				}
			$o->closeField();
			
		$o->closeSection();

		$o->openSection(translate('passwd_blacklist'));
			$o->openField(translate('word_list'), '', translate('word_list_note'));

				if ($write_access) {
					text_area($class.'[SQ_PASSWD_RULE_BLACK_LIST]', SQ_PASSWD_RULE_BLACK_LIST, 30, 8);
				} else {
					echo nl2br(SQ_PASSWD_RULE_BLACK_LIST);
				}
			$o->closeField();

			$o->openField(translate('exact_match'), '', translate('exact_match_note'));

				if ($write_access) {
					check_box($class.'[SQ_PASSWD_RULE_BLACK_LIST_EXACT]', '1', SQ_PASSWD_RULE_BLACK_LIST_EXACT);
					label(translate('exact_match_blacklist_words'), $class.'[SQ_PASSWD_RULE_BLACK_LIST_EXACT]');
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
