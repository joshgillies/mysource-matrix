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
* $Id: data_source_rss_management.inc,v 1.4 2006/12/08 00:52:00 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_RSS_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_RSS_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Data_Source_RSS_Management(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'url'	=> Array(
									'added'		=> '0.1',
									'type'		=> 'text',
									'default'	=> '',
								   ),
					  );

	}//end constructor


}//end class

?>
