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
* $Id: excel_doc.inc,v 1.1.4.1 2008/09/29 02:04:25 hnguyen Exp $
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
* @version $Revision: 1.1.4.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Excel_Doc extends File
{

	/**
	* All allowed extensions for a word doc (document and template recognised)
	* @var Array(string)
	*/
	var $allowed_extensions = Array('xls', 'xlsx');


}//end class
?>
