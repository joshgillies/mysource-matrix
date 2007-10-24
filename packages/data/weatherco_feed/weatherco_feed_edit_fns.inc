<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: weatherco_feed_edit_fns.inc,v 1.1.2.2 2007/10/24 06:07:37 bshkara Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/data/data_source_xml/data_source_xml_edit_fns.inc';

/**
* Weatherco_Feed_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1.2.2 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Weatherco_Feed_Edit_Fns extends Data_Source_Xml_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Weatherco_Feed_Edit_Fns()
	{
		$this->Data_Source_Xml_Edit_Fns();

	}//end constructor


}//end class
?>
