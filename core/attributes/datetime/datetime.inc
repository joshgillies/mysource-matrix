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
* $Id: datetime.inc,v 1.15 2004/05/07 01:22:31 lwright Exp $
* $Name: not supported by cvs2svn $
*/


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
* @package MySource_Matrix
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

		// default values
		$this->_edit_params['max']          = '2030-12-31 23:59:59';
		$this->_edit_params['min']          = '1970-01-01 00:00:00';
		$this->_edit_params['allow_circa']  = '0';
		$this->_edit_params['print_format'] = 'l, F jS, Y H:i:s';

	}//end constructor


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

		if (!empty($node->attributes['min'])) $this->_edit_params['min'] = $node->attributes['min'];
		if (!empty($node->attributes['max'])) $this->_edit_params['max'] = $node->attributes['max'];
		if (!empty($node->attributes['allow_circa'])) $this->_edit_params['allow_circa'] = $node->attributes['allow_circa'];
		if (!empty($node->attributes['print_format'])) $this->_edit_params['print_format'] = $node->attributes['print_format'];
		
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

	}//end setEditParams()


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
		if (isset($this->_params['print_format'])) {
			$this->_edit_params['print_format'] = $this->_params['print_format'];
		}
		$field = new Datetime_Field($prefix.'_'.$this->name, $this->value, $this->_edit_params);

		if ($read_only) {
			echo $field->format();
			return;
		}

		$field->printField();
		
	}//end paint()


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

	}//end process()


	/**
	* Get the current value as a unix timestamp
	*
	* @return int
	* @access public
	*/
	function getTimestamp()
	{
		$field = new Datetime_Field('blah', $this->value, $this->_edit_params);
		$units = $field->strToUnitsArray($this->value);
		foreach(Array('y', 'm', 'd') as $u) if ($units[$u] == -1) return -1;

		// now we know that we can formulate a date
		$strtotime = $units['m'].'/'.$units['d'].'/'.$units['y'];

		$dotime = true;
		foreach(Array('h', 'i', 's') as $u) if ($units[$u] == -1) $dotime = false;
		if ($dotime) $strtotime .= ' '.$units['h'].':'.$units['i'].':'.$units['s'];

		return strtotime($strtotime);

	}//end getTimestamp()


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
