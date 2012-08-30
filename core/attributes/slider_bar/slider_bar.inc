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
* $Id: slider_bar.inc,v 1.6 2012/08/30 01:09:18 ewang Exp $
*
*/


require_once SQ_ATTRIBUTES_PATH.'/int/int.inc';

/**
* Slider Bar Asset Attribute
*
* parameters => Array(
* 					'range_lower'		=> [int],
* 					'range_upper'		=> [int],
*				)
* edit_params => Array(
*					'lower_text'		=> [string],
*					'upper_text'		=> [string],
*				 )
*
*
* <!-- Sample Edit Interface XML Node -->
*    <slider_bar lower_text="slow" upper_text="fast">
* (NB slow and fast are localised string codes)
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Slider_Bar extends Asset_Attribute_Int
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Slider_Bar($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute($attributeid, $value);

		// apply default params where necessary
		$this->_params['allow_empty'] = FALSE;
		$this->_params['allow_negative'] = FALSE;
		if (!isset($this->_params['range_lower'])) {
			$this->_params['range_lower'] = 0;
		}
		if (!isset($this->_params['range_upper'])) {
			$this->_params['range_upper'] = 10;
		}

		// default edit params
		if (!isset($this->_edit_params['lower_text'])) {
			$this->_edit_params['lower_text'] = 'Lower';
		}
		if (!isset($this->_edit_params['upper_text'])) {
			$this->_edit_params['upper_text'] = 'upper';
		}


	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	XML_Tree_Node object to generate params from.
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{// TODO TOF TEST
		if (!Asset_Attribute::setEditParams($node)) {
			return FALSE;
		}
		$this->_edit_params['lower_text']  = translate((string)$node->attributes()->lower_text);
		$this->_edit_params['upper_text']  = translate((string)$node->attributes()->upper_text);
		return TRUE;

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
	function paint($prefix, $read_only=FALSE)
	{
		if ($this->value !== '') {
			$this->value = (int)$this->value;
		}
		$textbox_extras = $read_only ? 'disabled="disabled"' : '';
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		?>
		<div class="slider-container">
			<div class="slider-label-left">
				<?php echo $this->_edit_params['lower_text']; ?>
				<span>= <?php echo $this->_params['range_lower']; ?></span>
			</div>
			<?php int_text_box($prefix, $this->value, TRUE, 4, $this->_params['range_lower'], $this->_params['range_upper'], 0, FALSE, $textbox_extras); ?>
			<div class="slider-label-right">
				<?php echo $this->_edit_params['upper_text']; ?>
				<span>= <?php echo $this->_params['range_upper']; ?></span>
			</div>
			&nbsp;
		</div>
		<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/slider_bar.js"></script>
		<style type="text/css">
			@import url(<?php echo sq_web_path('lib');?>/web/css/slider_bar.css);
		</style>
		<?php

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix])) return FALSE;
		$value = $_REQUEST[$prefix];
		if ($value !== '') $value = (int)$value;
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = TRUE;
		} else {
			$this->processed = FALSE;
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
		$value = trim($value);

		$allow_negative = array_get_index($this->_params, 'allow_negative', TRUE);
		$allow_empty = array_get_index($this->_params, 'allow_empty', TRUE);

		if ($allow_empty && ($value === '')) {
			return TRUE;
		}

		if (!$allow_negative && $value < 0) {
			return FALSE;
		}

		$new_value = (int)$value;

		// we are comparing by string here, reason is that passing in a string as $value
		// becomes comparing $value with zero which is TRUE. So instead we would be comparing
		// it with a string '0', which is FALSE and what we want for it to invalidate.
		// Only genuine ints will pass through.
		if ((string)$new_value != $value) return FALSE;
		return TRUE;

	}//end validateValue()


}//end class

?>
