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
* $Id: text.inc,v 1.21 2004/03/03 23:50:35 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
	*
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
		?>
		<table class="sq-backend-table">
			<tr>
				<td colspan="2" class="sq-backend-table-header"><?php echo $this->name; ?></td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading">Width</td>
				<td class="sq-backend-section-heading">Height</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_width', $this->_edit_params['width']);
					else echo $this->_edit_params['width'];
					?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_height', $this->_edit_params['height']);
					else echo $this->_edit_params['height'];
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
		if (!empty($_POST[$prefix.'_' . $this->name . '_width'])) {
			$this->_edit_params['width'] = $_POST[$prefix.'_' . $this->name . '_width'];
		}

		if (!empty($_POST[$prefix.'_' . $this->name . '_height'])) {
			$this->_edit_params['height'] = $_POST[$prefix.'_' . $this->name . '_height'];
		}

		$values = Array();
		$values['width'] = $this->_edit_params['width'];
		$values['height'] = $this->_edit_params['height'];

		$result = Array();
		$result[$this->name] = $values;
		
		return $result;

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
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes['width']))  $this->_edit_params['width']  = (int) $node->attributes['width'];
		if (isset($node->attributes['height'])) $this->_edit_params['height'] = (int) $node->attributes['height'];

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
	function paint($prefix, $read_only=false) // '$add' is missing. TO-DO: check why? (maybe on purpose)
	{
		if ($read_only) {
			echo $this->value;
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if($this->_edit_params['height'] > 1) {
			text_area($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_edit_params['height'], $this->_params['max_length']);
		} else {
			text_box($prefix.'_'.$this->name, $this->value, $this->_edit_params['width'], $this->_params['max_length']);
		}

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
		if (!isset($_POST[$prefix.'_'.$this->name])) return false;
		$value = (string) $_POST[$prefix.'_'.$this->name];
		if ($this->value != $value && $this->setValue($value)) {
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


}//end class

?>