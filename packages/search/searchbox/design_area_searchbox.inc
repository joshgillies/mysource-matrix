<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_searchbox.inc,v 1.2 2003/10/13 10:03:14 brobertson Exp $
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
* @version $Revision: 1.2 $ - 1.0
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
		}
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$asset =& $am->getAsset($assetid);

		echo '<form method="GET" action="'. $asset->getURL(). '">';

		echo '<input type=hidden name="mode" value="results">';
		echo '<input type=text size="'. $this->attr('querybox_width') .'" name="queries['. $this->attr('field_name').'_query]" value="<?';
		echo 'if (isset($_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'])) echo $_REQUEST[\'queries\'][\''. $this->attr('field_name'). '_query\'];';
		echo '?>">';

		if ($this->attr('submit_button')) {
			echo '<input type=submit value ="' . 'go' . '">';
		} else {
			$submit_image_assetid = $this->attr('submit_image_assetid');
			if ($submit_image_assetid > 0) {
				$submit_image_asset =& $am->getAsset($submit_image_assetid);
				echo '<input type=image src="'. $submit_image_asset->getURL() .'">';
			} else {
				echo 'No image specified for submitting.';
			}
		}

		echo '</form>';
		return true;

	}//end printArea()


}

?>