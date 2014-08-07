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
* $Id: url.inc,v 1.10 2012/08/30 01:09:18 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* URL Attribute
*
* <!-- Sample Edit Interface XML Node -->
*    <url width="40" protocols="http,https,ftp,mailto" />
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Url extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Url($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value);

		// set default edit parameters
		$this->_edit_params['width']  = 0;
		$this->_edit_params['protocols'] = Array('http');

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
		echo '<b>URL Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('width'); ?></td>
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
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('protocols'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_'.$this->name.'_height', implode(',', $this->_edit_params['protocols']));
					} else {
						echo $this->_edit_params['protocols'];
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

		if (!empty($_POST[$prefix.'_'.$this->name.'_protocols'])) {
			$protocols = str_replace(' ', '', $_POST[$prefix.'_'.$this->name.'_protocols']);
			$this->_edit_params['protocols'] = explode(',', $protocols);
		}

		$values = Array();
		$values['width']  = $this->_edit_params['width'];
		$values['protocols'] = $this->_edit_params['protocols'];

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
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes()->width)) {
			$this->_edit_params['width']  = (int) $node->attributes()->width;
		}
		if (isset($node->attributes()->protocols)) {
			$protocols = str_replace(' ', '', (string)$node->attributes()->protocols);
			$this->_edit_params['protocols'] = explode(',', $protocols);
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
			echo htmlspecialchars(nl2br($this->value));
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		url_tester($prefix, $this->value, $this->_edit_params['protocols'], $this->_edit_params['width']);

		return true;

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
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		if (!isset($_REQUEST[$prefix.'_url'])) return false;
		if (!isset($_REQUEST[$prefix.'_protocol'])) return false;

		$value = get_url_tester_url($prefix);
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


}//end class

?>
