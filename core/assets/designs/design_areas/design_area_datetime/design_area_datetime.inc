<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_datetime.inc,v 1.11 2006/12/05 22:42:12 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_DateTime
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_DateTime extends Design_Area
{


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	function _printPaintingCode()
	{
		$format = addslashes($this->attr('format'));
		switch ($this->attr('datetime')) {
			case 'updated' :
				$datetime = '$ASSET->updated';
			break;
			case 'now' :
			default :
				$datetime = 'time()';
		}

		echo '<', '?php echo date("'.$format.'", '.$datetime.'); ?', '>';
		return TRUE;

	}//end _printPaintingCode()


}//end class
?>
