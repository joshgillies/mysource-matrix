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
* $Id: design_area_asset_lineage_management.inc,v 1.14 2008/04/06 22:55:39 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Asset_Lineage_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'divider_contents'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'The contents that divides the asset info in the output',
													   ),
						'asset_contents'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'The contents that prints the asset info in the output',
													   ),
						'prefix_with_home_link'		=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'whether or not to start the list with a link to the site\'s index',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'suffix_with_current_link'	=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'whether or not to end the list with a link to the current page',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'prefix_with_divider'		=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'whether or not to start the list with a divider',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'suffix_with_divider'		=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'description'	=> 'whether or not to end the list with a divider',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'show_hidden_links'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'boolean',
														'default'		=> TRUE,
														'description'	=> 'whether or not to show TYPE_2 links as well as TYPE_1',
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													   ),
						'levels_to_print'			=> Array(
														'added'			=> '0.0.1',
														'type'			=> 'int',
														'default'		=> 0,
														'description'	=> 'How many levels deep to print, (zero, blank, empty means print all)',
													   ),
						'start_at_level'			=> Array(
														'added' => '0.0.1',
												  		'type'  => 'int',
														'default' => NULL,
														'description' => 'At what level to start printing',
														'parameters'	=> Array(
																			'allow_empty'	=> TRUE,
																		   ),
													   ),
						'reverse_lineage'			=> Array(
														'added'			=> '0.2',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'description'	=> 'If true, reverse how the lineage is printed (ie. nearest asset first)',
													   ),
						'unwanted_asset_types'		=> Array(
														'added'			=> '0.3',
														'type'			=> 'serialise',
														'default'		=> Array(),
														'description'	=> 'List of asset types to exclude from the lineage',
													   ),
					  );

	}//end constructor


}//end class
?>
