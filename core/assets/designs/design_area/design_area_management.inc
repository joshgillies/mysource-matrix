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
* $Id: design_area_management.inc,v 1.18 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'id_name'			=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'contents'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Holds the processed contents array',
											   ),
						'var_references'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Holds the names for all variables that are "references" to values in other DAs',
											   ),
						'print'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'Whether this DA should print output from its AREA tag or just initialise itself for later reference',
											   ),
						'cache'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'description'	=> 'Whether this DA should cache its output',
											   ),
					  );

	}//end constructor


}//end class
?>
