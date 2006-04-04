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
* $Id: option_list.inc,v 1.27 2006/04/04 01:51:14 tbarrett Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

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
* @version $Revision: 1.27 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Option_List extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Option_List($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute($attributeid, $value);

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$prefix = str_replace(':','_',$prefix); // handle shadow assets
		$value = explode("\r\n", $this->value);
		if (!$read_only) {
			?>
			<div id="option-list-<?php echo $prefix; ?>">
				<?php
				$order = 0;
				foreach ($value as $v) {
					$v = htmlentities($v);
					?>
					<input type="text"  name="<?php echo $prefix; ?>_options[]" value="<?php echo $v; ?>" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" /><button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
					<?php
						if ($order !=0) {
							$this->_printMoveUpButton($order, $prefix);
						}
						$this->_printMoveDownButton($order, $prefix);
					?><br />
					<?php
					$order++;
				}
				for ($i =0; $i < 2; $i++) {
					?>
					<input type="text" name="<?php echo $prefix; ?>_options[]" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]" /><button type="button" tabindex="99999" class="delete-button">&nbsp;</button>
					<?php
						$this->_printMoveUpButton($order, $prefix);
						if ($i < 1) {
							$this->_printMoveDownButton($order, $prefix);
						}
					?>
					<br />
					<?php
					$order++;
				}
				?>
			</div>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/edit.js"></script>
			<script type="text/javascript">
				var optionItemPrefix = '<?php echo $prefix; ?>';
				// attach the event handlers
				var optionList = document.getElementById('option-list-<?php echo $prefix; ?>');
				var	inputs = optionList.getElementsByTagName('INPUT');
				for (var j=0; j < inputs.length; j++) {
					inputs[j].onfocus = expandListFn;
				}
				var buttons = optionList.getElementsByTagName('BUTTON');
				for (var j=0; j < buttons.length; j++) {
					buttons[j].onclick = deleteRowFn;
				}

				lastOrder = <?php echo $order; ?>;
			</script>
			<?php
		} else {
			echo '<ul><li>'.implode('</li><li>', $value).'</li></ul>';
		}

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
			<a href="#" style="cursor:pointer;" onclick="listMoveDown(this);return false;" name="movedown" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>", "16", "16", "Move Down");</script></a>
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
			<a href="#" style="cursor:pointer;" onclick="listMoveUp(this);return false;" name="moveup" id="<?php echo $prefix; ?>_options[<?php echo $order; ?>]"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a>
		<?php

	}//end _printMoveUpButton()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix.'_options'])) return false;
		$prefix = str_replace(':','_',$prefix);	// handle shadow assets
		if (!isset($_REQUEST[$prefix.'_options'])) return;
		$value = $_REQUEST[$prefix.'_options'];
		foreach ($value as $i => $v) {
			if ($v == '') unset($value[$i]);
		}
		$this->processed = $this->setValue(implode("\r\n",$value));

	}//end process()


	/**
	* Returns a representation of this asset's parameters, edit parameters and value as XML
	*
	* @return string
	* @access public
	*/
	function exportContentsToXML()
	{

		$option_array = explode("\r\n", $this->value);
		$option_xml = '';
		foreach ($option_array as $option) {
			$option_xml .= '<option><![CDATA['.$option.']]></option>';
		}
		$xml = '<value>'.$option_xml.'</value>';

		return $xml;

	}//end exportContentsToXML()


}//end class

?>