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
* $Id: design_area_searchbox.inc,v 1.11.2.1 2004/02/26 19:10:22 brobertson Exp $
* $Name: not supported by cvs2svn $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Design_Area_SearchBox
*
* Searchbox - A design area that shows a search box for 'quick'-searching.
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.11.2.1 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Design_Area_Searchbox extends Design_Area
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Design_Area_Searchbox($assetid=0)
	{
		$this->Design_Area($assetid);

	}//end constructor


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	* @see Design_Area::printArea()
	*/
	function printArea(&$design)
	{
		$assetid = $this->attr('search_page_assetid');
		if ($assetid <= 0) return true;
		else return parent::printArea($design);

	}//end printArea()


	/**
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{
		$assetid = $this->attr('search_page_assetid');
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset =& $am->getAsset($assetid);

		switch($var) {

			case 'form_start' :
				echo '<'.'?php $asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$asset->id.', "'.$asset->type().'"); ?'.'>'."\n";
				echo '<form method="get" action="<'.'?php echo $asset->getURL(); ?'.'>"><input type="hidden" name="mode" value="results" />';
			break;

			case 'form_end' :
				echo '</form>';
			break;

			case 'box' :
				echo '
				<'.'?php
				$default_value = \''.$this->attr('default_text').'\';
				$hide_default = true;
				if (isset($_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'])) {
					$default_value = $_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'];
					$hide_default = false;
				}
				$class = \''.((isset($args['class'])) ? $args['class'] : '').'\';

				require_once SQ_LIB_PATH.\'/html_form/html_form.inc\';
				text_box(\'queries['. $this->attr('field_name').'_query]\', $default_value, \''.$this->attr('querybox_width').'\', \'\', $hide_default, (empty($class)) ? \'\' : \'class="\'.$class.\'"\');
				?'.'>
				';
			break;

			case 'submit' :
				if ($this->attr('submit_button')) {
					echo '<input type="submit" value ="' . 'go' . '"';
					if (isset($args['class'])) {
						echo ' class="'. $args['class'] . '"';
					}
					echo ' />';
				} else {
					$submit_image_assetid = $this->attr('submit_image_assetid');
					if ($submit_image_assetid > 0) {
						$submit_image_asset =& $am->getAsset($submit_image_assetid);
						echo '<'.'?php $submit_image_asset = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$submit_image_asset->id.', "'.$submit_image_asset->type().'"); ?'.'>';
						echo '<input type="image"';
						if (isset($args['class'])) {
							echo ' class="'. $args['class'] .'"';
						}
						echo ' src="<'.'?php echo $submit_image_asset->getURL(); ?'.'>" />';
					} else {
						// no image defined
					}
				}
			break;

		}//end switch

	}//end printVar()


}//end classs

?>