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
* $Id: design_area_metadata.inc,v 1.15 2007/12/10 06:20:49 rong Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Metadata
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Metadata extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		echo '<', '?php'."\n";
		$field_name = $this->attr('metadata_field');
		if (empty($field_name)) {
			// printing all metadata
			echo '
					if (file_exists($ASSET->data_path.\'/metadata.php\')) {
						require($ASSET->data_path.\'/metadata.php\');
					}
				 ';
		} else {
			// printing a specific field
			echo '
					$METADATA_MANAGER = $GLOBALS[\'SQ_SYSTEM\']->getMetadataManager();
					$metadata_field_value = $METADATA_MANAGER->getMetadataFieldValues($ASSET->id, Array('.var_export($field_name, TRUE).'));
					echo array_get_index($metadata_field_value, '.var_export($field_name, TRUE).', \'\');
				 ';
		}
		echo '?', ">\n";
		return TRUE;

	}//end _printPaintingCode()


}//end class
?>
