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
* $Id: design_area_colourise_image_edit_fns.inc,v 1.22 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Colourise_Image_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Colourise_Image_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design_Area_Colourise_Image	$asset		the asset to which we belong
	* @param array							$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Colourise_Image $asset, Array $contents)
	{
		// blank out the colours array so that we don't end up remembering old values
		$asset->setAttrValue('colours', Array());
		return parent::_processContents($asset, $contents);

	}//end _processContents()


}//end class
?>
