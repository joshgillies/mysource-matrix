<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Selection Asset Attribute
*
* parameters => Array(
*                     'options'  => Array('value' => 'text name')
*                     'multiple' => [true|false]
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*   <selection style="list"  />
*   <selection style="boxes" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage attributes
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
		if (!$attributeid) {
			// Set Some default params for when we are loaded without an ID
			$this->_params = Array('options'  => Array(), 'multiple' => false);
		}
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
		$this->_edit_params['height']    = empty($node->attributes['height'])    ? ''      : $node->attributes['height'];
		$this->_edit_params['columns']   = empty($node->attributes['columns'])   ? '1'     : $node->attributes['columns'];
		return true;
	}// end setEditParams()


	/**
	* Converts an array to a | seperated string
	*
	* @param array	$array	the array to be converted
	*
	* @return string
	* @access private
	*/
	function _ar2str($array)
	{
		return implode('|',array_intersect(array_keys($this->_params['options']),$array));
	}// end _ar2str()


	/**
	* Converts a | seperated string to an array
	*
	* @param string	$string	the string to be converted
	*
	* @return array
	* @access private
	*/
	function _str2ar($string)
	{
		return array_intersect(array_keys($this->_params['options']),explode('|',$string));
	}// end _str2ar()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$options = $this->_params['options'];
		if(!empty($this->_params['multiple'])) $this->value = $this->_str2ar($this->value);

		if ($read_only) {
			if (empty($this->value)) return;

			if(empty($this->_params['multiple'])) {
				if (!isset($options[$this->value])) {
					// may have a value left over from when we
					// were a mutiple selection field
					$value = $this->_str2ar($this->value);
					if (!isset($options[$value[0]])) return;
					echo $options[$value[0]];
				} else {
					echo $options[$this->value];
				}

			} else {
				echo '<ul>';
				foreach ($this->value as $k) {
					if (!isset($options[$k])) continue;
					echo '<li class="sq-backend-data">'.$options[$k].'</li>';
				}
				echo '</ul>';

			}
			return;
		}
		
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		// just have this so that we can check whether this field was posted or not
		// because checkboxes don't have an entry in the $_POST array when they are not selected
		hidden_field($prefix.'_'.$this->name.'_posted', '1');
		
		if($this->_edit_params['style'] == 'list') {
			combo_box($prefix.'_'.$this->name, $options, !empty($this->_params['multiple']), $this->value, $this->_edit_params['height']);
		} else {
			echo ('<table cellpadding="0" cellspacing="0" border="0"><tr>');
			reset($options);
			$i = 0;
			while(list($key,$val) = each($options)) {
				if(empty($this->_params['multiple'])) {
					?><td>&nbsp;<input type="radio" name="<?php echo $prefix.'_'.$this->name?>" value="<?php echo $key?>"<?php
					if($key == $this->value) echo ' checked';
					?>><?php echo $val?></td><?php
				} else {
					?><td>&nbsp;<input type="checkbox" name="<?php echo $prefix.'_'.$this->name."[$i]"?>" value="<?php echo $key?>"<?php
					if(is_array($this->value) && in_array($key, $this->value)) echo ' checked';
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
		require_once SQ_FUDGE_PATH.'/general/general.inc';
		if (!isset($_POST[$prefix.'_'.$this->name.'_posted'])) return false;
		$value = (isset($_POST[$prefix.'_'.$this->name])) ? $_POST[$prefix.'_'.$this->name] : '';

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
		if(empty($this->_params['multiple'])) {
			if (!isset($options[$value])) return false;
		} else {
			$new_value = $this->_str2ar($value);
			if (!is_array($new_value)) return false;
			$return_value = array();
			foreach ($new_value as $key) {
				if (isset($options[$key])) $return_value[] = $key;
			}
			$value = $this->_ar2str($return_value);
		}
		
		return true;
	}//end validateValue()


}//end class

?>