<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Metadata
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Metadata extends Design_Area
{

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		echo '<', '?php'."\n";
		echo '
				if (file_exists($ASSET->data_path.\'/metadata.php\')) {
					require($ASSET->data_path.\'/metadata.php\');
				}
			 ';
		echo '?', ">\n";
		return true;

	}//end printArea()

}//end class

?>