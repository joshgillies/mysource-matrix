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
* $Id: excel_doc.inc,v 1.5 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';


/**
* Excel_Doc
*
* Handler of Excel documents
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Excel_Doc extends File
{


	/**
	* All allowed extensions for a excel doc
	*
	* Workbook and templates recognised, for both earlier versions of Excel, and
	* Office 2007+ OOXML format (with macros both disabled and enabled).
	*
	* @var Array(string)
	*/
	public $allowed_extensions = Array('xls', 'xlt', 'xlsx', 'xlsm', 'xltx', 'xltm');


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


}//end class
?>
