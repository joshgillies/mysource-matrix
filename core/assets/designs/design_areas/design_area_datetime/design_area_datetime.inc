<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_datetime.inc,v 1.4 2003/09/26 05:26:24 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_DateTime
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_DateTime extends Design_Area
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
		$format = addslashes($this->attr('format'));
		switch($this->attr('datetime')) {
			case 'updated' :
				$datetime = '$ASSET->updated';
				break;
			case 'now' :
			default :
				$datetime = 'time()';
		}

		echo '<', '?php echo date("'.$format.'", '.$datetime.'); ?', '>';
		return true;
	}//end printArea()

}//end class
?>
