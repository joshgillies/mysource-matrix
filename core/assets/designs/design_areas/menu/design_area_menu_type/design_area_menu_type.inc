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

	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array('design_area_menu_type' => 'M'),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


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

		// if they are trying to set a level, so some validation
		} else if ($name == 'level') {
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

		// if they are trying to set the type_codes, split the string
		} else if ($name == 'type_codes') {
			$value = preg_split('/[ ,]+/', $value);

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


	/**
	* Returns an array of Array(id_name => Array(var)) for all var references that are needed by this design area
	*
	* @access public
	*/
	function getVarReferences()
	{
		$var_refs = parent::getVarReferences();
		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_var_refs = $sub_menu->getVarReferences();
			for(reset($sub_var_refs); null !== ($id_name = key($sub_var_refs)); next($sub_var_refs)) {
				if (!isset($var_refs[$id_name])) $var_refs[$id_name] = Array();
				foreach($sub_var_refs[$id_name] as $var) {
					if (in_array($var, $var_refs[$id_name])) continue;
					$var_refs[$id_name][] = $var;
				}// end foreach
			}// end foreach
		}// end if

		return $var_refs;

	}//end getVarReferences();


	/**
	* Set's the values for the var references values (registered in $this->var_references)
	*
	* @param array $var_ref_values	array of values - Array(id_name => Array(var => [value]))
	*
	* @access public
	*/
	function setVarReferenceValues($var_ref_values)
	{
		parent::setVarReferenceValues($var_ref_values);
		$sub_menu = &$this->getSubMenu();
		if (!is_null($sub_menu)) {
			$sub_menu->setVarReferenceValues($var_ref_values);
		}// end if
	}//end setVarReferenceValues();


	/*
	* Returns the link to the sub menu for this menu (if any)
	*
	* @return Design_Area_Menu_Type | null
	* @access public
	*/
	function getSubMenuLink() 
	{
		return $this->getLink(SQ_LINK_USES, 'design_area_menu_type', false, 'sub_menu');
	}//end getSubMenuLink()

	/*
	* Returns the sub menu for this menu (if any)
	*
	* @return Design_Area_Menu_Type | null
	* @access public
	*/
	function &getSubMenu() 
	{
		$link = $this->getSubMenuLink();
		if (empty($link)) {
			$null = null;
			return $null;
		}
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code'], true);
	}//end getSubMenu()

	/*
	* Returns the parent menu for this menu (assuming it is a sub menu)
	*
	* @return Design_Area_Menu_Type | null
	* @access public
	*/
	function &getParentMenu()
	{
		$link = $this->getLink(SQ_LINK_USES, 'design_area_menu_type', false, 'sub_menu', 'minor');
		if (empty($link)) {
			$null = null;
			return $null;
		}
		return $GLOBALS['SQ_SYSTEM']->am->getAsset($link['majorid'], $link['type_code'], true);
	}//end getParentMenu()

	/*
	* Returns whether the passed setting exists or not
	*
	* @return boolean
	* @access public
	*/
	function settingExists($name)
	{
		return !empty($this->vars['settings']['value'][$name]);
	}//end settingExists()

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		// if they are trying to set a page position dependent setting
		if (substr($var, 0, 9) == 'settings.') {
			// The name is in the form 'settings.[var_name].[setting type]'
			list(,$setting_name) = explode('.', $var, 2);
			if ($setting_name && $this->settingExists($setting_name)) {
				echo '<', '?php echo $'.$this->attr('id_name').'_settings[\''.$setting_name.'\'][$'.$this->attr('id_name').'_current_setting]; ?', '>';
			}// end if
		} else {

			if (!empty($args['escape']) && strtolower($args['escape']) != 'no') { 
				$value_prefix = 'addslashes('.$value_prefix;
				$value_suffix = $value_suffix.')';
			}

			switch($var) {
				case 'page_link' :
					echo '<?', 'php echo $'.$this->attr('id_name').'_kid->getHref($'.$this->attr('id_name').'_base_url); ?', '>';
					break;

				case 'page_short_name' :
					echo '<?', 'php echo ', $value_prefix, '$'.$this->attr('id_name').'_kid->short_name', $value_suffix, '; ?', '>';
					break;

				case 'page_name' :
					echo '<?', 'php echo ', $value_prefix, '$'.$this->attr('id_name').'_kid->name', $value_suffix, '; ?', '>';
					break;

				default :
					parent::printVar($var, $args);
			}// end switch
		}// end if

	}//end printVar()


}//end class
?>
