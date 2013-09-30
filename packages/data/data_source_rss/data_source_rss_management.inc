<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_rss_management.inc,v 1.7.6.1 2013/09/30 05:50:44 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_RSS_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7.6.1 $
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
						'url'			=> Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'date_field'	=> Array(
											'added'		=> '0.2',
											'type'		=> 'text',
											'default'	=> '',
										   ),
		    				'data_delimiter'			=> Array (
												'added'			=> '0.3',
												'type'			=> 'text',
												'default'		=> ',',
												'description'	=> 'Delimiter to seperate the data from xml record with mutiple elements with same name',
											   ),
					  );

	}//end constructor


}//end class

?>
