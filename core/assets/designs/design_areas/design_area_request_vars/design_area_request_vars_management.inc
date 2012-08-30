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
* $Id: design_area_request_vars_management.inc,v 1.6 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Request_Vars_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Request_Vars_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'method'	=> Array(
										'added'			=> '0.0.1',
										'type'			=> 'selection',
										'default'		=> 'request',
										'parameters'	=> Array(
															'options'		=> Array(
																				'request'	=> 'All Request Variables',
																				'get'		=> 'GET Variables',
																				'post'		=> 'POST Variables',
																			   ),
															'multiple'		=> FALSE,
															'allow_empty'	=> FALSE,
														   ),
										'description'	=> 'The set of request variables to declare',
									   ),
					  );

	}//end constructor


}//end class
?>
