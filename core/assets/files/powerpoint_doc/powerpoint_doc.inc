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
* $Id: powerpoint_doc.inc,v 1.1 2007/01/11 05:53:33 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';


/**
* Powerpoint_Doc
*
* Handler of Excel documents
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Powerpoint_Doc extends File
{

	/**
	* All allowed extensions for a word doc (document and template recognised)
	* @var Array(string)
	*/
	var $allowed_extensions = Array('ppt');


}//end class
?>
