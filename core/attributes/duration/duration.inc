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
* $Id: duration.inc,v 1.2 2005/03/14 01:24:54 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Duration Asset Attribute
*
*
*
* <!-- Sample Edit Interface XML Node -->
*	<duration biggest_units="days" smallest_units="minutes"/>
*	Both parameters can be days, hours, minutes or seconds.
*
* @author	Dmitry Baranovskiy	<dbaranovskiy@squiz.net>
* @author	Tom Barrett			<tbarrett@squiz.net>
*
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Duration extends Asset_Attribute
{


	var $units = Array('days' => 86400, 'hours' => 3600, 'minutes' => 60, 'seconds' => 1);


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Duration($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, (int) $value);

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
		$biggest	=  $this->units[$node->attributes['biggest_units']];
		$smallest	= $this->units[$node->attributes['smallest_units']];
		$this->_edit_params['biggest_units']	= max($biggest, $smallest);
		$this->_edit_params['smallest_units']	= min($biggest, $smallest);
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
		$biggest_units	= array_get_index($this->_edit_params, 'biggest_units', $this->units['days']);
		$smallest_units	= array_get_index($this->_edit_params, 'smallest_units', $this->units['minutes']);

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$remaining_value = $this->value;
		foreach ($this->units as $current_name => $current_units) {
			if ($current_units >= $smallest_units && $current_units <= $biggest_units) {
				$current_value = (int) ($remaining_value / $current_units);
				$remaining_value -= $current_value * $current_units;
				if ($smallest_units != $biggest_units) {
					if ($current_units == $smallest_units) {
						echo ' and ';
					} elseif ($current_units != $biggest_units) {
						echo ', ';
					}
				}
				if ($read_only) {
					echo ' '.$current_value.' '.$current_name;
				} else {
					text_box($prefix.'_'.$current_name, $current_value, 2, 0, false, ' onkeypress="return checkDurationValue(this, event)" onblur="updateDurationValues(this)" style="width:4ex"');
					echo '<label for="'.$prefix.'_'.$current_name.'"> '.$current_name.'</label>';
				}
			}
		}

	}//end paint()


	/**
	* Processes the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$biggest_units	= array_get_index($this->_edit_params, 'biggest_units', $this->units['days']);
		$smallest_units	= array_get_index($this->_edit_params, 'smallest_units', $this->units['minutes']);

		if (!isset($_REQUEST[$prefix.'_'.array_search($biggest_units, $this->units)])) return false;

		$res = 0;
		foreach ($this->units as $name => $number) {
			$res += array_get_index($_REQUEST, $prefix.'_'.$name, 0) * $number;
		}

		$value = (int) $res;
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is an integer
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$value = trim($value);


		if ($value < 0) {
			$value = $this->_default_value;
		}

		$new_value = (int)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return false;
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
		if (empty($this->value)) return Array();
		return Array($this->value => 1);

	}//end getKeyData()


}//end class

?>
