<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Menu_Type
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Menu_Type extends Design_Area
{

	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value) 
	{
		// if they are trying to set a page position dependent setting
		if (substr($name, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			// where setting type is either 'normal', 'heirarchy' or 'current'
			$split = explode('.', $name);
			if (count($split) != 3) {
				trigger_error('Setting "'.$name.'" unable to be set, three elements in name not found', E_USER_WARNING);
				return false;
			}
			switch($split[2]) {
				case 'normal'    : 
				case 'hierarchy' : 
				case 'current'   : 
					$new_value = $this->vars['settings']['value'];
					if(!is_array($new_value)) $new_value = Array();
					if(!isset($new_value[$split[1]])) $new_value[$split[1]] = Array();
					$new_value[$split[1]][$split[2]] = $value;
					$name  = 'settings';
					$value = $new_value;
					break;

				default : 
					trigger_error('Settings Type "'.$split[2].'" not known', E_USER_WARNING);
					return false;
			}// end switch

		}// end if

		// if they are trying to set a level, so some validation
		if ($name == 'level') {
			switch(strtolower($value)) {
				case 'top';
					$value = 0;
					break;
				case 'sub';
					$value = 1;
					break;
				default :
					$value = max(0, (int) $value);
			}// end switch

		}// end if


		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name
	*
	* @return mixed
	* @access public
	*/
	function attr($name) 
	{
		// if they are trying to set a page position dependent setting
		if (substr($name, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			$split = explode('.', $name);
			if (count($split) == 3 && isset($this->vars['settings']['value'][$split[1]][$split[2]])) {
				return $this->vars['settings']['value'][$split[1]][$split[2]];
			}// end if
		}// end if

		// if we hit this, just send it upwards
		return parent::attr($name);

	}//end attr();	

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @access public
	*/
	function printVar($var, $args)
	{
		// if they are trying to set a page position dependent setting
		if (substr($var, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			list(,$setting_name) = explode('.', $var, 2);
			if ($setting_name && isset($this->vars['settings']['value'][$setting_name])) {
				echo '<', '?php echo $settings[\''.$setting_name.'\'][$current_setting]; ?', '>';
			}// end if
		} else {

			if (empty($args['escape']) || strtolower($args['escape']) == 'no') { 
				$escape_start = '';
				$escape_end = '';
			} else {
				$escape_start = 'addslashes(';
				$escape_end = ')';
			}

			switch($var) {
				case 'page_link' :
					echo '<?', 'php echo $kid->getHref(); ?', '>';
					break;

				case 'page_short_name' :
					echo '<?', 'php echo ', $escape_start, '$kid->short_name', $escape_end, '; ?', '>';
					break;

				case 'page_name' :
					echo '<?', 'php echo ', $escape_start, '$kid->name', $escape_end, '; ?', '>';
					break;

				default :
					echo 'PRINT '.$var;
					#parent::printVar($var);
			}// end switch
		}// end if

	}//end printVar()


}//end class
?>
