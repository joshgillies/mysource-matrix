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
* $Id: selection.inc,v 1.31 2004/09/15 07:05:30 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Selection Asset Attribute
*
* parameters => Array(
*                     'options'     => Array('value' => 'text name')
*                     'multiple'    => [true|false]
*                     'allow_empty' => [true|false]
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*   <selection style="list"  height="1" empty_text="" />
*   <selection style="boxes" columns="1" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Selection extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Selection($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);

		// set default parameters
		if (!isset($this->_params['options']))     $this->_params['options']     = Array();
		if (!isset($this->_params['multiple']))    $this->_params['multiple']    = false;
		if (!isset($this->_params['allow_empty'])) $this->_params['allow_empty'] = true;

		// set default edit parameters
		$this->_edit_params['style']      = 'list';
		$this->_edit_params['height']     = '';
		$this->_edit_params['columns']    = '1';
		$this->_edit_params['empty_text'] = '';

	}//end constructor


	/**
	* Take an XML_Tree_Node from an edit interface XML file and create the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes['style'])) $this->_edit_params['style'] = $node->attributes['style'];
		if (isset($node->attributes['height'])) $this->_edit_params['height'] = $node->attributes['height'];
		if (isset($node->attributes['columns'])) $this->_edit_params['columns'] = $node->attributes['columns'];
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
		$searchStyles = Array();
		$searchStyles['list'] = 'List';
		$searchStyles['checkbox'] = 'Checkbox';
		
		if (!empty($this->name)) {
			echo '<b>Selection Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		}
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;">Style</td>	
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) combo_box($prefix.'_style', $searchStyles, false, $this->_edit_params['style']);
					else echo $this->_edit_params['style'];
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;">Height</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_height', $this->_edit_params['height']);
					else echo $this->_edit_params['height'];
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;">Columns</td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_columns', $this->_edit_params['columns']);
					else echo $this->_edit_params['columns'];
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
						if ($write_access) text_box($prefix.'_empty', $this->_edit_params['empty_text']);
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
	* Process the interface for changing edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return array
	* @access public
	*/
	function processEditParams($prefix)
	{
		if (!empty($_POST[$prefix.'_style'])) {
			$this->_edit_params['style'] = $_POST[$prefix.'_style'];
		}

		if (!empty($_POST[$prefix.'_height'])) {
			$this->_edit_params['height'] = $_POST[$prefix.'_height'];
		}

		if (!empty($_POST[$prefix.'_columns'])) {
			$this->_edit_params['columns'] = $_POST[$prefix.'_columns'];
		}
		
		if (!empty($_POST[$prefix.'_empty'])) {
			$this->_edit_params['empty_text'] = $_POST[$prefix.'_empty'];
		}

		$values = Array();
		$values['style']      = $this->_edit_params['style'];
		$values['height']     = $this->_edit_params['height'];
		$values['columns']    = $this->_edit_params['columns'];
		$values['empty_text'] = $this->_edit_params['empty_text'];

		return $values;

	}//end processEditParams()


	/**
	* Convert an array to a | seperated string
	*
	* @param array	$array	the array to be converted
	*
	* @return string
	* @access private
	*/
	function _ar2str($array)
	{
		return implode('|', array_intersect(array_keys($this->_params['options']), $array));

	}//end _ar2str()


	/**
	* Convert a | seperated string to an array
	*
	* @param string	$string	the string to be converted
	*
	* @return array
	* @access private
	*/
	function _str2ar($string)
	{
		return array_intersect(array_keys($this->_params['options']), explode('|', $string));

	}//end _str2ar()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$options = $this->_params['options'];
		if ($this->_params['allow_empty'] && !$this->_params['multiple']) {
			if (empty($this->_edit_params['empty_text'])) $this->_edit_params['empty_text'] = '-- Leave Empty --';
			$options = array_reverse($options);
			$options[''] = $this->_edit_params['empty_text'];
			$options = array_reverse($options);
		}

		if (!empty($this->_params['multiple']) && !is_array($this->value)) {
			$value = $this->_str2ar($this->value);
		} else { 
			$value = $this->value;
		}

		if ($read_only) {
			if (empty($this->_params['multiple'])) {
				if (!isset($options[$value])) {
					// may have a value left over from when we
					// were a mutiple selection field
					$value = $this->_str2ar($value);
					if (!isset($options[$value[0]])) return;
					echo $options[$value[0]];
				} else {
					echo $options[$value];
				}

			} else {
				echo '<ul>';
				foreach ($value as $k) {
					if (!isset($options[$k])) continue;
					echo '<li class="sq-backend-data">'.$options[$k].'</li>';
				}
				echo '</ul>';

			}
			return;
		}
		
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		// just have this so that we can check whether this field was posted or not
		// because checkboxes don't have an entry in the $_POST array when they are not selected
		hidden_field($prefix.'_posted', '1');

		if ($this->_edit_params['style'] == 'list') {
			combo_box($prefix, $options, !empty($this->_params['multiple']), $value, $this->_edit_params['height']);
		} else {
			?>
			<table cellpadding="0" cellspacing="0" border="0">
				<tr>
					<?php
					$i = 0;
					while (list($key, $val) = each($options)) {
						if (empty($this->_params['multiple'])) {
							?>
							<td>&nbsp;
								<input type="radio" name="<?php echo $prefix; ?>" value="<?php echo $key; ?>"<?php echo (($key == $value) ? ' checked' : ''); ?>>
								<?php echo $val?>
							</td>
							<?php
						} else {
							?>
							<td>&nbsp;
								<input type="checkbox" name="<?php echo $prefix."[$i]"; ?>" value="<?php echo $key; ?>"<?php echo ((is_array($value) && in_array($key, $value)) ? ' checked' : ''); ?>>
								<?php echo $val?>
							</td>
							<?php

						}
						if ((($i++) % $this->_edit_params['columns']) == ($this->_edit_params['columns'] - 1)) {
							echo '</tr><tr>';
						}
					}
					?>
				</tr>
			</table>
			<?php
		}//end if style is list

	}//end paint()


	/**
	* Print the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/general.inc';
		if (!isset($_REQUEST[$prefix.'_posted'])) return false;
		$value = (isset($_REQUEST[$prefix])) ? $_REQUEST[$prefix] : '';

		if (is_array($value)) $value = $this->_ar2str($value);

		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a valid selection
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$options = $this->_params['options'];

		if (is_array($value)) $value = $this->_ar2str($value);
		$value = trim((string) $value);
		if ($value == '') return true;

		// special case for multiple values
		if (empty($this->_params['multiple'])) {
			if (!isset($options[$value])) return false;
		} else {
			$new_value = $this->_str2ar($value);
			if (!is_array($new_value)) return false;
			$return_value = array();
			foreach ($new_value as $key) {
				if (isset($options[$key])) $return_value[] = $key;
			}
			$value = $this->_ar2str($return_value);
		}
		
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


	/**
	* Returns an array of the current value with the individual selection values as keys 
	* and the display names as values
	*
	* @return array(scalar => scalar)
	* @access public
	*/
	function getValueArray()
	{
		$value = $this->_str2ar($this->value);
		$value_arr = Array();
		foreach($value as $k) $value_arr[$k] = $this->_params['options'][$k];
		return $value_arr;

	}//end getValueArray()


}//end class

?>
