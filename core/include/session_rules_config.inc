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
* $Id: session_rules_config.inc,v 1.1 2005/05/26 03:18:15 dmckee Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';

/**
* System_Config
*
* Purpose
*
*    Looks after the creation and editing of the location of external tools
*    e.g. HTML Tidy, PDFtoHTML, Antiword, etc.
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Session_Rules_Config extends Config
{

	/**
	* Holds all the vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
						'SQ_CONF_SESSION_PERSIST'			=> Array('editable' => 1, 'default' => 0),
						'SQ_CONF_SESSION_TIMEOUT'			=> Array('editable' => 1, 'default' => 0),
						'SQ_CONF_SESSION_MAX_LENGTH'		=> Array('editable' => 1, 'default' => 0),
						'SQ_CONF_SESSION_GC_MAXLIFETIME'	=> Array('editable' => 1, 'default' => 1400),
						'SQ_CONF_EDITING_TIME'				=> Array('editable' => 0, 'default' => 3600),
					   );


	/**
	* Constructor
	*
	*/
	function Session_Rules_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/session_rules.inc';

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
	* Convert the amount of seconds into hours minutes and seconds
	*
	* @param int	$seconds - amount of time to convert
	*
	* @return string
	* @access public
	*/
	function days_hours_mins($seconds)
	{
		$time_string = '';
		$days = floor($seconds / (60 * 60 * 24));
		$hours = floor($seconds / (60 * 60) - $days * 24);
		$minutes = ($seconds / 60) - $hours * 60 - $days * 24 * 60 * 60;
		if ($days != 0) {
			$time_string .= strval($days).' day';
			$time_string .= ($days != 1) ? 's, ' : ', ' ;
		}
		$time_string .= strval($hours).' hour';
		$time_string .= ($hours != 1) ? 's and ' : ' and ' ;
		$time_string .= strval($minutes).' minute';
		$time_string .= ($minutes != 1) ? 's' : '' ;
		return $time_string;

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
	* @see User::validatePassword()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		eval('$persist_vals=Array('.SQ_CONF_SESSION_PERSIST.');');
		eval('$timeout_vals=Array('.SQ_CONF_SESSION_TIMEOUT.');');
		eval('$max_length_vals=Array('.SQ_CONF_SESSION_MAX_LENGTH.');');

		require_once $this->config_file;

		$write_access = $this->writeAccess();

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$user_types = $am->getTypeDescendants('user', true);

		$class = get_class($this);
		?>

		<script type="text/javascript">
			function toggleDiv(elt) {
					var span = elt.parentNode.getElementsByTagName('SPAN')[0];
					span.style.visibility = (span.style.visibility == 'hidden') ? 'visible' : 'hidden';
			}
		</script>
		<style type="text/css">
					td {
						white-space: nowrap;
					}
		</style>

		<?php
		$o->openSection('Manage Session Rules');

			$o->openField('End the user\'s session: ', 'new_line', '
												  * Matrix-Activity refers to either accessing a page on the frontend, or having the backend interface open
												  .');

				?>
				<table class="sq-backend-table" id="session_table">
				<tr>
					<th>&nbsp;</th>
					<th>When they close the browser</th>
					<th>When the time since their last <br />Matrix-Activity<sup>*</sup> is longer than</th>
					<th>When the time since their <br />last login is longer than</th>
				</tr>
				<?php
				foreach ($user_types as $user_id => $class_details) {
					?>
					<tr>
						<td>
							<?php
							$asset_attr = $am->getTypeInfo($class_details);
							echo $asset_attr['name'];
							?>
						</td>
						<td>
							<?php
							if ($write_access) {
								check_box('persist_'.$class_details, '1', !$persist_vals[$class_details]);
							} else {
								$output = $persist_vals[$class_details] == 0 ? 'Yes' : 'No';
								echo $output;
							}
							?>
						</td>
						<td>
							<?php 
								if ($write_access) {
									check_box('timeout_enabled_'.$class_details, '1', ($timeout_vals[$class_details] != 0), "toggleDiv(this)"); 
								}
							?>
							<span style="visibility:
								<?php 
								if (!$write_access || ($timeout_vals[$class_details] != 0)) {
									echo 'visible';
								} else {
									echo 'hidden';
								}
								?>">
								<?php
									$dur = new Asset_Attribute_Duration(0, $timeout_vals[$class_details]);
									if (($dur->value == 0) && (!$write_access)) {
										echo 'N/A';
									} else {
										$dur->paint('timeout_'.$class_details.'_', !$write_access);
									}
								?>
							</span>
						</td>
						<td>
							<?php
								if ($write_access) {
									check_box('max_length_enabled_'.$class_details, '1', ($max_length_vals[$class_details] != 0), "toggleDiv(this)");
								}
							?>
							<span style="visibility: 
								<?php 
								if (!$write_access || ($max_length_vals[$class_details] != 0)) {
									echo 'visible">';
								} else {
									echo 'hidden">';
								}
									$dur = new Asset_Attribute_Duration(0, $max_length_vals[$class_details]);
									if (($dur->value == 0) && (!$write_access)) {
										echo 'N/A';
									} else {
										$dur->paint('max_length_'.$class_details.'_', !$write_access);
									}
								?>
							</span>
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
	* @see User::validatePassword()
	*/
	function processBackend(&$o)
	{
		$class = get_class($this);
		require_once $this->config_file;
		$persist_vals = '';
		$timeout_vals = '';
		$max_length_vals = '';
		$overall_max = 1440;
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$user_types = $am->getTypeDescendants('user', true);
		foreach ($user_types as $user_id => $class_details) {

			//process persist checkboxes
			$persist_vals .= "'".$class_details."' => ";
			$check_val = isset($_POST['persist_'.$class_details]) ? '0' : '1';
			$persist_vals .= $check_val.', ';

			//process timeout duration boxes
			$timeout_dur = new Asset_Attribute_Duration();
			if (isset($_POST['timeout_enabled_'.$class_details])) {
				$timeout_dur->process('timeout_'.$class_details.'_');
				$timeout_vals .= "'".$class_details."' => ".$timeout_dur->value.', ';
			} else {
				$timeout_vals .= "'".$class_details.'\' => 0, ';
			}

			//process max length duration boxes
			$max_length_dur = new Asset_Attribute_Duration();
			if (isset($_POST['max_length_enabled_'.$class_details])) {
				$max_length_dur->process('max_length_'.$class_details.'_');
				$max_length_vals .= "'".$class_details."' => ".$max_length_dur->value.', ';
			} else {
				$max_length_vals .= "'".$class_details.'\' => 0, ';
			}
			
			if ($max_length_dur->value > $overall_max) {
				$overall_max = intval($max_length_dur->value);
			}
		}
		
		//setup new constants array
			$_POST[$class]['SQ_CONF_SESSION_PERSIST'] = $persist_vals;
			$_POST[$class]['SQ_CONF_SESSION_TIMEOUT'] = $timeout_vals;
			$_POST[$class]['SQ_CONF_SESSION_MAX_LENGTH'] = $max_length_vals;
			$_POST[$class]['SQ_CONF_SESSION_GC_MAXLIFETIME'] = $overall_max * 24 * 60 * 60;

		return parent::processBackend($o);

	}//end processBackend()


}//end class

?>
