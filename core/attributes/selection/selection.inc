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
* $Id: selection.inc,v 1.52 2006/01/26 23:39:31 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

/**
* Selection Asset Attribute
*
* For attributes of this type users must choose from the listed options.
* If "allow_empty" is enabled they may leave all options unchosen
* If "multiple" is enabled they may choose many options
* If "multiple" is disabled and "allow_other" is enabled they may choose an "other" option and enter a value manually
* NOTE: the "other" option is not available with multiple-selection fields
*
* parameters => Array(
*                     'options'     => Array('value' => 'text name')
*                     'multiple'    => [true|false]
*                     'allow_empty' => [true|false]
*					  'allow_other' => [true|false]
*               )
*
*
* <!-- Sample Edit Interface XML Node -->
*   <selection style="list"  height="1" empty_text="" extras="" />
*   <selection style="boxes" columns="1" extras="" />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.52 $
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
		if (!isset($this->_params['options'])) {
			$this->_params['options'] = Array();
		}
		if (!isset($this->_params['multiple'])) {
			$this->_params['multiple'] = false;
		}
		if (!isset($this->_params['allow_empty'])) {
			$this->_params['allow_empty'] = true;
		}
		if (!isset($this->_params['allow_other'])) {
			$this->_params['allow_other'] = false;
		}

		// set default edit parameters
		$this->_edit_params['style']      = 'list';
		$this->_edit_params['type']       = 'table';
		$this->_edit_params['height']     = '';
		$this->_edit_params['columns']    = '1';
		$this->_edit_params['empty_text'] = '';
		$this->_edit_params['extras']     = '';

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

		foreach (Array('style', 'height', 'columns', 'empty_text', 'extras') as $param) {
			if (isset($node->attributes[$param])) {
				$this->_edit_params[$param] = $node->attributes[$param];
			}
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
		$searchStyles = Array();
		$searchStyles['list'] = 'List';
		$searchStyles['checkbox'] = 'Checkbox';
		$searchTypes['table'] = 'Table';
		$searchTypes['list'] = 'Unordered List';

		if (!empty($this->name)) {
			echo '<b>Selection Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		}
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('style'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						combo_box($prefix.'_style', $searchStyles, false, $this->_edit_params['style']);
					} else {
						echo $this->_edit_params['style'];
					}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;">Type<?php //echo translate('columns'); ?></td>
				<td class="sq-backend-table-cell">
				<?php
				if ($write_access) {
					combo_box($prefix.'_type', $searchTypes, false, $this->_edit_params['type']);
				} else {
					echo $this->_edit_params['type'];
				}
			?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('height'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_height', $this->_edit_params['height']);
					} else {
						echo $this->_edit_params['height'];
					}
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('columns'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) {
						text_box($prefix.'_columns', $this->_edit_params['columns']);
					} else {
						echo $this->_edit_params['columns'];
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

		if (!empty($_POST[$prefix.'_type'])) {
			$this->_edit_params['type'] = $_POST[$prefix.'_type'];
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
		$values['type']       = $this->_edit_params['type'];
		$values['height']     = $this->_edit_params['height'];
		$values['columns']    = $this->_edit_params['columns'];
		$values['empty_text'] = $this->_edit_params['empty_text'];
		$values['extras']     = $this->_edit_params['extras'];

		return $values;

	}//end processEditParams()


	/**
	* Convert an array to a | separated string
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
	* Convert a | separated string to an array
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
			if (empty($this->_edit_params['empty_text'])) {
				$this->_edit_params['empty_text'] = '-- Leave Empty --';
			}
			$options = Array('' => $this->_edit_params['empty_text']) + $options;
		}
		if (!empty($this->_params['multiple']) && !is_array($this->value)) {
			$value = $this->_str2ar($this->value);
		} else {
			$value = $this->value;
		}

		if ($read_only) {
			if (empty($this->_params['multiple'])) {
				if (!isset($options[$value])) {
					// may be an "other" choice
					if ($this->_params['allow_other']) {
						echo translate('other').': '.$value;
					}
					// may have a value left over from when we
					// were a mutiple selection field
					$value = $this->_str2ar($value);
					if (empty($value) || !isset($options[$value[0]])) {
						return;
					}
					echo $options[$value[0]];
				} else {
					echo $options[$value];
				}

			} else {
				echo '<ul>';
				foreach ($value as $k) {
					if (!isset($options[$k])) continue; // "other" not allowed with multiple
					echo '<li class="sq-backend-data">'.htmlspecialchars($options[$k]).'</li>';
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
			if (empty($this->_params['multiple']) && $this->_params['allow_other']) {
				$options['__other__'] = translate('other');
				$onchange_code = 'document.getElementById(\''.$prefix.'_other\').style.visibility = (this.options[this.selectedIndex].value == \'__other__\') ? \'visible\' : \'hidden\'';
				$onchange_pos = strpos(strtolower($this->_edit_params['extras']), 'onchange=');
				if (false === $onchange_pos) {
					$this->_edit_params['extras'] .= ' onchange="'.$onchange_code.'"';
				} else {
					$this->_edit_params['extras'] = substr($this->edit_params['extras'], 0, $onchange_pos+10)
													.$onchange_code.'; '
													.substr($this->edit_params['extras'], $onchange_pos+11);
				}
			}
			combo_box($prefix, $options, !empty($this->_params['multiple']), $this->hasUnlistedValue() ? '__other__' : $value, $this->_edit_params['height'], $this->_edit_params['extras']);
			if (array_get_index($this->_params, 'allow_other')) {
				echo '&nbsp;';
				text_box($prefix.'_other', $this->hasUnlistedValue() ? $this->value : '', '', '', false, $this->hasUnlistedValue() ? '' : 'style="visibility: hidden"');
			}
		} else {
			if (empty($this->_params['multiple']) && $this->_params['allow_other']) {
				$options['__other__'] = translate('other');
				$onclick_pos = strpos(strtolower($this->_edit_params['extras']), 'onchange=');
				if (false === $onclick_pos) {
					$this->_edit_params['extras'] .= ' onclick="%on_click%"';
				} else {
					$this->_edit_params['extras'] = substr($this->edit_params['extras'], 0, $onclick_pos+10)
													.'%on_click%; '
													.substr($this->edit_params['extras'], $onclick_pos+11);
				}
			}

			if ($this->_edit_params['type'] == 'table') {
				//begin painting table
				?>
				<table cellpadding="0" cellspacing="0" border="0">
				<tr>
				<?php
				$i = 0;
				while (list($key, $val) = each($options)) {
					if (empty($this->_params['multiple'])) {
						?>
						<td>&nbsp;
						<?php
						$selected = (($key == '__other__') && $this->_params['allow_other']) && $this->hasUnlistedValue() || ($key == $value);
						$onclick = 'document.getElementById(\''.$prefix.'_other\').style.visibility = \''.(($key == '__other__') ? 'visible' : 'hidden').'\'';
						$extras = str_replace('%on_click%', $onclick, $this->_edit_params['extras']);
						$id = $prefix.'_'.$key;
						$extras .= ' id="'.$id.'"';
						radio_button($prefix, $key, $selected, '', $extras);
						label($val, $id);
						if (($key == '__other__') && $this->_params['allow_other']) {
							echo '&nbsp;';
							text_box($prefix.'_other', $this->hasUnlistedValue() ? $this->value : '', '', '', false, $selected ? '' : 'style="visibility: hidden"');
						}
					?>
						</td>
						<?php
					} else {
						$id = $prefix.'_'.$i;
						$extras = $this->_edit_params['extras'].' id="'.$id.'"';
							?>
								<td>&nbsp;
							<?php
								check_box($prefix.'['.$i.']', $key, (is_array($value) && in_array($key, $value)), '', $extras);
							label($val, $id);
							?>
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
					// end painting table
			} else {
				?>
				<ul>
				<?php
				$i = 0;
				while (list($key, $val) = each($options)) {
					?><li><?php
					if (empty($this->_params['multiple'])) {
						$selected = (($key == '__other__') && $this->_params['allow_other']) && $this->hasUnlistedValue() || ($key == $value);
						$onclick = 'document.getElementById(\''.$prefix.'_other\').style.visibility = \''.(($key == '__other__') ? 'visible' : 'hidden').'\'';
						$extras = str_replace('%on_click%', $onclick, $this->_edit_params['extras']);
						$id = $prefix.'_'.$key;
						$extras .= ' id="'.$id.'"';
						radio_button($prefix, $key, $selected, '', $extras);
						label($val, $id);
						if (($key == '__other__') && $this->_params['allow_other']) {
							echo '&nbsp;';
							text_box($prefix.'_other', $this->hasUnlistedValue() ? $this->value : '', '', '', false, $selected ? '' : 'style="visibility: hidden"');
						}
					} else {
						$id = $prefix.'_'.$i;
						$extras = $this->_edit_params['extras'].' id="'.$id.'"';
						check_box($prefix.'['.$i.']', $key, (is_array($value) && in_array($key, $value)), '', $extras);
						label($val, $id);
						?>
						</li>
						<?php
					}

					$i++;
				}
				?>
					</ul>
					<?php
			}

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
		if ($this->_params['allow_other'] && ($value == '__other__')) {
			$value = $_REQUEST[$prefix.'_other'];
		}

		if (is_array($value)) $value = $this->_ar2str($value);

		if ($this->value !== $value && $this->setValue($value)) {
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
		if ($value == '') {
			return $this->_params['allow_empty'];
		}

		// special case for multiple values
		if (empty($this->_params['multiple'])) {
			if (!isset($options[$value])) {
				return $this->_params['allow_other'];
			}
		} else {
			$new_value = $this->_str2ar($value);
			if (!is_array($new_value)) return false;
			$return_value = Array();
			foreach ($new_value as $key) {
				if (isset($options[$key])) $return_value[] = $key;
			}
			$value = $this->_ar2str($return_value);
		}

		return true;

	}//end validateValue()


	/**
	* Get all the options for this attribute, key => value
	*
	* @access public
	* @return array
	*/
	function getOptions()
	{
		return $this->_params['options'];

	}//end getOptions()


	/**
	* Returns an array of the current value with the individual selection values as keys
	* and the display names as values
	*
	* @return Array(scalar => scalar)
	* @access public
	*/
	function getValueArray()
	{
		$value = $this->_str2ar($this->value);
		$value_arr = Array();
		foreach ($value as $k) {
			$value_arr[$k] = $this->_params['options'][$k];
		}
		return $value_arr;

	}//end getValueArray()


	/**
	* Returns true if this attribute permits the "other" option and this option is selected
	*
	* @access public
	* @return void
	*/
	function hasUnlistedValue()
	{
		return $this->_params['allow_other'] && !isset($this->_params['options'][$this->value]);

	}//end hasUnlistedValue()


}//end class

?>
