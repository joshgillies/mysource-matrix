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
* $Id: text.inc,v 1.38 2012/08/30 01:09:18 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';


/**
* Text Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <text width="40" height="4" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Text extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*/
	function Asset_Attribute_Text($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value, Array('max_length' => 0));

		// set default edit parameters
		$this->_edit_params['width']  = 0;
		$this->_edit_params['height'] = 1;

	}//end constructor


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
		echo '<b>Text Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('Width'); ?></td>

				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_'.$this->name.'_width', $this->_edit_params['width']);
					} else {
						echo $this->_edit_params['width'];
					}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('Height'); ?></td>

				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_'.$this->name.'_height', $this->_edit_params['height']);
					} else {
						echo $this->_edit_params['height'];
					}
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
		if (!empty($_POST[$prefix.'_'.$this->name.'_width'])) {
			$this->_edit_params['width'] = $_POST[$prefix.'_'.$this->name.'_width'];
		}

		if (!empty($_POST[$prefix.'_'.$this->name.'_height'])) {
			$this->_edit_params['height'] = $_POST[$prefix.'_'.$this->name.'_height'];
		}

		$values = Array();
		$values['width']  = $this->_edit_params['width'];
		$values['height'] = $this->_edit_params['height'];

		return $values;

	}//end processEditParams()


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
	{// TODO TOF TEST
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes()->width)) {
			$this->_edit_params['width']  = (int) $node->attributes()->width;
		}
		if (isset($node->attributes()->height)) {
			$this->_edit_params['height'] = (int) $node->attributes()->height;
		}

		return true;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			echo nl2br(htmlspecialchars($this->value));
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if ($this->_edit_params['height'] > 1) {
			text_area($prefix, $this->value, $this->_edit_params['width'], $this->_edit_params['height'], $this->_params['max_length']);
		} else {
			text_box($prefix, $this->value, $this->_edit_params['width'], $this->_params['max_length']);
		}

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix])) return false;
		$value = (string) $_REQUEST[$prefix];
		if ($this->value !== $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a string
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$new_value = (string) $value;
		if ($new_value != $value) return false;
		$value = $new_value;
		return true;

	}//end validateValue()


}//end class

?>
