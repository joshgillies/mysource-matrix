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
* $Id: content_type_raw_html.inc,v 1.5 2006/12/07 00:04:16 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Raw_HTML
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Raw_HTML extends Content_Type
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Content_Type_Raw_HTML($assetid=0)
	{
		$this->Content_Type($assetid);

	}//end constructor


}//end class

?>