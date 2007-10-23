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
* $Id: rtf_file.inc,v 1.2 2007/10/23 03:35:15 bshkara Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';

/**
* RTF File
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Rtf_File extends File
{


	/**
	* All allowed extension for an rtf file
	* @var Array(string)
	*/
	var $allowed_extensions = Array('rtf');


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function Rtf_File($assetid=0)
    {
        $this->File($assetid);

    }//end constructor


}//end class
?>
