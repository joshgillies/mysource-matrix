<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_searchbox.inc,v 1.4 2003/10/17 00:31:15 dwong Exp $
* $Name: not supported by cvs2svn $
*
*/

/**
* Design_Area_SearchBox
*
* Searchbox - A design area that shows a search box for 'quick'-searching. 
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.4 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage search
*/

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';
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
	}

	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @see Design_Area::printArea()
	* @access public
	*/

	function printArea(&$design) 
	{
		$assetid = $this->attr('search_page_assetid');
		if ($assetid <= 0) {
			echo "No search page configured for this search box.";
			return true;
		} else {
			return parent::printArea($design);
		}
	}

	/*
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
			case 'form_start':
				echo '<form method="GET" action="'. $asset->getURL(). '">';
			break;
			
			case 'form_end':
				echo '</form>';
			break;

			case 'box':
				echo '<input type=text size="'. $this->attr('querybox_width') .'" name="queries['. $this->attr('field_name').'_query]" value="';

				echo '<?php ';
				echo 'if (isset($_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'])) 
						echo $_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'];';
				echo '?>';

				echo '">';
			break;

			case 'submit':
				if ($this->attr('submit_button')) {
					echo '<td><input type=submit value ="' . 'go' . '"></td>';
				} else {
					echo '<td>';
					$submit_image_assetid = $this->attr('submit_image_assetid');
					if ($submit_image_assetid > 0) {
						$submit_image_asset =& $am->getAsset($submit_image_assetid);
						echo '<input type=image src="'. $submit_image_asset->getURL() .'">';
					} else {
						echo 'No image specified for submitting.';
					}
					echo '</td>';
				}
			break;
		}
	} // end printVar()

}

?>