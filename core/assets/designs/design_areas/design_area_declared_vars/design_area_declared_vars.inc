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
* $Id: design_area_declared_vars.inc,v 1.11 2003/11/26 00:50:56 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Declared_Vars
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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


	/**
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


	/**
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
			require_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
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
