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
* $Id: text_file.inc,v 1.10 2012/08/30 01:09:07 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* Text_File
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Text_File extends File
{


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns the editable content of this asset
	*
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	function getContent($empty_content=TRUE)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parse_file = $this->data_path.'/'.$this->name;
		$content = file_to_string($parse_file);

		return trim($content);

	}//end getContent()


}//end class
?>
