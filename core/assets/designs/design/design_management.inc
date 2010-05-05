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
* $Id: design_management.inc,v 1.21 2010/05/05 01:16:53 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.21 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'id_name' 			=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
								 			   ),
						'wysiwyg_classes'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Use saved classes in the wysiwyg dropdown list',
											   ),
						'div_classes'		=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Use saved classes in the DIV container properties class dropdown',
											   ),
						'limbo_no_frames'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'    => Array (
                                                                    'allow_empty'   => FALSE,
                                                                   ),
												'description'	=> 'LIMBO no frame options for each design, not using frames while inside LIMBO mode',
											   ),
					  );

	}//end constructor


}//end class


?>
