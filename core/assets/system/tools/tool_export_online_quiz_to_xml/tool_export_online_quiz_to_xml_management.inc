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
* $Id: tool_export_online_quiz_to_xml_management.inc,v 1.1 2012/02/15 04:17:08 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Tool_Export_Online_Quiz_To_XML_Management
*
* @see Asset_Management
*
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Tool_Export_Online_Quiz_To_XML_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Tool_Export_Online_Quiz_To_XML_Management(&$pm)
	{
		$this->Asset_Management($pm);

	}//end constructor

  

}//end class

?>