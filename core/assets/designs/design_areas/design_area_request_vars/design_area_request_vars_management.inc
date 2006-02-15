<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: design_area_request_vars_management.inc,v 1.3 2006/02/15 02:55:37 sdanis Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Request_Vars_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Request_Vars_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Request_Vars_Management(&$pm)
	{
		$this->Asset_Management($pm);

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
