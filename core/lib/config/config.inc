<?php

/**
* Config
*
* Purpose
*
*    Looks after the creation of the config files that are in PHP code
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Config extends Resolve_Object
{
	/**
	* @var	Holds all the vars for this that config vars for this config
	*		In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	*/
	var $config_vars = Array();


	/**
	* @var	The file to save the config file to
	*		In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	*/
	var $config_file = '';


	/**
	* Constructor
	*
	*/
	function Config($config_file)
	{
		$this->Resolve_Object();
		$this->config_file = $config_file;
	}

	/**
	* Rewrites the conf file with current variables
	*
	*/
	function save($vars, $backup_existing=false)
	{
		// attempt to load the config file to make sure we get any current settings
		if (file_exists($this->config_file)) {
			require_once $this->config_file;
		}

		ob_start();
		foreach($this->config_vars as $var_name => $data) {

			$value = $data['default'];
			if ($data['editable'] && isset($vars[$var_name])) {
				$value = $vars[$var_name];
			} elseif (defined($var_name)) {
				$value = constant($var_name);
			}

			$this->_paintConfigVar($var_name, $value);

		}//end foreach

		$str = "<"."?php\n".ob_get_contents()."?".">\n";

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

		echo $str;
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		return string_to_file($str, $this->config_file);

	}// end save()


	/**
	* Paint's out the code for the config file for the passed config
	* This allows extra things to be added for special cases
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*/
	function _paintConfigVar($var_name, $value)
	{
		echo "define('".$var_name."', ".var_export($value, true).");\n";
	}//end _paintConfigVar()

}//end class

?>
