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
* $Id: boolean.inc,v 1.21 2004/09/08 01:22:16 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Boolean Asset Attribute
*
* parameters => Array(
*                     'allow_empty' => true
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <boolean true_text="true" false_text="false" empty_text="maybe" />
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
		$this->Asset_Attribute($attributeid, $value);
		
		// set default parameters
		if (!isset($this->_params['allow_empty'])) $this->_params['allow_empty'] = true;
		
		// set default edit parameters
		$this->_edit_params['true_text']  = 'True';
		$this->_edit_params['false_text'] = 'False';
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
	{
		if (!parent::setEditParams($node)) return false;
		if (isset($node->attributes['true_text']))  $this->_edit_params['true_text']  = $node->attributes['true_text'];
		if (isset($node->attributes['false_text'])) $this->_edit_params['false_text'] = $node->attributes['false_text'];
		if (isset($node->attributes['empty_text'])) $this->_edit_params['empty_text'] = $node->attributes['empty_text'];
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
				<td class="sq-backend-section-heading" style="width: 100px;">True Text</td>	
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_'.$this->name.'_true', $this->_edit_params['true_text']);
					else echo $this->_edit_params['true_text'];
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;">False Text</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_'.$this->name.'_false', $this->_edit_params['false_text']);
					else echo $this->_edit_params['false_text'];
					?>
				</td>
			</tr>
			<?php
			if ($this->_params['allow_empty']) {
				?>
				<tr>
					<td class="sq-backend-section-heading" style="width: 100px;">Empty Text</td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) text_box($prefix.'_'.$this->name.'_empty', $this->_edit_params['empty_text']);
						else echo $this->_edit_params['empty_text'];
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
		if (!empty($_POST[$prefix.'_'.$this->name.'_true'])) {
			$this->_edit_params['true_text'] = $_POST[$prefix.'_'.$this->name.'_true'];
		}

		if (!empty($_POST[$prefix.'_'.$this->name.'_false'])) {
			$this->_edit_params['false_text'] = $_POST[$prefix.'_'.$this->name.'_false'];
		}
		
		if (!empty($_POST[$prefix.'_'.$this->name.'_empty'])) {
			$this->_edit_params['empty_text'] = $_POST[$prefix.'_'.$this->name.'_empty'];
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
		
		if ($this->_params['allow_empty'] && !empty($this->_edit_params['empty_text'])) {
			$options[''] = $this->_edit_params['empty_text'];
		}

		$options['1'] = $this->_edit_params['true_text'];
		$options['0'] = $this->_edit_params['false_text'];

		if ($read_only) {
			echo $options[$this->value];
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
		if (!isset($_POST[$prefix])) return false;
		$value = $_POST[$prefix];
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
		if ($this->_params['allow_empty'] && $value == '') return true;

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
		require_once SQ_FUDGE_PATH.'/general/text.inc';

		if ($this->value == 0) {
			return get_word_counts('false');
		} else if ($this->value == 1) {
			return get_word_counts('true');
		} else {
			return get_word_counts('empty');
		}

	}//end getKeyData()


}//end class

?>