<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: config.inc,v 1.6 2003/10/14 22:46:06 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Config
*
* Purpose
*
*    Looks after the creation of the config files that are in PHP code
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
	* Constructor
	*
	*/
	function Config()
	{
		$this->MySource_Object();
	}// end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited) Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false)
	{
		// attempt to load the config file to make sure we get any current settings
		if (file_exists($this->config_file)) {
			require_once $this->config_file;
		}

		$changed_vars = Array();
		ob_start();
		foreach($this->config_vars as $var_name => $data) {

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
			} while(file_exists($old_version));

			if (!copy($this->config_file, $old_version)) {
				return false;
			}

		}// endif

		if (SQ_PHP_CLI) echo "----------------------------------\n".($str)."----------------------------------\n";
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!string_to_file($str, $this->config_file)) return false;

		if ($changed_vars) {
			$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
			$message_body = '';
			foreach($changed_vars as $var_name => $values) {
				$message_body .= $var_name.' changed from '.var_export($values['old'], true).' to '.var_export($values['new'], true)."\n";
			}
			$config_name = ucwords(str_replace('_', ' ', get_class($this)));
			$message = $ms->newMessage(Array(), $config_name.' Updated', $message_body, 'config.'.get_class($this));
			$message->send();

		}// end if

		return true;

	}// end save()


	/**
	* Paint's out the code for the config file for the passed config
	* This allows extra things to be added for special cases, and extra checks to be made if necessary on the value
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value)
	{
		echo "define('".$var_name."', ".var_export($value, true).");\n";
		return true;
	}//end _paintConfigVar()

}//end class

?>
