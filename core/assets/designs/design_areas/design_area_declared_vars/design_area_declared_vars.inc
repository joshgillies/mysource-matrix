<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Declared_Vars
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Area_Declared_Vars extends Design_Area
{

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
		if (empty($this->vars['declared_vars']['value'][$name])) {
			return parent::attr($name);
		} else { 
			return $this->vars['declared_vars']['value'][$name]['value'];
		}
	}//end attr();

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design) 
	{
		return true;
	}//end printArea()

	/*
	* Returns an array of Asset_Attribute object, one foreach declared var
	*
	* @return Array(object Asset_Attribute)
	* @access public
	*/
	function getDeclaredVarAttributes() 
	{
		$attrs = Array();
		foreach($this->vars['declared_vars']['value'] as $name => $data) {
			$type = basename($data['type']); // just to make sure they aren't attempting to be a bit tricky
			// do include,so it doesn't die if it ain't there
			include_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
			$attr_class = 'Asset_Attribute_'.$type;
			if(!class_exists($attr_class)) {
				trigger_error('Attribute Type "'.$type.'" not found for "'.$name.'"', E_USER_WARNING);
				continue;
			}
			$attrs[$name] = new $attr_class(0, $data['value']);
			$attrs[$name]->name = $name;
		}

		return $attrs;
		
	}//end getDeclaresVarAttributes()


}//end class
?>
