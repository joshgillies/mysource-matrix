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
* $Id: data_source_ical_management.inc,v 1.1 2009/08/16 23:05:21 bpearson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_iCal_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_iCal_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_iCal_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'path'		=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
									   ),
					  );

	}//end constructor


}//end class

?>
