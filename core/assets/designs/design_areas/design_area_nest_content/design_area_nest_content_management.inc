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
* $Id: design_area_nest_content_management.inc,v 1.9 2006/12/05 22:42:13 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Nest_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'type_codes'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'A list of type codes to restrict the asset selection to, these types are not strict however so any sub-types of these will also be valid. While this is a "serialise" attribute it can be set using a comma or space separated list of type codes',
										   ),
					  );

	}//end constructor


}//end class
?>
