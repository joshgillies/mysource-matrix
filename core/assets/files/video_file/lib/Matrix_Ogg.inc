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
* $Id: Matrix_Ogg.inc,v 1.1 2012/03/16 00:20:12 akarelia Exp $
*
*/

require_once SQ_TOOL_OGG_METADATA_EXTRACTION_PATH;



/**
* Matrix_Ogg
*
* Purpose
*		The original Ogg.class.php v1.3 (http://opensource.grisambre.net/ogg/)
*		has a small bug in its function realpath() where it is prefixing the
*		passed absolute file path with $_SERVER['DOCUMENT_ROOT'].
*		This is failing Matrix from extracting the Metadata for this ogg file
*		What we are doing here is extending our own class to original class
*		but on our way fixing the following function below.
*
*
* @author     Ashish Karelia <akarelia@gmail.com>
* @version    $Revision: 1.1 $
* @package    MySource_Matrix_Packages
* @subpackage lib
*/
class Matrix_Ogg extends Ogg
{

	/**
	* Construct
	*
	*/
	function __construct($filename, $caching)
	{
		parent::__construct($filename, $caching);

	}//end construct


	/**
	* This function checks for the path provided to
	* be valid. If relative path is provided will
	* make sure it is converted to a absolute patch
	*
	* @param string $path	path to the file
	*
	* return string
	*/
	function realpath($path)
	{ 
		if ((strpos($path, $_SERVER['DOCUMENT_ROOT']) !== FALSE) || (strpos($path, "://") !== FALSE)) {
			// path already absolute or remote
			return $path ;
		} else if ($path[0] != '/') {
			return str_replace("//","/",$_SERVER['DOCUMENT_ROOT'].$path); 
		} else {
			// relative path
			return $path ;
		}

	}//end realpath()


}//end class

?>
