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
* $Id: boolean.inc,v 1.11.2.2 2004/03/16 16:55:34 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Boolean Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <boolean true_text="true" false_text="false" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Boolean extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Boolean($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, (($value) ? 1 : 0));
		
		// set default edit parameters
		$this->_edit_params['true_text']  = 'True';
		$this->_edit_params['false_text'] = 'False';

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
		$this->_edit_params['true_text']  = empty($node->attributes['true_text'])  ? 'True' : $node->attributes['true_text'];
		$this->_edit_params['false_text'] = empty($node->attributes['false_text']) ? 'False' : $node->attributes['false_text'];
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
		?>
		<table class="sq-backend-table">
			<tr>
				<td colspan="2" class="sq-backend-table-header"><?php echo $this->name; ?></td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading">True text</td>
				<td class="sq-backend-section-heading">False text</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_true', $this->_edit_params['true_text']);
					else echo $this->_edit_params['true_text'];
					?>
				</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_' . $this->name . '_false', $this->_edit_params['false_text']);
					else echo $this->_edit_params['false_text'];
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
		if (!empty($_POST[$prefix.'_' . $this->name . '_true'])) {
			$this->_edit_params['true_text'] = $_POST[$prefix.'_' . $this->name . '_true'];
		}

		if (!empty($_POST[$prefix.'_' . $this->name . '_false'])) {
			$this->_edit_params['false_text'] = $_POST[$prefix.'_' . $this->name . '_false'];
		}

		$values = Array();
		$values['true_text'] = $this->_edit_params['true_text'];
		$values['false_text'] = $this->_edit_params['false_text'];

		$result = Array();
		$result[$this->name] = $values;
		
		return $result;

	}//end processEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false, $add=true)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		if ($add) {
			$options = Array(0 => $this->_edit_params['false_text'],
							 1 => $this->_edit_params['true_text']
							);
		} else {
			$options = Array('_' => '-- Leave Empty --',
							 0 => $this->_edit_params['false_text'],
							 1 => $this->_edit_params['true_text']
							);
		}

		if ($read_only) {
			echo $options[$this->value];
			return;
		}

		if ($add) {
			combo_box($prefix.'_'.$this->name, $options, false, $this->value, 0, $extras='', $maxlength=0);
		} else {
			combo_box($prefix, $options, false, null, 0, $extras='', $maxlength=0);
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
		$value = $_POST[$prefix.'_'.$this->name];
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
		$new_value = (int) $value;
		if ((string) $new_value != (string) $value) {
			// check for strings that mean the same as true and false
			switch(strtolower((string) $value)) {
				case 'true' :
				case 't'    :
				case 'yes'  :
				case 'y'    :
					$new_value = 1;
					break;

				case 'false' :
				case 'f'     :
				case 'no'    :
				case 'n'     :
					$new_value = 0;
					break;
			}
		}
		$value = (empty($new_value)) ? 0 : 1;
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
		//return get_word_counts($this->value); // -- NOTE: int values never get indexed, only 'words' !!!

		if ($this->value == 0) {
			return get_word_counts('false');
		} else {
			return get_word_counts('true');
		}

	}//end getKeyData()


}//end class

?>