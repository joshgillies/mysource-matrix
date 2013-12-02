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
* $Id: option_list.inc,v 1.45 2013/07/25 23:23:49 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

define('SQ_OPTION_LIST_DELIMITER_UNIX', "\n");
define('SQ_OPTION_LIST_DELIMITER_WINDOWS', "\r\n");
define('SQ_OPTION_LIST_DELIMITER_MAC', "\n");

/**
* Option List Asset Attribute
*
* A list of text fields. The user can add additional fields as required.
*
*
* <!-- Sample Edit Interface XML Node -->
*    <option_list />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.45 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Option_List extends Asset_Attribute
{


	/**
	* Defines how each option is delimited
	*
	* @var string
	* @access private
	*/
	var $delimiter = SQ_OPTION_LIST_DELIMITER_WINDOWS;


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Option_List($attributeid=0, $value=NULL, $delimiter=SQ_OPTION_LIST_DELIMITER_WINDOWS)
	{
		// As per Bug #2950, asset_attribute.inc passes a third parameter as an array
		// Check if delimiter is *NOT* an array, and set it as a delimiter
		if (!is_array($delimiter)) {
			$this->delimiter = $delimiter;
		}

		$this->Asset_Attribute($attributeid, $value);
		$this->_edit_params['width'] = '';

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
		if (!parent::setEditParams($node)) return FALSE;
		if (isset($node->attributes()->allow_reorder)) {
			if (strtolower($node->attributes()->allow_reorder) == 'no') {
				$this->_edit_params['allow_reorder'] = FALSE;
			} else {
				$this->_edit_params['allow_reorder'] = TRUE;
			}//end if
		}//end if
		return TRUE;

	}//end setEditParams()


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$read_only		are we just printing the value
	* @param boolean	$use_delimiter	Use the delimiter (TRUE) or was the value an array (FALSE)
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE, $use_delimiter=TRUE)
	{
		$allow_reorder = array_get_index($this->_edit_params, 'allow_reorder', TRUE);

		$prefix = str_replace(':','_',$prefix); // handle shadow assets
		if (!$use_delimiter) {
			$value = @unserialize($this->value);
		} else {
			$value = explode($this->delimiter, $this->value);
		}//end if
		if (!is_array($value) && empty($value)) {
			// If empty, give it a blank option
			$value = Array('0'=>'');
		}//end if

		if (!$read_only) {
			?>
			<div class="sq-option-list" id="option-list-<?php echo $prefix; ?>">
				<?php
				$order = 0;

				// Find the last key for the next empty options
				$temp_value = $value;
				end($temp_value);
				$last_number = key($temp_value);
				$next_number = '0';
				if (empty($value)) {
					$next_number = '0';
				} else if ($last_number >= 0) {
					$next_number = $last_number + 1;
				}//end if

				foreach ($value as $k => $v) {
					$v = htmlspecialchars($v);
					?>
					<input type="text"  name="<?php echo $prefix; ?>_options[]" value="<?php echo $v; ?>" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" <?php echo ($this->_edit_params['width'] ? 'size="'.$this->_edit_params['width'].'"' : ''); ?> /><button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
					<?php
						if ($allow_reorder) {
							if ($order !=0) {
								$this->_printMoveUpButton($order, $prefix);
							}
							$this->_printMoveDownButton($order, $prefix);
						}//end if
					?><br />
					<?php
					$order++;
				}
				for ($i = $next_number; $i < ($next_number + 2); $i++) {
					?>
					<input type="text" name="<?php echo $prefix; ?>_options[]" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" <?php echo ($this->_edit_params['width'] ? 'size="'.$this->_edit_params['width'].'"' : ''); ?> /><button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
					<?php
						if ($allow_reorder) {
							$this->_printMoveUpButton($order, $prefix);
							if ($i < ($next_number + 1)) {
								$this->_printMoveDownButton($order, $prefix);
							}
						}//end if
					?>
					<br />
					<?php
					$order++;
				}
				?>
			</div>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/edit.js"></script>
			<script type="text/javascript">
			<?php
			// Yeesh... this is hackish but this essentially (attempts to)
			// correct the lack of delete-button class in front-end
			// asset builder creation
			if (!SQ_IN_BACKEND && !SQ_IN_LIMBO) {
			?>
				buttons = document.getElementsByTagName('BUTTON');
				for (i = 0; i < buttons.length; i++) {
					if (buttons[i].className == 'delete-button') {
						with (buttons[i].style) {
							backgroundImage = 'url(<?php echo sq_web_path('lib') ?>/web/images/icons/delete.png)';
							backgroundPosition = '50% 50%';
							backgroundRepeat = 'no-repeat';
							width = '16px';
							height = '16px';
							backgroundColor = 'transparent';
							border = '0px';
							cursor = 'pointer';
						}
					}
				}
			<?php
			}
			?>
				var optionItemPrefix = '<?php echo $prefix; ?>';
				// attach the event handlers
				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var	inputs = optionList.getElementsByTagName('INPUT');
				for (var j=0; j < inputs.length; j++) {
					<?php if ($allow_reorder) { ?>
						inputs[j].onfocus = expandListFn;
					<?php } else { ?>
						inputs[j].onfocus = noreorderExpandListFn;
					<?php } ?>
				}
				var buttons = optionList.getElementsByTagName('BUTTON');
				for (var j=0; j < buttons.length; j++) {
					<?php if ($allow_reorder) { ?>
						buttons[j].onclick = deleteRowFn;
					<?php } else { ?>
						buttons[j].onclick = noreorderDeleteRowFn;
					<?php } ?>
				}

				lastOrder = <?php echo $order; ?>;
			</script>
			<?php
		} else {
			if (!empty($value)) {
				echo '<ul><li>'.implode('</li><li>', $value).'</li></ul>';
			} else {
				echo translate('not_set');
			}//end if
		}//end if

	}//end paint()


	/**
	* paint the down arrow button
	*
	* @param string	$order	Order of the element
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function _printMoveDownButton($order, $prefix)
	{
		?>
			<a href="#" style="cursor:pointer;" onclick="listMoveDown(this, document.getElementById('option-list-<?php echo $prefix; ?>'));return false;" tabindex="99999" name="movedown" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "15", "15", "Move Down");</script></a>
		<?php

	}//end _printMoveDownButton()


	/**
	* paint the down arrow button
	*
	* @param string	$order	Order of the element
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function _printMoveUpButton($order, $prefix)
	{
		?>
			<a href="#" style="cursor:pointer;" onclick="listMoveUp(this, document.getElementById('option-list-<?php echo $prefix; ?>'));return false;" tabindex="99999" name="moveup" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "15", "15", "Move Up");</script></a>
		<?php

	}//end _printMoveUpButton()


	/**
	* Process the interface for filling in a value
	*
	* @param string		$prefix			prefix for the form elements
	* @param boolean	$use_delimiter	Use the delimiter (TRUE) or was the value an array (FALSE)
	* @param boolean	$remove_duplicates	Whether to remove duplicates from the list
	*
	* @return void
	* @access public
	*/
	function process($prefix, $use_delimiter=TRUE, $remove_duplicates=FALSE)
	{
		$prefix = str_replace(':','_',$prefix);	// handle shadow assets
		if (!isset($_REQUEST[$prefix.'_options'])) return;
		$value = $_REQUEST[$prefix.'_options'];
		foreach ($value as $i => $v) {
			if ($v == '') unset($value[$i]);
		}

		if ($remove_duplicates) {
			$value = array_unique($value);
		}

		if (!$use_delimiter) {
			$processed_value = serialize($value);
		} else {
			$processed_value = implode($this->delimiter,$value);
		}//end if
		$this->processed = $this->setValue($processed_value);

	}//end process()


	/**
	* Returns a representation of this asset's parameters, edit parameters and value as XML
	*
	* @return string
	* @access public
	*/
	function exportContentsToXML()
	{

		$option_array = explode($this->delimiter, $this->value);
		$option_xml = '';
		foreach ($option_array as $option) {
			$option_xml .= '<option><![CDATA['.$option.']]></option>';
		}
		$xml = '<value>'.$option_xml.'</value>';

		return $xml;

	}//end exportContentsToXML()


}//end class

?>
