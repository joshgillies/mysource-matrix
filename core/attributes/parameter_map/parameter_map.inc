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
* $Id: parameter_map.inc,v 1.1 2004/11/26 03:34:53 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once dirname(dirname(__FILE__)).'/serialise/serialise.inc';

/**
* Parameter_Map asset attribute
*
*   Attributes of this type store and allow for the manipulation of mappings
*   from runtime variable values (GET, POST etc) to certain parameters of
*   an asset.  When declaring the attribute in the asset's management class
*   you need to pass a parameter called 'parameters', which is a 
*   code => full_name array of the parameters you want to make mappable.
*   When requesting the runtime value for a parameter you give its code.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Parameter_Map extends Asset_Attribute_Serialise
{

	/**
	* The data sources available to get values from
	*
	* @var Array
	*/
	var $_source_types = Array(	'GET'	=> 'GET',
								'POST'	=> 'POST',
								'ASSET'	=> 'Current Asset');

	/**
	* The options available under the ASSET source type
	*
	* @var Array
	*/
	var $_asset_attribute_options = Array(	'name'			=> 'Name',
											'short_name'	=> 'Short Name',
											'assetid'		=> 'Asset ID');


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Parameter_Map($attributeid=0, $value=null) 
	{
		if (!isset($this->_params['parameters'])) $this->_params['parameters'] = Array();
		if ($value == null) $value = Array();
		$this->Asset_Attribute_Serialise($attributeid, $value);

	}// end constructor


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
		$value = unserialize($this->value);
		if (!$read_only) {
			?>
			<script language="JavaScript">
			<!--
			function updateFieldChooser(dataSource)
			{
			  if (dataSource.options[dataSource.selectedIndex].value != 'ASSET') {
				//alert(dataSource.name.substr(0, dataSource.name.lastIndexOf('['))+'[attribute]');
				var selectElts = document.getElementsByName(dataSource.name.substr(0, dataSource.name.lastIndexOf('['))+'[attribute]');
				selectElts[0].style.display = 'none';
				var inputElts = document.getElementsByName(dataSource.name.substr(0, dataSource.name.lastIndexOf('['))+'[index]');
				inputElts[0].style.display = 'inline';
			  } else {
				var selectElts = document.getElementsByName(dataSource.name.substr(0, dataSource.name.lastIndexOf('['))+'[attribute]');
				selectElts[0].style.display = 'inline';
				var inputElts = document.getElementsByName(dataSource.name.substr(0, dataSource.name.lastIndexOf('['))+'[index]');
				inputElts[0].style.display = 'none';
			  }
			}
			//-->
			</script>
			<?php
		}
		?>
		<table border="0" cellpadding="0" cellspacing="0" class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">Parameter</td>
				<td class="sq-backend-table-header">Source</td>
			</tr>
		<?php

		foreach ($value as $parameter => $sources) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo $this->_params['parameters'][$parameter]; ?></td>
				<td class="sq-backend-table-cell">
					<?php 
						foreach ($sources as $index => $details) {
							if (!$read_only) {
								$this->_printSourceChooser($prefix.'['.$parameter.']['.$index.']', $details['source'], ($details['source'] != 'ASSET') ? $details['index'] : $details['attribute']);
							} else {
								echo $this->_source_types[$details['source']];
								if ($details['source'] == 'ASSET') echo ' '.$details['attribute'];
								else echo '['.$details['index'].']';
								echo '<br />';
							}
						}
					?>
				</td>
			</tr>
			<?php
		}
		if (!$read_only) {
		?>

			<tr>
				<td class="sq-backend-table-cell"><?php $this->_printParameterChooser($prefix.'[new][0]'); ?></td>
				<td class="sq-backend-table-cell"><?php $this->_printSourceChooser($prefix.'[new][0]'); ?></td>
			</tr>
		<?php
		}
		?>
		</table>
		<?php
		
	}//end paint()


	/**
	* Paint the fields to choose a data source
	*
	* @param string		$prefix			The prefix for the fields' names
	* @param string		$source_type	The current value for the source-type field
	* @param string		$component		The current value for the array-index or asset-attribute field
	*
	* @access private
	* @return void
	*/
	function _printSourceChooser($prefix, $source_type=null, $component=null)
	{
		combo_box($prefix.'[source]', $this->_source_types, false, Array($source_type), 0, 'onchange="updateFieldChooser(this)" onkeyup="updateFieldChooser(this)"');
		echo '&nbsp;';
		text_box($prefix.'[index]', (($source_type == 'ASSET') ? '' : $component), '', '', false, 'style="width: 25ex; '.(($source_type == 'ASSET') ? 'display: none' : '').'"'); 
		combo_box($prefix.'[attribute]', $this->_asset_attribute_options, false, Array($component), 0, 'style="width: 25ex; '.(($source_type != 'ASSET') ? 'display: none' : '').'"'); 
		if ($source_type != null) {
			check_box($prefix.'[delete]');
			echo '<label for="'.$prefix.'[delete]">Delete</label>';
		}
		echo '<br />';

	}//end _printSourceChooser()

	
	/**
	* Paints the field to choose a parameter for a new mapping
	*
	* @param string		$prefix		The prefix for the field's name
	*
	* @access private
	* @return void
	*/
	function _printParameterChooser($prefix)
	{
		combo_box($prefix.'[parameter]', Array(''=>'-- New Mapping --')+$this->_params['parameters'], false, Array());

	}//end _printParameterChooser()


	/**
	* Prints the interface for filling in a value
	*
	* @param object Asset	$asset	the asset that owns the aprefix for the form element
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$value = Array();
		foreach ($_POST[$prefix] as $index => $details) {
			if ($index == 'new') continue;
			$value[$index] = $details;
			foreach ($details as $j => $fields) {
				if ($fields['source'] == 'ASSET') unset($value[$index][$j]['index']);
				else unset($value[$index][$j]['attribute']);
				if (isset($fields['delete'])) {
					unset($value[$index][$j]);
					if (empty($value[$index])) unset($value[$index]);
				}
			}
		}
		foreach ($_POST[$prefix]['new'] as $new_mapping) {
			if (empty($new_mapping['parameter'])) continue;
			$param = $new_mapping['parameter'];
			unset($new_mapping['parameter']);
			if ($new_mapping['source'] == 'ASSET') unset($new_mapping['index']);
			else unset($new_mapping['attribute']);
			if (!isset($value[$param])) {
				$value[$param] = Array($new_mapping);
			} else {
				$value[$param][] = $new_mapping;
			}
		}
		$this->validateValue($value);
		$this->processed = $this->setValue($value);

	}//end process()


	/**
	* Get the runtime value for the specified parameter
	*
	* @param string	$parameter	The code of the parameter we want the value for
	*
	* @access public
	* @return mixed		null if no value found
	*/
	function getParameterValue($parameter)
	{
		$value = unserialize($this->value);
		if (!isset($value[$parameter])) return null;
		foreach ($value[$parameter] as $rule) {
			switch ($rule['source']) {
				case 'GET':
					if (isset($_GET[$rule['index']])) {
						return $_GET[$rule['index']];
					}
				break;
				
				case 'POST':
					if (isset($_POST[$rule['index']])) {
						return $_POST[$rule['index']];
					}
				break;
				
				case 'ASSET':
					switch ($rule['attribute']) {
						case 'assetid':
							return $GLOBALS['SQ_SYSTEM']->frontend_asset->id;
							break;
						case 'name':
						case 'short_name':
							return $GLOBALS['SQ_SYSTEM']->frontend_asset->attr($rule['attribute']);
							break;
					}
				break;
			}
		}
		return null;

	}//end getParameterValue()


}//end class

?>