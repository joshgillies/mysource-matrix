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
* $Id: design_area_show_if_management.inc,v 1.6 2007/12/06 04:28:50 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Show_If_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Show_If_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'condition'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The condition to be evaluated.',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'condition_data'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Data passed to the condition to help in evaluation',
											   ),
						'then_contents'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The contents displayed when the condition is true',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'else_contents'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The contents displayed when the condition is false',
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
					  );

	}//end constructor


}//end class
?>
