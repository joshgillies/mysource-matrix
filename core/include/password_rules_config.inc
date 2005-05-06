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
* $Id: password_rules_config.inc,v 1.1 2005/05/06 03:38:13 ndvries Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

/**
* System_Config
*
* Purpose
*
*    Looks after the creation and editing of the location of external tools
*    e.g. HTML Tidy, PDFtoHTML, Antiword, etc.
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
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
						'SQ_PASSWD_RULE_LENGTH'					=> Array('editable' => 1, 'default' => 6),
						'SQ_PASSWD_RULE_ALLOW_USER_INFO'		=> Array('editable' => 1, 'default' => 1),
						'SQ_PASSWD_RULE_NUM_CAPS'				=> Array('editable' => 1, 'default' => -1),
						'SQ_PASSWD_RULE_NUM_LOWER'				=> Array('editable' => 1, 'default' => -1),
						'SQ_PASSWD_RULE_NUM_INT'				=> Array('editable' => 1, 'default' => -1),
						'SQ_PASSWD_RULE_NUM_PUNC'				=> Array('editable' => 1, 'default' => -1),
						'SQ_PASSWD_RULE_NUM_SPC'				=> Array('editable' => 1, 'default' => -1),

					   );


	/**
	* Constructor
	*
	*/
	function Password_Rules_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/password_rules.inc';

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
	function save($vars, $backup_existing=false, $send_message=true)
	{
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, go away
			if (!$is_admin) {
				trigger_localised_error('SYS0267', E_USER_WARNING, $this->name);
				return false;
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

		$o->openSection('Manage Password Rules');


			$o->openField('Minimum Length', '', 'Specifiy a minimum password length');
				if ($write_access && $is_root) {
					text_box($class.'[SQ_PASSWD_RULE_LENGTH]', SQ_PASSWD_RULE_LENGTH, 5);
				} else {
					echo SQ_PASSWD_RULE_LENGTH;
				}
			$o->closeField();

			$o->openField('Allow User Info', '', 'Prevent users having their username or other user information as their password.');
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_PASSWD_RULE_ALLOW_USER_INFO]', Array('1' => 'Yes', '0' => 'No'), false, SQ_PASSWD_RULE_ALLOW_USER_INFO);
				} else {
					echo (SQ_PASSWD_RULE_ALLOW_USER_INFO) ? 'Yes' : 'No';
				}
			$o->closeField();

			$o->openField('Character Rules', '', 'Enforce the use of a certain type of character in passwords by either specifying the minimum number of times it must be used, or specifying "0" to disable its use.');

				?>
				<table class="sq-backend-table">
				<tr>
					<td style="width: 200px;" class="sq-backend-table-header">Character Type</td>
					<td class="sq-backend-table-header">Minimum Number</td>
					<?php
						if ($write_access) {
							?><td style="width: 50px;" class="sq-backend-table-header">Disable Checking?</td><?php
						}
					?>
				</tr>
				<?php
				foreach (get_constant_values('SQ_PASSWD_RULE_NUM') as $password_rule => $value) {
					?>
					<tr>
						<td class="sq-backend-table-cell">
							<?php
							switch ($password_rule) {
								case 'SQ_PASSWD_RULE_NUM_CAPS':
									echo 'Capital letters';
									break;
								case 'SQ_PASSWD_RULE_NUM_LOWER':
									echo 'Lower case letters';
									break;
								case 'SQ_PASSWD_RULE_NUM_INT':
									echo 'Numbers';
									break;
								case 'SQ_PASSWD_RULE_NUM_PUNC':
									echo 'Punctuation';
									break;
								case 'SQ_PASSWD_RULE_NUM_SPC':
									echo 'Spaces';
									break;

								default:
									continue;
							}
							?>
						</td>
						<td class="sq-backend-table-cell">
							<?php
							if ($value > -1) {
								if ($write_access) {
									int_text_box($class.'['.$password_rule.']', $value, false, 5);
								} else {
									echo $value;
								}
							} else {
								echo '&nbsp;';
							}
							?>
						</td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php
							check_box($class.'_disable'.'['.$password_rule.']', '1', ($value > -1 ? false : true));
							?></td><?php
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
		$o->closeSection();

		if ($write_access) $o->commitButton('', true);

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
		$class = get_class($this);
		require_once $this->config_file;

		$const = get_constant_values('SQ_PASSWD_RULE_NUM');
		foreach ($const as $password_rule => $value) {
			// get the values from $_POST[password_rules_config(_enable)][password_rule]
			eval('$check_box = isset($_POST'."['".$class.'_disable'."']"."['".$password_rule."']".');');
			eval('$text_box  = (isset($_POST'."['".$class."']"."['".$password_rule."']".') ? $_POST'."['".$class."']"."['".$password_rule."']".' : false);');
			if ($check_box) {
				$_POST['password_rules_config'][$password_rule] = -1;
			} else {
				if ($text_box === false) {
					// fool the config into thinking a textbox was passed
					$_POST[$class][$password_rule] = 1;
				}
			}
		}

		return parent::processBackend($o);
	}


}//end class

?>
