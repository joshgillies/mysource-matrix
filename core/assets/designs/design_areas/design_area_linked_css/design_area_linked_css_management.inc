<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_linked_css_management.inc,v 1.11 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Linked_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars['tag_type'] = Array(
									'added'			=> '0.0.1',
									'type'			=> 'selection',
									'parameters'	=> Array(
														'options'		=> Array(
																			'link_tag'		=> 'Link Tag',
																			'import_tag'	=> '@import Tag',
																		   ),
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
													   ),
									'default'		=> 'link_tag',
								  );

		$this->vars['media'] = Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'parameters'	=> Array(
													'options'		=> Array(
																		'all'			=> 'All',
																		'aural'			=> 'Aural',
																		'braille'		=> 'Braille',
																		'embossed'		=> 'Embossed',
																		'handheld'		=> 'Handheld',
																		'print'			=> 'Print',
																		'projection'	=> 'Projection',
																		'screen'		=> 'Screen',
																		'tty'			=> 'TTY',
																		'tv'			=> 'TV',
																	   ),
													'multiple'		=> FALSE,
													'allow_empty'	=> FALSE,
												   ),
								'default'		=> 'all',
							   );

	}//end constructor


}//end class
?>
