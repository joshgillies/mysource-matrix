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
* $Id: design_area_body_management.inc,v 1.12 2007/12/06 04:28:48 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Body_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Body_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'format'	=> Array(
										'added'			=> '0.1',
										'type'			=> 'selection',
										'default'		=> 'html',
										'description'	=> 'Format that the body tag is printed as (text, HTML, etc)',
										'parameters'	=> Array(
															'options'		=> Array(
																				'html'			=> translate('full_html'),
																				'text'			=> translate('text_only'),
																				'low_bandwidth'	=> translate('low_bandwidth'),
																			   ),
															'allow_empty'	=> FALSE,
														   ),
									   ),
					  );

	}//end constructor


}//end class
?>
