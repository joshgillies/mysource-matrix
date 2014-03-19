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
* $Id: boolean.inc,v 1.38 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Boolean Asset Attribute Type
*
* The raw value of attributes of this type is '1' for true, '0' for false or '' for empty
*
* parameters => Array(
*                     'allow_empty' => [true|false]
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <boolean true_text="true" false_text="false" empty_text="maybe" />
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.38 $
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
		$this->Asset_Attribute($attributeid, $value);

		// set default parameters
		if (!isset($this->_params['allow_empty'])) {
			$this->_params['allow_empty'] = true;
		}

		// set default edit parameters
		$this->_edit_params['true_text']  = translate('true');
		$this->_edit_params['false_text'] = translate('false');
		$this->_edit_params['empty_text'] = '';

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{// TODO TOF TEST
		if (!parent::setEditParams($node)) return false;
		if (isset($node->attributes()->true_text)) {
			$this->_edit_params['true_text']  = translate((string)$node->attributes()->true_text);
		}
		if (isset($node->attributes()->false_text)) {
			$this->_edit_params['false_text'] = translate((string)$node->attributes()->false_text);
		}
		if (isset($node->attributes()->empty_text)) {
			$this->_edit_params['empty_text'] = translate((string)$node->attributes()->empty_text);
		}
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
		echo '<b>Boolean Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('true_text'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_true', $this->_edit_params['true_text']);
					} else {
						echo $this->_edit_params['true_text'];
					}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('false_text'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_false', $this->_edit_params['false_text']);
					} else {
						echo $this->_edit_params['false_text'];
					}
					?>
				</td>
			</tr>
			<?php
			if ($this->_params['allow_empty']) {
				?>
				<tr>
					<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('empty_text'); ?></td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							text_box($prefix.'_empty', $this->_edit_params['empty_text']);
						} else {
							echo $this->_edit_params['empty_text'];
						}
						?>
					</td>
				</tr>
				<?php
			}
			?>
		</table>
		<?php

	}//end paintEditParams()


	/**
	* Process the interface for changig edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return array
	* @access public
	*/
	function processEditParams($prefix)
	{
		if (!empty($_POST[$prefix.'_true'])) {
			$this->_edit_params['true_text'] = $_POST[$prefix.'_true'];
		}

		if (!empty($_POST[$prefix.'_false'])) {
			$this->_edit_params['false_text'] = $_POST[$prefix.'_false'];
		}

		if (!empty($_POST[$prefix.'_empty'])) {
			$this->_edit_params['empty_text'] = $_POST[$prefix.'_empty'];
		}

		$values = Array();
		$values['true_text']  = $this->_edit_params['true_text'];
		$values['false_text'] = $this->_edit_params['false_text'];
		$values['empty_text'] = $this->_edit_params['empty_text'];

		return $values;

	}//end processEditParams()


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
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		$options = Array();

		if ($this->_params['allow_empty']) {
			if (empty($this->_edit_params['empty_text'])) {
				$this->_edit_params['empty_text'] = '-- Leave Empty --';
			}
			$options[''] = $this->_edit_params['empty_text'];
		}

		$options['1'] = $this->_edit_params['true_text'];
		$options['0'] = $this->_edit_params['false_text'];

		if ($read_only) {
			echo $options[(string)$this->value];
			return;
		}

		combo_box($prefix, $options, false, $this->value);

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
		if (!isset($_REQUEST[$prefix])) return false;
		$value = $_REQUEST[$prefix];
		if ($this->value !== $value && $this->setValue($value)) {
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
		if (array_get_index($this->_params, 'allow_empty', false) && is_string($value) && $value == '') {
			return true;
		}
		$value = (int) $value;
		$new_value = (int) $value;
		if ((string) $new_value != (string) $value) {
			// check for strings that mean the same as true and false
			switch (strtolower((string) $value)) {
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
	* Returns the raw content for this attribute type
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		if ($this->value == 0) {
			return 'false';
		} else if ($this->value == 1) {
			return 'true';
		} else {
			return 'empty';
		}

	}//end getContent()


}//end class

?>
