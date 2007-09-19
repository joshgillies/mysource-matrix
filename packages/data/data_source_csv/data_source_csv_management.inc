<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_csv_management.inc,v 1.1 2007/09/19 04:35:53 amiller Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_CSV_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Csv_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Csv_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
							'header_row'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
												'is_admin'		=> TRUE,
											   ),
							'cached_content'	=> Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
													'is_admin'	=> TRUE,
												   ),
					  );

	}//end constructor


}//end class

?>
