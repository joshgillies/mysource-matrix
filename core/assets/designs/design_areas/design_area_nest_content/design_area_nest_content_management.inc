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
* $Id: design_area_nest_content_management.inc,v 1.11 2008/07/25 04:52:49 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Nest_Content_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'type_codes'	=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'A list of type codes to restrict the asset selection to, these types are not strict however so any sub-types of these will also be valid. While this is a "serialise" attribute it can be set using a comma or space separated list of type codes',
										   ),
						'paint_with'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'text',
											'default'		=> 'raw',
											'description'	=> 'Allow this nested content to be painted by a specified paint layout',
										   ),
						'paint_layout'	=> Array(
											'added'			=> '0.2',
											'type'			=> 'int',
											'default'		=> '0',
											'description'	=> 'Allow this nested content to be painted by a this paint layout',
										   ),
					  );

	}//end constructor


}//end class
?>
