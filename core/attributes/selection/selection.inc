<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* Selection Asset Attribute
*
* 
*
*
* <!-- Sample Edit Interface XML Node -->
*   <selection style="list"  multiple="0" />
*   <selection style="boxes" multiple="1" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::attributes
*/
class Asset_Attribute_Selection extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Selection($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);
	}// end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		$this->_edit_params['style']     = empty($node->attributes['style'])     ? 'list'  : $node->attributes['style'];
		$this->_edit_params['multiple']  = empty($node->attributes['multiple'])  ? '0'     : $node->attributes['multiple'];
		$this->_edit_params['height']    = empty($node->attributes['height'])    ? ''      : $node->attributes['height'];
		$this->_edit_params['columns']   = empty($node->attributes['columns'])   ? '1'     : $node->attributes['columns'];
		return true;
	}


	/**
	* Converts an array to a | seperated string
	*
	* @param array	$array	the array to be converted
	*
	* @return string
	* @access private
	*/
	function _ar2str($array) {
		return implode('|',array_intersect(array_keys($this->_params['options']),$array));
	}


	/**
	* Converts a | seperated string to an array
	*
	* @param string	$string	the string to be converted
	*
	* @return array
	* @access private
	*/
	function _str2ar($string) {
		return array_intersect(array_keys($this->_params['options']),explode('|',$string));
	}


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @access public
	*/
	function paint($prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$options = $this->_params['options'];

		if($this->_edit_params['multiple']) $this->value = $this->_str2ar($this->value);
		if($this->_edit_params['style'] == 'list') {
			combo_box($prefix.'_'.$this->name, $options, $this->_edit_params['multiple'], $this->value, $this->_edit_params['height']);
		} else {
			echo ('<table cellpadding="0" cellspacing="0" border="0"><tr>');
			reset($options);
			$i = 0;
			while(list($key,$val) = each($options)) {
				if($this->_edit_params['multiple']) {
					?><td>&nbsp;<input type="checkbox" name="<?php echo $prefix.'_'.$this->name."[$i]"?>" value="<?php echo $key?>"<?php
					if(is_array($this->value) && in_array($key, $this->value)) echo ' checked';
					?>><?php echo $val?></td><?php
				} else {
					?><td>&nbsp;<input type="radio" name="<?php echo $prefix.'_'.$this->name?>" value="<?php echo $key?>"<?php
					if($key == $this->value) echo ' checked';
					?>><?php echo $val?></td><?php
				}
				if ((($i++) % $this->_edit_params['columns']) == ($this->_edit_params['columns'] - 1)) {
					echo "</tr><tr>" ;
				}
			}
			echo "</tr></table>" ;
		}
		
	}// end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$value = (isset($_POST[$prefix.'_'.$this->name])) ? $_POST[$prefix.'_'.$this->name] : '';
		$value = gpc_stripslashes($value);

		if(is_array($value)) {
			$value = $this->_ar2str($value);
		}

		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}
	}// end process()


	/**
	* Ensure the attribute being set is a valid selection
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$options = $this->_params['options'];

		$value = trim((string) $value);
		if ($value == '') return true;

		// special case for multiple values
		if($this->_edit_params['multiple']) {
			$new_value = $this->_str2ar($value);
			if (!is_array($new_value)) return false;
			$return_value = array();
			foreach ($new_value as $key) {
				if (isset($options[$key])) $return_value[] = $key;
			}
			$value = $this->_ar2str($return_value);
		} else {
			if (!isset($options[$value])) return false;
		}
		
		return true;
	}//end validateValue()


}//end class

?>