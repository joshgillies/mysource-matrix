<?php

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Date/Time Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <datetime min="1970-01-01 00:00:00" max="2030-12-31 23:59:59" allow_circa="1">
*      <show>
*        <y style="t" allow_null="1" />
*        <m style="s" allow_null="1" />
*        <d style="s" allow_null="1" />
*        <h style="s" allow_null="1" />
*        <i style="s" allow_null="1" />
*        <s style="s" allow_null="1" />
*      </show>
*    </datetime>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage attributes
*/
class Asset_Attribute_DateTime extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_DateTime($attributeid=0, $value=null) 
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

		$this->_edit_params['min'] = empty($node->attributes['min']) ? '1970-01-01 00:00:00' : $node->attributes['min'];
		$this->_edit_params['max'] = empty($node->attributes['max']) ? '2030-12-31 23:59:59' : $node->attributes['max'];
		
		$this->_edit_params['allow_circa'] = empty($node->attributes['allow_circa']) ? '0' : $node->attributes['allow_circa'];
		
		// date formats for printing
		$this->_edit_params['print_format'] = empty($node->attributes['print_format']) ? 'l, F jS, Y H:i:s' : $node->attributes['print_format'];
		
		// what to print and how to show it (textbox or select box)
		// need to do some more intense looking around for these options :)
		$this->_edit_params['show']  = Array();
		$this->_edit_params['style'] = Array();
		foreach ($node->children as $c => $child) {
			if ($child->name == 'show') {
				foreach ($child->children as $i => $format) {
					$this->_edit_params['show'][] = $format->name;
					$this->_edit_params['style'][$format->name] = $format->attributes['style'];
					if (isset($format->attributes['allow_null']) && $format->attributes['allow_null']) {
						$this->_edit_params['null'][] = $format->name;
					}
				}
				break;
			}
		}

		// if nothing was found, define some defaults of our own
		if (empty($this->_edit_params['show'])) {
			$this->_edit_params['show'] = Array('y','m','d','h','i','s');
			foreach($this->_edit_params['show'] as $u) {
				$this->_edit_params['style'][$u] = 's';
			}
		}
		
		return true;
	}// end setEditParams()


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
		$field = new Datetime_Field($prefix.'_'.$this->name, $this->value, $this->_edit_params);

		if ($read_only) {
			echo $field->format();
			return;
		}

		$field->printField();
		
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
		$value = $this->value;
		$field = new Datetime_Field($prefix.'_'.$this->name, $value, $this->_edit_params);
		if ($field->processField()) {
			if ($this->value != $value && $this->setValue($value)) {
				$this->processed = true;
			} else {
				$this->processed = false;
			}
		} else {
			$this->processed = false;
		}
	}// end process()


	/**
	* Ensure the attribute being set is a valid date/time
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$ref = '';
		$orig_value = $value;
		$field = new Datetime_Field('blah', $ref, $this->_edit_params);
		$validated_value = $field->validateValue($value);
		return ($orig_value == $validated_value);
	}//end validateValue()


}//end class

?>
