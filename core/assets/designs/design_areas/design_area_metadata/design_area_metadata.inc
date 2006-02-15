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
* $Id: design_area_metadata.inc,v 1.12 2006/02/15 00:05:20 sdanis Exp $
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
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Metadata extends Design_Area
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
					$METADATA_MANAGER = &$GLOBALS[\'SQ_SYSTEM\']->getMetadataManager();
					$metadata_field_value = $METADATA_MANAGER->getMetadataFieldValues($ASSET->id, Array('.var_export($field_name, TRUE).'));
					echo array_get_index($metadata_field_value, '.var_export($field_name, TRUE).', \'\');
				 ';
		}
		echo '?', ">\n";
		return TRUE;

	}//end _printPaintingCode()


}//end class
?>
