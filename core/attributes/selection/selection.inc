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
* $Id: selection.inc,v 1.24 2004/06/25 16:12:30 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


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
*   <selection style="list"  height="1" />
*   <selection style="boxes" columns="1" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
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

		// set default edit parameters
		$this->_edit_params['style']   = 'list';
		$this->_edit_params['height']  = '';
		$this->_edit_params['columns'] = '1';

	}//end constructor


	/**
	* Take an XML_Tree_Node from an edit interface XML file and create the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (!empty($node->attributes['style'])) $this->_edit_params['style'] = $node->attributes['style'];
		if (!empty($node->attributes['height'])) $this->_edit_params['height'] = $node->attributes['height'];
		if (!empty($node->attributes['columns'])) $this->_edit_params['columns'] = $node->attributes['columns'];
		return true;

	}//end setEditParams()


	/**
	* Print the interface for changing edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function paintEditParams($prefix, $write_access=false)
	{
		$searchStyles = Array();
		$searchStyles['list'] = 'List';
		$searchStyles['checkbox'] = 'Checkbox';

		?>
		<table class="sq-backend-table">
			<tr>
				<td colspan="3" class="sq-backend-table-header"><?php echo $this->name; ?></td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading">Style</td>
				<td class="sq-backend-section-heading">Height</td>
				<td class="sq-backend-section-heading">Columns</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) combo_box($prefix.'_' . $this->name . '_style', $searchStyles, false, $this->_edit_params['style']);
					else echo $this->_edit_params['style'];
					?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_height', $this->_edit_params['height']);
					else echo $this->_edit_params['height'];
					?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_columns', $this->_edit_params['columns']);
					else echo $this->_edit_params['columns'];
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintEditParams()


	/**
	* Process the interface for changing edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return array
	* @access public
	*/
	function processEditParams($prefix)
	{
		// reading submitted vars
		if (!empty($_POST[$prefix.'_' . $this->name . '_style'])) {
			$this->_edit_params['style'] = $_POST[$prefix.'_' . $this->name . '_style'];
		}

		if (!empty($_POST[$prefix.'_' . $this->name . '_height'])) {
			$this->_edit_params['height'] = $_POST[$prefix.'_' . $this->name . '_height'];
		}

		if (!empty($_POST[$prefix.'_' . $this->name . '_columns'])) {
			$this->_edit_params['columns'] = $_POST[$prefix.'_' . $this->name . '_columns'];
		}

		$values = Array();
		$values['style'] = $this->_edit_params['style'];
		$values['height'] = $this->_edit_params['height'];
		$values['columns'] = $this->_edit_params['columns'];

		$result = Array();
		$result[$this->name] = $values;

		return $result;

	}//end processEditParams()


	/**
	* Convert an array to a | seperated string
	*
	* @param array	$array	the array to be converted
	*
	* @return string
	* @access private
	*/
	function _ar2str($array)
	{
		return implode('|',array_intersect(array_keys($this->_params['options']),$array));

	}//end _ar2str()


	/**
	* Convert a | seperated string to an array
	*
	* @param string	$string	the string to be converted
	*
	* @return array
	* @access private
	*/
	function _str2ar($string)
	{
		return array_intersect(array_keys($this->_params['options']),explode('|',$string));

	}//end _str2ar()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	* @param boolean	$add		if we should add the field name, or not (not - for search page)
	*
	* @access public
	*/
	function paint($prefix, $read_only=false, $add=true)
	{
		$options = $this->_params['options'];
		if (!empty($this->_params['multiple']) && !is_array($this->value)) $this->value = $this->_str2ar($this->value);

		if ($read_only) {
			if (empty($this->value)) return;

			if (empty($this->_params['multiple'])) {
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

		// add the hidden field only if working in backend (i.e. not on search page itself)
		if ($add) {
			hidden_field($prefix.'_'.$this->name.'_posted', '1');
		}

		if($this->_edit_params['style'] == 'list') {
			if ($add) {
				combo_box($prefix.'_'.$this->name, $options, !empty($this->_params['multiple']), $this->value, $this->_edit_params['height']);
			} else {
				combo_box($prefix, $options, !empty($this->_params['multiple']), $this->value, $this->_edit_params['height']);
			}
		} else {
			echo ('<table cellpadding="0" cellspacing="0" border="0"><tr>');
			reset($options);
			$i = 0;
			while(list($key,$val) = each($options)) {
				if(empty($this->_params['multiple'])) {
					?><td>&nbsp;<input type="radio" name="<?php 
						if ($add) {
							echo $prefix.'_'.$this->name;
						} else {
							echo $prefix;
						}
					?>" value="<?php echo $key?>"<?php
					if($key == $this->value) echo ' checked';
					?>><?php echo $val?></td><?php
				} else {
					?><td>&nbsp;<input type="checkbox" name="<?php 
						if ($add) {
							echo $prefix.'_'.$this->name."[$i]";
						} else { 
							echo $prefix . "[$i]";
						}
							?>" value="<?php echo $key?>"<?php
					if(is_array($this->value) && in_array($key, $this->value)) echo ' checked';
					?>><?php echo $val?></td><?php
				}
				if ((($i++) % $this->_edit_params['columns']) == ($this->_edit_params['columns'] - 1)) {
					echo "</tr><tr>" ;
				}
			}
			echo "</tr></table>" ;
		}

	}//end paint()


	/**
	* Print the interface for filling in a value
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

	}//end process()


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


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		return get_word_counts(strtolower($this->value));

	}//end getKeyData()


	/**
	* Returns an array of the current value with the individual selection values as keys 
	* and the display names as values
	*
	* @return array(scalar => scalar)
	* @access public
	*/
	function getValueArray()
	{
		$value = $this->_str2ar($this->value);
		$value_arr = Array();
		foreach($value as $k) $value_arr[$k] = $this->_params['options'][$k];
		return $value_arr;

	}//end getValueArray()


}//end class

?>
