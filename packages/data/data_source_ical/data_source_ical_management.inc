<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ical_management.inc,v 1.2 2010/05/05 07:11:14 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_iCal_Management
*
* @see Asset_Management
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
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
