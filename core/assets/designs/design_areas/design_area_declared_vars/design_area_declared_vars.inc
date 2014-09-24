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
* $Id: design_area_declared_vars.inc,v 1.24 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Declared_Vars
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Declared_Vars extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Easy way to get the value of an attribute
	*
	* @param string	$name	Attribute name
	*
	* @return mixed string|object|int|float
	* @access public
	*/
	public function attr($name)
	{
		if (empty($this->vars['declared_vars']['value'][$name])) {
			return parent::attr($name);
		} else {
			return str_replace(Array('<'.'?php', '<'.'?', '?'.'>'), '', $this->vars['declared_vars']['value'][$name]['value']);
		}

	}//end attr()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* Declared Vars DAs never show any direct HTML output
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{

	}//end _printPaintingCode()


	/**
	* Returns an array of Asset_Attribute object, one for each declared var
	*
	* @return array
	* @access public
	*/
	public function getDeclaredVarAttributes()
	{
		$attrs = Array();
		foreach ($this->vars['declared_vars']['value'] as $name => $data) {
			$type = basename($data['type']); // just to make sure they aren't attempting to be a bit tricky
			// do include,so it doesn't die if it ain't there
			require_once SQ_ATTRIBUTES_PATH.'/'.$type.'/'.$type.'.inc';
			$attr_class = 'Asset_Attribute_'.$type;
			if (!class_exists($attr_class)) {
				trigger_localised_error('CORE0146', sprintf(translate('Attribute Type "%1$s" not found for "%2$s"'), $type, $name), E_USER_WARNING);
				continue;
			}
			$attrs[$name] = new $attr_class(0, $data['value']);
			$attrs[$name]->name = $name;
		}

		return $attrs;

	}//end getDeclaredVarAttributes()


	/**
	* Merges any new declared vars from the parent design
	*
	* @param array	$parent_attrs	Parent attributes
	*
	* @return void
	* @access public
	*/
	public function mergeParentDeclaredVars(Array $parent_attrs)
	{
		$current_attrs = $this->attr('declared_vars');
		foreach ($parent_attrs as $id => $value) {
			if (isset($current_attrs[$id])) {
				$parent_attrs[$id] = $current_attrs[$id];
			}
		}
		$this->setAttrValue('declared_vars', $parent_attrs);

	}//end mergeParentDeclaredVars()


}//end class
?>
