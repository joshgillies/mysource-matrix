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
* $Id: duration.inc,v 1.11 2012/08/30 01:09:17 ewang Exp $
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
* @version $Revision: 1.11 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Duration extends Asset_Attribute
{


	var $units = Array(
					'days'		=> 86400,
					'hours'		=> 3600,
					'minutes'	=> 60,
					'seconds'	=> 1,
				 );


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Duration($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute($attributeid, (int) $value);

		// Default display.
		$this->_edit_params['biggest_units'] = 86400;
		$this->_edit_params['smallest_units'] = 60;

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	an XML_Tree_Node from an edit interface XML file
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{// TODO TOF TO TEST
		if (!parent::setEditParams($node)) return FALSE;
		$biggest	=  $this->units[(string)$node->attributes()->biggest_units];
		$smallest	= $this->units[(string)$node->attributes()->smallest_units];
		$this->_edit_params['biggest_units']	= max($biggest, $smallest);
		$this->_edit_params['smallest_units']	= min($biggest, $smallest);
		return TRUE;

	}//end setEditParams()


	/**
	* Returns an array of the current units and their actual values
	*
	* @return array
	* @access public
	*/
	function getUnitValues()
	{
		$unit_values = Array();
		$biggest_units	= array_get_index($this->_edit_params, 'biggest_units', $this->units['days']);
		$smallest_units	= array_get_index($this->_edit_params, 'smallest_units', $this->units['minutes']);
		$remaining_value = $this->value;
		foreach ($this->units as $current_name => $current_units) {
			if ($current_units >= $smallest_units && $current_units <= $biggest_units) {
				$current_value = (int) ($remaining_value / $current_units);
				$unit_values[$current_name] = $current_value;
				$remaining_value -= $current_value * $current_units;
			}
		}
		return $unit_values;

	}//end getUnitValues()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return NULL
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$biggest_units	= array_get_index($this->_edit_params, 'biggest_units', $this->units['days']);
		$smallest_units	= array_get_index($this->_edit_params, 'smallest_units', $this->units['minutes']);

		// moves smallest unit to the lowest unit above the smallest unit that has a value
		if ($read_only) {
			$all_values = array_reverse($this->getUnitValues(), TRUE);
			$set = FALSE;
			foreach ($all_values as $unit_name => $value) {
				if ($set) $smallest_units = $this->units[$unit_name];
				if ($this->units[$unit_name] == $smallest_units) {
					if ($value == 0) {
						$set = TRUE;
					} else {
						break;
					}
				}
			}
		}
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$remaining_value = $this->value;
		$last_value = 0;
		$printed_any = FALSE;
		echo '<span class="sq-inline-fields-wrapper">';
		foreach ($this->units as $current_name => $current_units) {
			if ($current_units >= $smallest_units && $current_units <= $biggest_units) {
				$current_value = (int) ($remaining_value / $current_units);
				$remaining_value -= $current_value * $current_units;

				// modified below to only print duration units that have a value other than 0 when read-only is set
				if ($smallest_units != $biggest_units) {
					if (!$read_only) {
						if ($current_units == $smallest_units) {
							echo ' and ';
						} else if ($current_units != $biggest_units) {
							echo ', ';
						}
					} else {
						if ($current_value != 0) {
							if ($current_units == $smallest_units && $printed_any) {
								echo ' and ';
							} else if ($current_units != $biggest_units && $printed_any) {
								echo ', ';
							}
						}
					}
				}
				if ($read_only) {
					if ($current_value != 0) {
						if ($current_value == 1) {
							// If singular, strip the plural 's' off the name
							$current_name = substr($current_name, 0, -1);
						}
						echo ' '.$current_value.' '.$current_name;
						$printed_any = TRUE;
					}
				} else {
					text_box($prefix.'_'.$current_name, $current_value, 2, 0, FALSE, ' onkeypress="return checkDurationValue(this, event)" onblur="updateDurationValues(this)" style="width:4ex"');
					echo '<label for="'.$prefix.'_'.$current_name.'"> '.$current_name.'</label>';
					$printed_any = TRUE;
				}
			}//end if
		}//end foreach unit
		echo '</span>';

	}//end paint()


	/**
	* Processes the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return NULL
	* @access public
	*/
	function process($prefix)
	{
		$biggest_units	= array_get_index($this->_edit_params, 'biggest_units', $this->units['days']);
		$smallest_units	= array_get_index($this->_edit_params, 'smallest_units', $this->units['minutes']);

		if (!isset($_REQUEST[$prefix.'_'.array_search($biggest_units, $this->units)])) {
			return FALSE;
		}

		$res = 0;
		foreach ($this->units as $name => $number) {
			$res += array_get_index($_REQUEST, $prefix.'_'.$name, 0) * $number;
		}

		$value = (int) $res;
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = TRUE;
		} else {
			$this->processed = FALSE;
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


		if ($value < 0) $value = $this->_default_value;

		$new_value = (int)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return FALSE;
		return TRUE;

	}//end validateValue()


	/**
	* Returns a representation of this asset's parameters, edit parameters and value as XML
	*
	* @return string
	* @access public
	*/
	function exportContentsToXML()
	{
		$value_array = $this->getUnitValues();
		$value_xml = '';
		foreach ($value_array as $unit => $unit_value) {
			$value_xml .= '<'.$unit.'><![CDATA['.$unit_value.']]></'.$unit.'>';
		}
		$xml = '<value>'.$value_xml.'</value>';

		return $xml;

	}//end exportContentsToXML()


}//end class

?>
