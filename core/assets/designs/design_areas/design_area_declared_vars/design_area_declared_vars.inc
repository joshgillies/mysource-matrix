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
* @package Resolve::packages::__core__
*/
class Design_Area_Declared_Vars extends Design_Area
{
	
	var $declared_vars = Array();

	/**
	* Load the asset represented by the passed asset id
	*
	* @param int    $assetid      the asset id to be loaded
	*
	* @access public
	*/
	function load($assetid)
	{
		parent::load($assetid);
		if ($this->vars['declared_vars']['value']) $this->declared_vars = unserialize($this->vars['declared_vars']['value']);
	}//end load()

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
		if (empty($this->declared_vars[$name])) {
			return parent::attr($name);
		} else { 
			return $this->declared_vars[$name]['value'];
		}
	}//end attr();

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	*
	* @access public
	*/
	function printVar($var) 
	{
		echo $this->declared_vars[$var]['value'];
	}//end printVar()


}//end class
?>
