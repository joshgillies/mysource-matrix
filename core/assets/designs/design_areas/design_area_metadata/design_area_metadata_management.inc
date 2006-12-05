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
* $Id: design_area_metadata_management.inc,v 1.9 2006/12/05 22:42:13 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Metadata_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Metadata_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Metadata_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'metadata_field'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Output a specific field value - leave blank to output all metadata',
											   ),
					  );

	}//end constructor


}//end class

?>