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
* $Id: design_area_menu_recursive_management.inc,v 1.15 2007/12/06 04:28:50 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Menu_Recursive_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Recursive_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'min_num_levels'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'The minimum number of levels into the hierarchy to always show. Set to zero to emulate "show_subs" == "on_current", set to number greater than zero to be "show_sub" == "always" for that many levels',
											   ),
						'max_num_levels'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'int',
												'default'		=> 0,
												'description'	=> 'The maximum number of levels into the hierarchy to show, blank or zero means no maximum',
											   ),
					  );

	}//end constructor


}//end class
?>
